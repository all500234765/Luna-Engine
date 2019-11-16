#pragma once

#include "Engine Includes/MainInclude.h"

struct SSAOArgs {
    mfloat4x4 _mView;   // View matrix
    mfloat4x4 _mProj;   // Proj matrix
    float _Radius;      // Sphere radius
    float _OffsetRad;   // Radius for random points
    float _Power;       // SSAO Power to raise to
    float _CameraNear;  // Camera's near clip plane
    float _CameraFar;   // Camera's far clip plane
    bool _Blur;         // Apply blur or not
};

// TODO: Try blue noise

class SSAOPostProcess {
private:
    struct BlurArgs {
        // Res of downscaled target: x - width, y - height
        uint2 _Res; // Backbuffer / 4

        // Total pixels in the downscaled img
        uint _Domain; // Res.x * Res.y

        // Number of groups dispatched on 1st pass
        uint _GroupSize; // Domain / 1024

        // Unused for this effect
        float4 _Empty;
    };

    struct Downscaling {
        uint2  _Res;        // Resolution
        float2 _ResRcp;     // 1.f / _Res
        float4 _ProjValues; // 
        float _OffsetRad;   // Radius for random points
        float _Radius;      // Sphere radius
        float _SSAOPower;   // SSAO Power to raise to
        float _NoiseSize;   // Empty
        mfloat4x4 _mView;   // View matrix
    };

    Shader *shDownscaleDepthNormal; // Dispatch(Width * Height / (4 * 1024), 1, 1);
    Shader *shSSAO;                 // Dispatch(Width * Height / (4 * 1024), 1, 1);
    Shader *shVerticalFilter;       // Dispatch(Width / 16, Height / (16 * (128 - 12)) + 1, 1);
    Shader *shHorizontalFilter;     // Dispatch(ceil(Width / (16 * (128 - 12))), Height / 16, 1);

    StructuredBuffer<float4> *sbDepthNDS; // r=linear depth, gba=normal;

    Texture *_BlueNoise;
    Texture *_SSAOInter;
    Texture *_SSAO;

    ConstantBuffer *cbBlurArgs;
    ConstantBuffer *cbDownscaling; // Depth/Normal

    // Saves last RT's size.
    // So we can do something later
    float fWidth;
    float fHeight;

    UINT mSlot = 0; // Last bound slot
public:
    SSAOPostProcess() {
        uint32_t Width = 1366 / 2;
        uint32_t Height = 768 / 2;
        
        shDownscaleDepthNormal = new Shader();
        shDownscaleDepthNormal->LoadFile("shDownscaleDepthNormalCS.cso", Shader::Compute);
        shDownscaleDepthNormal->ReleaseBlobs();
        
        shHorizontalFilter = new Shader();
        shHorizontalFilter->LoadFile("shHorizontalFilterCS.cso", Shader::Compute);
        shHorizontalFilter->ReleaseBlobs();

        shVerticalFilter = new Shader();
        shVerticalFilter->LoadFile("shVerticalFilterCS.cso", Shader::Compute);
        shVerticalFilter->ReleaseBlobs();

        shSSAO = new Shader();
        shSSAO->LoadFile("shSSAOCS.cso", Shader::Compute);
        shSSAO->ReleaseBlobs();

        sbDepthNDS = new StructuredBuffer<float4>();
        sbDepthNDS->CreateDefault(Width * Height, nullptr, true);

        _SSAO      = new Texture(Width, Height, DXGI_FORMAT_R32_FLOAT, true);
        _SSAOInter = new Texture(Width, Height, DXGI_FORMAT_R32_FLOAT, true);
        _BlueNoise = new Texture();
        
        _BlueNoise->Load("../Textures/Noise/Blue/LDR_RG01_0.png", DXGI_FORMAT_R16G16_UNORM);

        cbDownscaling = new ConstantBuffer();
        cbDownscaling->CreateDefault(sizeof(Downscaling));

        cbBlurArgs = new ConstantBuffer();
        cbBlurArgs->CreateDefault(sizeof(BlurArgs));
    }

    ~SSAOPostProcess() {
        shDownscaleDepthNormal->Release();
        shHorizontalFilter->Release();
        shVerticalFilter->Release();
        shSSAO->Release();

        cbDownscaling->Release();
        cbBlurArgs->Release();

        sbDepthNDS->Release();

        _BlueNoise->Release();
        _SSAOInter->Release();
        _SSAO->Release();

        delete shDownscaleDepthNormal;
        delete shHorizontalFilter;
        delete shVerticalFilter;
        delete shSSAO;

        delete cbDownscaling;
        delete cbBlurArgs;
        
        delete sbDepthNDS;

        delete _BlueNoise;
        delete _SSAOInter;
        delete _SSAO;
    }

    void Resize(UINT Width, UINT Height) {
        _SSAO->Resize(Width / 2, Height / 2);
        _SSAOInter->Resize(Width / 2, Height / 2);

        // Re-create structured buffer
        sbDepthNDS->Release();
        sbDepthNDS->CreateDefault(Width * Height / 4, nullptr, true);
    }

    // Normals must be encoded in spherical coordinates
    // RenderTarget is used to get depth buffer and size
    // Depth and Normal buffers must be same size
    template<size_t dim, size_t BufferNum, bool DepthBuffer=false,
             size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps
                                                         to create per RT buffer   */
             bool WillHaveMSAA=false, bool Cube=false>
    void Begin(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube> *RB, const SSAOArgs& args) {
        ScopedRangeProfiler s1(L"SSAO Pass");

        // Unbind views
        ID3D11RenderTargetView* nullRTV = nullptr;
        gDirectX->gContext->OMSetRenderTargets(1, &nullRTV, nullptr);

        // 
        fWidth  = (float)RB->GetWidth();
        fHeight = (float)RB->GetHeight();

        UINT X = UINT(ceil(fWidth * fHeight / (4.f * 1024.f)));
        float fQ = args._CameraFar / (args._CameraNear - args._CameraFar);

        float4x4 dest;
        DirectX::XMStoreFloat4x4(&dest, DirectX::XMMatrixTranspose(args._mProj));

        // Update constant buffers
        Downscaling *inst0 = (Downscaling*)cbDownscaling->Map();
            inst0->_mView      = (args._mView);
            inst0->_Res        = { (UINT)fWidth / 2, (UINT)fHeight / 2 };
            inst0->_ResRcp     = { 2.f / fWidth, 2.f / fHeight };
            inst0->_Radius     = args._Radius;
            inst0->_OffsetRad  = args._OffsetRad;
            inst0->_SSAOPower  = args._Power;
            inst0->_ProjValues = { 1.f / dest.m[0][0], 1.f / dest.m[1][1], args._CameraNear * fQ, fQ };
            inst0->_NoiseSize  = _BlueNoise->GetWidth();
        cbDownscaling->Unmap();

        BlurArgs *inst1 = (BlurArgs*)cbBlurArgs->Map();
            inst1->_Res       = inst0->_Res;
            inst1->_Domain    = inst1->_Res.x * inst1->_Res.y;
            inst1->_GroupSize = inst1->_Domain / 1024;
        cbBlurArgs->Unmap();

        /////////////////////////////////////// Depth Normal Downscaling ///////////////////////////////////////
        sbDepthNDS->Bind(Shader::Compute, 0, true); // UAV
        cbDownscaling->Bind(Shader::Compute, 0);    // CB
        RB->Bind(0u, Shader::Compute, 0);           // Texture2D; SRV // Depth
        RB->Bind(2u, Shader::Compute, 1);           // Texture2D; SRV // Normals

        shDownscaleDepthNormal->Dispatch(X, 1, 1);

        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<2>();
        LunaEngine::CSDiscardCB<1>();

        /////////////////////////////////////// SSAO Computation ///////////////////////////////////////
        _SSAO->Bind(Shader::Compute, 0, true);   // RWTexture2D; UAV
        cbDownscaling->Bind(Shader::Compute, 0); // CB
        sbDepthNDS->Bind(Shader::Compute, 0);    // SRV
        _BlueNoise->Bind(Shader::Compute, 1);    // SRV

        shSSAO->Dispatch(X, 1, 1);

        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<1>();
        LunaEngine::CSDiscardCB<1>();

        //////////////////////////// SSAO Blur ////////////////////////////
        if( args._Blur ) {
            ScopedRangeProfiler s1(L"SSAO Blur");

            // Horizontal pass
            cbBlurArgs->Bind(Shader::Compute, 0);       // CB
            _SSAO->Bind(Shader::Compute, 0);            // Texture2D; _Input
            _SSAOInter->Bind(Shader::Compute, 0, true); // RWTexture2D; _Output

            shHorizontalFilter->Dispatch((UINT)ceil(fWidth / (2.f * (128.f - 12.f))), (UINT)ceil(fHeight / 2.f), 1);

            // Unbind slots
            LunaEngine::CSDiscardUAV<1>();
            LunaEngine::CSDiscardSRV<1>();
            LunaEngine::CSDiscardCB <1>();

            // Vertical pass
            cbBlurArgs->Bind(Shader::Compute, 0);   // CB
            _SSAOInter->Bind(Shader::Compute, 0);   // Texture2D; _Input
            _SSAO->Bind(Shader::Compute, 0, true);  // RWTexture2D; _Output

            shVerticalFilter->Dispatch((UINT)ceil(fWidth / 2.f), (UINT)ceil(fHeight / (2.f * (128.f - 12.f))), 1);

            // Unbind slots
            LunaEngine::CSDiscardUAV<1>();
            LunaEngine::CSDiscardSRV<1>();
            LunaEngine::CSDiscardCB <1>();
        }
    }

    void BindAO(Shader::ShaderType shader=Shader::Pixel, UINT slot=8) {
        //_SSAO
        _SSAO->Bind(shader, slot, false);
        mSlot = slot;
    }

    void End() {
        ScopedRangeProfiler s1(L"SSAO Unbind");

        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->PSSetShaderResources(mSlot, 1, &pEmpty);
    }

    inline ID3D11ShaderResourceView *GetSSAOSRV() const { return _SSAO->GetSRV(); }
};
