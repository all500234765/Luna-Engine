#pragma once

#include "pc.h"
#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Scene/Texture.h"
#include "Engine/Utility/Utils.h"                   // LunaEngine::
#include "Engine/Profiler/ScopedRangeProfiler.h"

// SIMD
#include "SIMD/avx_mathfun.h"

extern _DirectX *gDirectX;

struct CBuffArgs {
    // Val - Scaling
    //  0  - 100%
    //  1  - 50%
    //  2  - 25%
    //  3  - 12%
    uint32_t Scaling = 1;

    float _CameraNear;
    float _CameraFar;

};

static const int sBBIndexList[36] = {
    // Top
    3, 7, 6,
    3, 6, 2,

    // Bottom
    4, 0, 1,
    4, 1, 5,

    // Left
    4, 7, 3,
    4, 3, 0,

    // Right
    1, 2, 6,
    1, 6, 5,

    // Back
    5, 6, 7,
    5, 7, 4,

    // Front
    0, 3, 2,
    0, 2, 1
};


struct Box3F {
    float3 Origin, Size;

    inline float3 getCenter() const { return { Origin.x + Size.x * .5f,
                                               Origin.y + Size.y * .5f,
                                               Origin.z + Size.z * .5f }; };
    inline float3 getExtents() const { return Size; };

};

class CoverageBuffer {
private:

    Shader *shDownScaleDepthCS;

    Texture *_DepthDS, *_DepthDS_Read;

    ConstantBuffer *cbDownscaling;

    struct Downscaling {
        uint2  _Res;        // Resolution
        float2 _ResRcp;     // 1.f / _Res
        float2 _ProjValues; // 
        int1   _DownScale;  // Downscale on both axis
        int1   _DownScale2; // Downscale log2
    };

    // Saves last RT's size.
    // So we can do something later
    float fWidth;
    float fHeight;
    uint32_t mLastScaling = 100u;

    // Depth buffer texture data
    size_t mLastSize = 0u;
    float* pDepthBuffer = nullptr;

    // SSE
    __m128 SSETransformCoords(__m128 *v, __m128 *m) {
        __m128 vResult = _mm_shuffle_ps(*v, *v, _MM_SHUFFLE(0, 0, 0, 0));
        vResult = _mm_mul_ps(vResult, m[0]);

        __m128 vTemp = _mm_shuffle_ps(*v, *v, _MM_SHUFFLE(1, 1, 1, 1));
        vTemp = _mm_mul_ps(vTemp, m[1]);

        vResult = _mm_add_ps(vResult, vTemp);
        vTemp = _mm_shuffle_ps(*v, *v, _MM_SHUFFLE(2, 2, 2, 2));

        vTemp = _mm_mul_ps(vTemp, m[2]);
        vResult = _mm_add_ps(vResult, vTemp);

        vResult = _mm_add_ps(vResult, m[3]);
        return vResult;
    }

    __forceinline __m128i Min(const __m128i &v0, const __m128i &v1) {
        __m128i tmp;
        tmp = _mm_min_epi32(v0, v1);
        return tmp;
    }
    __forceinline __m128i Max(const __m128i &v0, const __m128i &v1) {
        __m128i tmp;
        tmp = _mm_max_epi32(v0, v1);
        return tmp;
    }

    struct SSEVFloat4 {
        __m128 X;
        __m128 Y;
        __m128 Z;
        __m128 W;
    };

    // get 4 triangles from vertices
    void SSEGather(SSEVFloat4 pOut[3], int triId, const __m128 xformedPos[]) {
        for( int i = 0; i < 3; i++ ) {
            int ind0 = sBBIndexList[triId * 3 + i + 0];
            int ind1 = sBBIndexList[triId * 3 + i + 3];
            int ind2 = sBBIndexList[triId * 3 + i + 6];
            int ind3 = sBBIndexList[triId * 3 + i + 9];

            __m128 v0 = xformedPos[ind0];
            __m128 v1 = xformedPos[ind1];
            __m128 v2 = xformedPos[ind2];
            __m128 v3 = xformedPos[ind3];
            _MM_TRANSPOSE4_PS(v0, v1, v2, v3);
            pOut[i].X = v0;
            pOut[i].Y = v1;
            pOut[i].Z = v2;
            pOut[i].W = v3;

            //now X contains X0 x1 x2 x3, Y - Y0 Y1 Y2 Y3 and so on...
        }
    }

    bool RasterizeTestBBoxSSE(Box3F box, __m128 *matrix, float *buffer, int4 res) {
        //verts and flags
        __m128 verticesSSE[8];
        int flags[8];
        static float4 vertices[8];
        static float4 xformedPos[3];
        static int flagsLoc[3];

        // Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
        // Denormal are zero (DAZ) is bit 6 and Flush to zero (FZ) is bit 15. 
        // so to enable the two to have to set bits 6 and 15 which 1000 0000 0100 0000 = 0x8040
        _mm_setcsr(_mm_getcsr() | 0x8040);


        // init vertices
        float3 center  = box.getCenter();  // origin + dim * .5f
        float3 extent  = box.getExtents(); // Size in each dim?
        float4 vCenter = float4(center.x, center.y, center.z, 1.f);
        float4 vHalf   = float4(extent.x * .5f, extent.y * .5f, extent.z * .5f, 1.0);

        float4 vMin = { vCenter.x - vHalf.x, vCenter.y - vHalf.y, vCenter.z - vHalf.z, vCenter.w - vHalf.w };
        float4 vMax = { vCenter.x + vHalf.x, vCenter.y + vHalf.y, vCenter.z + vHalf.z, vCenter.w + vHalf.w };

        // fill vertices
        vertices[0] = float4(vMin.x, vMin.y, vMin.z, 1.f);
        vertices[1] = float4(vMax.x, vMin.y, vMin.z, 1.f);
        vertices[2] = float4(vMax.x, vMax.y, vMin.z, 1.f);
        vertices[3] = float4(vMin.x, vMax.y, vMin.z, 1.f);
        vertices[4] = float4(vMin.x, vMin.y, vMax.z, 1.f);
        vertices[5] = float4(vMax.x, vMin.y, vMax.z, 1.f);
        vertices[6] = float4(vMax.x, vMax.y, vMax.z, 1.f);
        vertices[7] = float4(vMin.x, vMax.y, vMax.z, 1.f);

        // transforms
        for( int i = 0; i < 8; i++ ) {
            verticesSSE[i] = _mm_loadu_ps(&vertices[i].x);

            verticesSSE[i] = SSETransformCoords(&verticesSSE[i], matrix);

            __m128 vertX = _mm_shuffle_ps(verticesSSE[i], verticesSSE[i], _MM_SHUFFLE(0, 0, 0, 0)); // xxxx
            __m128 vertY = _mm_shuffle_ps(verticesSSE[i], verticesSSE[i], _MM_SHUFFLE(1, 1, 1, 1)); // yyyy
            __m128 vertZ = _mm_shuffle_ps(verticesSSE[i], verticesSSE[i], _MM_SHUFFLE(2, 2, 2, 2)); // zzzz
            __m128 vertW = _mm_shuffle_ps(verticesSSE[i], verticesSSE[i], _MM_SHUFFLE(3, 3, 3, 3)); // wwww
            static const __m128 sign_mask = _mm_set1_ps(-0.f); // -0.f = 1 << 31
            vertW = _mm_andnot_ps(sign_mask, vertW); // abs
            vertW = _mm_shuffle_ps(vertW, _mm_set1_ps(1.0f), _MM_SHUFFLE(0, 0, 0, 0)); //w,w,1,1
            vertW = _mm_shuffle_ps(vertW, vertW, _MM_SHUFFLE(3, 0, 0, 0)); //w,w,w,1

            // project
            verticesSSE[i] = _mm_div_ps(verticesSSE[i], vertW);

            // now vertices are between -1 and 1
            const __m128 sadd = _mm_setr_ps(res.x * 0.5, res.y * 0.5, 0, 0);
            const __m128 smult = _mm_setr_ps(res.x * 0.5, res.y * (-0.5), 1, 1);

            verticesSSE[i] = _mm_add_ps(sadd, _mm_mul_ps(verticesSSE[i], smult));
        }

        // Rasterize the AABB triangles 4 at a time
        for( int i = 0; i < 12; i += 4 ) {
            SSEVFloat4 xformedPos[3];
            SSEGather(xformedPos, i, verticesSSE);

            // by 3 vertices
            // fxPtX[0] = X0 X1 X2 X3 of 1st vert in 4 triangles
            // fxPtX[1] = X0 X1 X2 X3 of 2nd vert in 4 triangles
            // and so on
            __m128i fxPtX[3], fxPtY[3];
            for( int m = 0; m < 3; m++ ) {
                fxPtX[m] = _mm_cvtps_epi32(xformedPos[m].X);
                fxPtY[m] = _mm_cvtps_epi32(xformedPos[m].Y);
            }

            // Fab(x, y) =     Ax       +       By     +      C              = 0
            // Fab(x, y) = (ya - yb)x   +   (xb - xa)y + (xa * yb - xb * ya) = 0
            // Compute A = (ya - yb) for the 3 line segments that make up each triangle
            __m128i A0 = _mm_sub_epi32(fxPtY[1], fxPtY[2]);
            __m128i A1 = _mm_sub_epi32(fxPtY[2], fxPtY[0]);
            __m128i A2 = _mm_sub_epi32(fxPtY[0], fxPtY[1]);

            // Compute B = (xb - xa) for the 3 line segments that make up each triangle
            __m128i B0 = _mm_sub_epi32(fxPtX[2], fxPtX[1]);
            __m128i B1 = _mm_sub_epi32(fxPtX[0], fxPtX[2]);
            __m128i B2 = _mm_sub_epi32(fxPtX[1], fxPtX[0]);

            // Compute C = (xa * yb - xb * ya) for the 3 line segments that make up each triangle
            __m128i C0 = _mm_sub_epi32(_mm_mullo_epi32(fxPtX[1], fxPtY[2]), _mm_mullo_epi32(fxPtX[2], fxPtY[1]));
            __m128i C1 = _mm_sub_epi32(_mm_mullo_epi32(fxPtX[2], fxPtY[0]), _mm_mullo_epi32(fxPtX[0], fxPtY[2]));
            __m128i C2 = _mm_sub_epi32(_mm_mullo_epi32(fxPtX[0], fxPtY[1]), _mm_mullo_epi32(fxPtX[1], fxPtY[0]));

            // Compute triangle area
            __m128i triArea = _mm_mullo_epi32(B2, A1);
            triArea = _mm_sub_epi32(triArea, _mm_mullo_epi32(B1, A2));
            __m128 oneOverTriArea = _mm_div_ps(_mm_set1_ps(1.0f), _mm_cvtepi32_ps(triArea));

            __m128 Z[3];
            Z[0] = xformedPos[0].W;
            Z[1] = _mm_mul_ps(_mm_sub_ps(xformedPos[1].W, Z[0]), oneOverTriArea);
            Z[2] = _mm_mul_ps(_mm_sub_ps(xformedPos[2].W, Z[0]), oneOverTriArea);

            // Use bounding box traversal strategy to determine which pixels to rasterize 
            __m128i startX = _mm_and_si128(Max(Min(Min(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(0)), _mm_set1_epi32(~1));
            __m128i endX = Min(Max(Max(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(res.x - 1));

            __m128i startY = _mm_and_si128(Max(Min(Min(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(0)), _mm_set1_epi32(~1));
            __m128i endY = Min(Max(Max(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(res.y - 1));

            // Now we have 4 triangles set up.  Rasterize them each individually.
            for( int lane = 0; lane < 4; lane++ ) {
                // Skip triangle if area is zero 
                if( triArea.m128i_i32[lane] <= 0 ) {
                    continue;
                }

                // Extract this triangle's properties from the SIMD versions
                __m128 zz[3];
                for( int vv = 0; vv < 3; vv++ ) {
                    zz[vv] = _mm_set1_ps(Z[vv].m128_f32[lane]);
                }

                //drop culled triangle

                int startXx = startX.m128i_i32[lane];
                int endXx = endX.m128i_i32[lane];
                int startYy = startY.m128i_i32[lane];
                int endYy = endY.m128i_i32[lane];

                __m128i aa0 = _mm_set1_epi32(A0.m128i_i32[lane]);
                __m128i aa1 = _mm_set1_epi32(A1.m128i_i32[lane]);
                __m128i aa2 = _mm_set1_epi32(A2.m128i_i32[lane]);

                __m128i bb0 = _mm_set1_epi32(B0.m128i_i32[lane]);
                __m128i bb1 = _mm_set1_epi32(B1.m128i_i32[lane]);
                __m128i bb2 = _mm_set1_epi32(B2.m128i_i32[lane]);

                __m128i cc0 = _mm_set1_epi32(C0.m128i_i32[lane]);
                __m128i cc1 = _mm_set1_epi32(C1.m128i_i32[lane]);
                __m128i cc2 = _mm_set1_epi32(C2.m128i_i32[lane]);

                __m128i aa0Inc = _mm_mul_epi32(aa0, _mm_setr_epi32(1, 2, 3, 4));
                __m128i aa1Inc = _mm_mul_epi32(aa1, _mm_setr_epi32(1, 2, 3, 4));
                __m128i aa2Inc = _mm_mul_epi32(aa2, _mm_setr_epi32(1, 2, 3, 4));

                __m128i alpha0 = _mm_add_epi32(_mm_mul_epi32(aa0, _mm_set1_epi32(startXx)), _mm_mul_epi32(bb0, _mm_set1_epi32(startYy)));
                alpha0 = _mm_add_epi32(cc0, alpha0);
                __m128i beta0 = _mm_add_epi32(_mm_mul_epi32(aa1, _mm_set1_epi32(startXx)), _mm_mul_epi32(bb1, _mm_set1_epi32(startYy)));
                beta0 = _mm_add_epi32(cc1, beta0);
                __m128i gama0 = _mm_add_epi32(_mm_mul_epi32(aa2, _mm_set1_epi32(startXx)), _mm_mul_epi32(bb2, _mm_set1_epi32(startYy)));
                gama0 = _mm_add_epi32(cc2, gama0);

                int  rowIdx = (startYy * res.x + startXx);

                __m128 zx = _mm_mul_ps(_mm_cvtepi32_ps(aa1), zz[1]);
                zx = _mm_add_ps(zx, _mm_mul_ps(_mm_cvtepi32_ps(aa2), zz[2]));
                zx = _mm_mul_ps(zx, _mm_setr_ps(1.f, 2.f, 3.f, 4.f));

                // Texels traverse
                for( int r = startYy; r < endYy; r++,
                    rowIdx += res.x,
                    alpha0 = _mm_add_epi32(alpha0, bb0),
                    beta0 = _mm_add_epi32(beta0, bb1),
                    gama0 = _mm_add_epi32(gama0, bb2) ) {
                    // Compute barycentric coordinates
                    // Z0 as an origin
                    int index = rowIdx;
                    __m128i alpha = alpha0;
                    __m128i beta = beta0;
                    __m128i gama = gama0;

                    //Compute barycentric-interpolated depth
                    __m128 depth = zz[0];
                    depth = _mm_add_ps(depth, _mm_mul_ps(_mm_cvtepi32_ps(beta), zz[1]));
                    depth = _mm_add_ps(depth, _mm_mul_ps(_mm_cvtepi32_ps(gama), zz[2]));
                    __m128i anyOut = _mm_setzero_si128();

                    __m128i mask;
                    __m128 previousDepth;
                    __m128 depthMask;
                    __m128i finalMask;
                    for( int c = startXx; c < endXx;
                        c += 4,
                        index += 4,
                        alpha = _mm_add_epi32(alpha, aa0Inc),
                        beta = _mm_add_epi32(beta, aa1Inc),
                        gama = _mm_add_epi32(gama, aa2Inc),
                        depth = _mm_add_ps(depth, zx) ) {
                        mask = _mm_or_si128(_mm_or_si128(alpha, beta), gama);
                        previousDepth = _mm_loadu_ps(&(buffer[index]));

                        //calculate current depth
                        //(log(depth) - -6.907755375) * 0.048254941;
                        __m128 curdepth = _mm_mul_ps(_mm_sub_ps(log_ps(depth), _mm_set1_ps(-6.907755375)), _mm_set1_ps(0.048254941));
                        curdepth = _mm_sub_ps(curdepth, _mm_set1_ps(0.05));

                        depthMask = _mm_cmplt_ps(curdepth, previousDepth);
                        finalMask = _mm_andnot_si128(mask, _mm_castps_si128(depthMask));
                        anyOut = _mm_or_si128(anyOut, finalMask);

                    }//for each column  

                    if( !_mm_testz_si128(anyOut, _mm_set1_epi32(0x80000000)) ) {
                        // stop timer
                        /*QueryPerformanceCounter(&t2);

                        // compute and print the elapsed time in millisec
                        elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;

                        RasterizationStats::RasterizeSSETimeSpent += elapsedTime;
                        */
                        return true; //early exit
                    }

                }// for each row

            }// for each triangle
        }// for each set of SIMD# triangles

        return false;
    }

public:
    CoverageBuffer() {
        uint32_t Width = 1366 / 2;
        uint32_t Height = 768 / 2;

        _DepthDS = new Texture(tf_dim_2 | tf_UAV, DXGI_FORMAT_R32_FLOAT, Width, Height, 1u, 1u, "CBuffer Depth RW");
        _DepthDS_Read = new Texture(tf_dim_2, DXGI_FORMAT_R32_FLOAT, Width, Height, 1u, 1u, "CBuffer Depth Read");
        //_DepthDS = new Texture(Width, Height, DXGI_FORMAT_R32_FLOAT, true);
        //_DepthDS_Read = new Texture(Width, Height, DXGI_FORMAT_R32_FLOAT, false, false, true);

        //_DepthDS->SetName("CBuffer Depth RW");
        //_DepthDS_Read->SetName("CBuffer Depth Read");

        cbDownscaling = new ConstantBuffer();
        cbDownscaling->CreateDefault(sizeof(Downscaling));
        cbDownscaling->SetName("CBuffer Downscaling");

        shDownScaleDepthCS = new Shader();
        shDownScaleDepthCS->LoadFile("shDownScaleDepthCS.cso", Shader::Compute);

        shDownScaleDepthCS->ReleaseBlobs();

        pDepthBuffer = nullptr;
        mLastSize = 0u;
        mLastScaling = 100u;
    }

    ~CoverageBuffer() {
        _DepthDS->Release();
        _DepthDS_Read->Release();

        cbDownscaling->Release();

        shDownScaleDepthCS->Release();

        delete _DepthDS;
        delete _DepthDS_Read;

        delete cbDownscaling;

        delete shDownScaleDepthCS;

        if( pDepthBuffer && mLastSize != 0u ) delete[] pDepthBuffer;
    }

    void Resize(UINT Width, UINT Height) {
        float scale = 1.f / std::clamp(1 << mLastScaling, 1, 8);
        _DepthDS->Resize(Width * scale, Height * scale);
        _DepthDS_Read->Resize(Width * scale, Height * scale);

    }

    template<size_t dim, size_t BufferNum, bool DepthBuffer=false,
        size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps
                                                    to create per RT buffer   */
        bool WillHaveMSAA=false, bool Cube=false>
    void Prepare(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube> *RB, const CBuffArgs& args) {
        if constexpr( !DepthBuffer ) {
            std::cout << "C-Buffer::Prepare: No depth buffer provided!" << std::endl;
            return;
        }
        return;

        ScopedRangeProfiler s1(L"Coverage Buffer::Prepare");

        // Unbind views
        ID3D11RenderTargetView* nullRTV = nullptr;
        gDirectX->gContext->OMSetRenderTargets(1, &nullRTV, nullptr);

        // 
        float unrcpScale = std::clamp(1 << args.Scaling, 1, 8);
        float scale = 1.f / unrcpScale;

        float fRBW = (float)RB->GetWidth();
        float fRBH = (float)RB->GetHeight();
        fWidth  = fRBW * scale;
        fHeight = fRBH * scale;

        // Resize
        if( mLastScaling != args.Scaling ) {
            mLastScaling = args.Scaling;
            Resize(fRBW, fRBH);
        }

        // 
        float fQ = args._CameraFar / (args._CameraNear - args._CameraFar);

        // Update constant buffers
        Downscaling *inst0 = (Downscaling*)cbDownscaling->Map();
            inst0->_Res        = { (UINT)fWidth, (UINT)fHeight };
            inst0->_ResRcp     = { 1.f / fWidth, 1.f / fHeight };
            inst0->_ProjValues = { args._CameraNear * fQ, fQ };
            inst0->_DownScale  = unrcpScale;
            inst0->_DownScale2 = args.Scaling;
        cbDownscaling->Unmap();

        // Bind resources
        cbDownscaling->Bind(Shader::Compute, 0);  // CB
        RB->Bind(0u, Shader::Compute, 0);         // Texture2D; SRV // Depth
        _DepthDS->Bind(Shader::Compute, 0, true); // RWTexture2D; _Output

        // Dispatch
        shDownScaleDepthCS->Dispatch((UINT)ceil(fRBW / 32.f), (UINT)ceil(fRBH / 32.f), 1);

        // Unbind slots
        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB <1>();

        // 
        {
            ScopedRangeProfiler s2(L"Copy depth");

            // Copy data over to another texture to read it with CPU
            _DepthDS_Read->Copy(_DepthDS);

            D3D11_MAPPED_SUBRESOURCE Mapped;
            // 
            gDirectX->gContext->Map(_DepthDS_Read->GetTexture(), 0, D3D11_MAP_READ, 0, &Mapped);

                // Calculate size
                size_t size = (size_t)Mapped.RowPitch * (size_t)ceil(fHeight);

                // Resize
                if( !pDepthBuffer ) {
                    // Allocate new
                    pDepthBuffer = new float[size];
                    mLastSize = size;
                } else if( mLastSize != size ) {
                    // Re-allocate
                    delete[] pDepthBuffer;
                    pDepthBuffer = new float[size];
                    mLastSize = size;
                }

                // Copy
                memcpy(&pDepthBuffer[0], Mapped.pData, size);

            // Done
            gDirectX->gContext->Unmap(_DepthDS_Read->GetTexture(), 0);
        }
    }

    
};
