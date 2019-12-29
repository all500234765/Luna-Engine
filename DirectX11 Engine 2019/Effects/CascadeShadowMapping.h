#pragma once

#include "pc.h"
#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Materials/Texture.h"
#include "Engine/Utility/Utils.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine/States/BlendState.h"
#include "Engine/States/DepthStencilState.h"
#include "Engine/States/RasterState.h"
#include "Engine/States/TopologyState.h"
#include "Engine/Materials/Sampler.h"
#include "Engine/Camera/Camera.h"

struct CSMArgs {
    // 2u < _CascadeNum <= CascadeMaxNum
    UINT _CascadeNum = 3;

    // Level = [1; 32]
    UINT _MSAALevel = 8;

    // WillHaveMSAA must be true
    bool _MSAA = false;

    // 
    bool _Antiflicker = true;

    // Shadow map resolution for each cascade
    float _Resolution = 2048;
    
    // 
    float _CascadeRange[3 + 1];
};

// CascadeMaxNum = [2; 3]
template<UINT CascadeMaxNum=3u, bool WillHaveMSAA=false>
class CascadeShadowMapping {
private:
    struct CSM {
        mfloat4x4 _ViewProj[CascadeMaxNum];
    };

    RenderTarget<2, 0, true, CascadeMaxNum, WillHaveMSAA> *rtCSM;

    Shader *shCSM;
    //Shader *shCSMAnimated;

    ConstantBuffer *cbCSM;

    // Cascade parameters
    UINT mCascadeNum = CascadeMaxNum;
    mfloat3 mShadowBoundCenter;
    float1 mShadowBoundRadius;

    mfloat3 mShadowBoundCenterC[CascadeMaxNum];
    float2 mCascadeOffset      [CascadeMaxNum];
    float  mCascadeScaleV      [CascadeMaxNum];
    float  mShadowBoundRadiusC [CascadeMaxNum];

    mfloat4x4 World2Shadow;

    // Camera parameters
    float1 fFOV, fAspect;
    float4x4 LightWorld;

    void SetCascadeNum(UINT Cascades) {
        UINT old = mCascadeNum;

        // Set & Clamp
        mCascadeNum = std::clamp(Cascades, 2u, CascadeMaxNum);

        // Reload shader
        if( mCascadeNum != old ) {
            using namespace std::string_literals;
            shCSM->Reload("shCSM"s + std::to_string(mCascadeNum) + "GS.cso"s, Shader::Geometry);
        }
    }
    
    // level = [1; 32]
    inline void SetMSAAMaxLevel(UINT level) const {
        if( !WillHaveMSAA ) { return; }

        // Set MSAA Max Level
        rtCSM->SetMSAAMaxLevel(level);

        // Re-create CSM buffers
        if( rtCSM->IsMSAAEnabled() ) {
            UINT w = (UINT)rtCSM->GetWidth();
            UINT h = (UINT)rtCSM->GetHeight();

            rtCSM->Resize(w, h);
        }
    }

    inline void SetMSAA(bool MSAA) const {
        if( !WillHaveMSAA ) { return; }

        // Toggle MSAA
        if( MSAA ) {
            if( !rtCSM->IsMSAAEnabled() ) rtCSM->EnableMSAA();
            else return;
        } else {
            if( rtCSM->IsMSAAEnabled() ) rtCSM->DisableMSAA();
            else return;
        }

        // Re-create CSM buffers
        UINT w = (UINT)rtCSM->GetWidth();
        UINT h = (UINT)rtCSM->GetHeight();

        rtCSM->Resize(w, h);
    }

    struct FBS {
        mfloat3 Center;
        float1  Radius;

        FBS(mfloat3 C, float1 R): Center(C), Radius(R) {};
    };

    FBS FrustrumBoundingSphere(float1 fNear, float1 fFar) const {
        // 
        mfloat3 Pos     = { LightWorld._41, LightWorld._42, LightWorld._43 };
        mfloat3 Right   = { LightWorld._11, LightWorld._12, LightWorld._13 };
        mfloat3 Up      = { LightWorld._21, LightWorld._22, LightWorld._23 };
        mfloat3 Forward = { LightWorld._31, LightWorld._32, LightWorld._33 };

        float1 TanFoVx = tan(fAspect * fFOV);
        float1 TanFoVy = tan(fAspect);

        float q = (fNear + .5f * (fNear + fFar));
        mfloat3 Center = { Pos.m128_f32[0] + Forward.m128_f32[0] * q, 
                           Pos.m128_f32[1] + Forward.m128_f32[1] * q, 
                           Pos.m128_f32[2] + Forward.m128_f32[2] * q };
        mfloat3 q2 = { fFar * (Forward.m128_f32[0] + Up.m128_f32[0] * TanFoVy - Right.m128_f32[0] * TanFoVx), 
                       fFar * (Forward.m128_f32[1] + Up.m128_f32[1] * TanFoVy - Right.m128_f32[1] * TanFoVx), 
                       fFar * (Forward.m128_f32[2] + Up.m128_f32[2] * TanFoVy - Right.m128_f32[2] * TanFoVx)};
        // fFar* (Forward + Up * TanFoVy - Right * TanFoVx);
        mfloat3 dc = { Pos.m128_f32[0] - Center.m128_f32[0] + q2.m128_f32[0], 
                       Pos.m128_f32[1] - Center.m128_f32[1] + q2.m128_f32[1],
                       Pos.m128_f32[2] - Center.m128_f32[2] + q2.m128_f32[2] }; // Pos - Center + q2
        float1 Radius = DirectX::XMVector3Length(dc).m128_f32[0];

        return { Center, Radius };
    }

    struct CNU {
        mfloat3 Offset;
        bool Res;

        CNU(mfloat3 O, bool R): Offset(O), Res(R) {};
    };

    CNU CascadeNeedsUpdate(const mfloat4x4& mShadowView, UINT CascadeIdx, const mfloat3& NewCenter) const {
        mfloat3 OldCenterInC = DirectX::XMVector3TransformCoord(mShadowBoundCenterC[CascadeIdx], mShadowView);
        mfloat3 NewCenterInC = DirectX::XMVector3TransformCoord(NewCenter, mShadowView);

        mfloat3 Diff = { NewCenterInC.m128_f32[0] - OldCenterInC.m128_f32[0], 
                         NewCenterInC.m128_f32[1] - OldCenterInC.m128_f32[1],
                         NewCenterInC.m128_f32[2] - OldCenterInC.m128_f32[2] };

        float1 PixelSize = rtCSM->GetWidth() / (2.f * mShadowBoundRadiusC[CascadeIdx]);

        float1 PixelOffX = Diff.m128_f32[0] * PixelSize;
        float1 PixelOffY = Diff.m128_f32[1] * PixelSize;

        bool Update = abs(PixelOffX) > .5f || abs(PixelOffY) > .5f;
        mfloat3 Offset = { 0.f, 0.f, 0.f };

        if( Update ) {
            Offset.m128_f32[0] = floorf(.5f + PixelOffX) / PixelSize;
            Offset.m128_f32[1] = floorf(.5f + PixelOffY) / PixelSize;
            Offset.m128_f32[2] = Diff.m128_f32[2];
        }

        return { Offset, Update };
    }

public:
    CascadeShadowMapping(UINT w=2048u, UINT CascadeNum=CascadeMaxNum, bool MSAA=false) {
        // Create Render Target
        rtCSM = new RenderTarget<2, 0, true, CascadeMaxNum, WillHaveMSAA>(w, w, 1.f, "CSM");
        if( MSAA ) rtCSM->EnableMSAA();
        rtCSM->Create(32u);
        
        // Load shader
        using namespace std::string_literals;

        shCSM = new Shader();
        //shCSM->DontTouch({ Shader::Hull, Shader::Domain });
        shCSM->LoadFile("shCSMVS.cso"s, Shader::Vertex);
        shCSM->LoadFile("shCSM"s + std::to_string(CascadeNum) + "GS.cso"s, Shader::Geometry);
        shCSM->SetNullShader(Shader::Pixel);
        shCSM->ReleaseBlobs();

        // Create constant buffer
        cbCSM = new ConstantBuffer();
        cbCSM->CreateDefault(sizeof(CSM));
    }

    ~CascadeShadowMapping() {
        SAFE_RELEASE(rtCSM);
        
        SAFE_RELEASE(shCSM);


        SAFE_RELEASE(cbCSM);


    }

    ID3D11ShaderResourceView* getSRV() const {
        return rtCSM->GetBufferSRV<0, true>();
    }

    void Begin(const Camera* light, const CSMArgs& args) {
        using namespace DirectX;

        // Total range for cascades
        // sum of _CascadeRange[]
        float _CascadeTotalRange = 0.f;
        for( size_t i = 0; i < args._CascadeNum; i++ ) _CascadeTotalRange += args._CascadeRange[i];

        // Reload shader
        SetCascadeNum(args._CascadeNum);
        SetMSAA(args._MSAA);
        SetMSAAMaxLevel(args._MSAALevel);
        //SetResolution(args._Resolution);

        // Begin updating CSM matrices
        CSM* inst = (CSM*)cbCSM->Map();

        // Calculate ViewProj matrices for each cascade
        CameraConfig ccfg = light->GetParams();
        float1 fFar       = ccfg.fFar;
        float1 fNear      = ccfg.fNear;
        float3 p          = light->GetPosition();

        mfloat4x4 mLightView  = light->GetViewMatrix();
        mfloat4x4 mLightProj  = light->GetProjMatrix();
        mfloat4x4 mLightWorld = XMMatrixInverse(&XMMatrixDeterminant(mLightView), mLightView);

        XMStoreFloat4x4(&LightWorld, mLightWorld);

        // Get shadow space bounds
        auto[vCenter, fRadius] = FrustrumBoundingSphere(args._CascadeRange[0], args._CascadeRange[mCascadeNum]);
        mShadowBoundRadius = std::max(mShadowBoundRadius, fRadius);

        // Calculate view matrix
        // Eye pos + Eye Direction * Total Cascade Range * .5f
        float q = _CascadeTotalRange * .5f;
        mfloat3 ep = { p.x, p.y, p.z }; //{ LightWorld._41, LightWorld._42, LightWorld._43 }; //reinterpret_cast<const float3*>(&LightWorld._41);
        mfloat3 ed = { LightWorld._31, LightWorld._32, LightWorld._33 }; //reinterpret_cast<const float3*>(&LightWorld._31);
        mfloat3 WorldCenter = ep - 0*ed * mfloat3({ q, q, q }) + 0*ed * fFar;
        mfloat3 Pos         = WorldCenter + ed * mShadowBoundRadius;
        mfloat3 LookAt      = WorldCenter;
        mfloat3 Right       = { -LightWorld._11, -LightWorld._12, -LightWorld._13 }; //{ 1.f, 0.f, 0.f };
        
        mfloat3 Up = { LightWorld._21, LightWorld._22, LightWorld._23 }; // 
        //Up = XMVector3Normalize(XMVector3Cross(ed, Right));

        mfloat4x4 mShadowView = XMMatrixLookAtRH(Pos, LookAt, Up);

        // Calculate projection matrix
        mfloat4x4 mShadowProj = XMMatrixOrthographicRH(mShadowBoundRadius,  mShadowBoundRadius, 
                                                       mShadowBoundRadius, -mShadowBoundRadius);

        // From world to shadow space
        World2Shadow = mShadowView * mShadowProj;
        
        // For each cascade calculate transformation
        // From shadow to cascade space
        mfloat4x4 mShadowInvView = XMMatrixTranspose(mShadowView);

// https://github.com/YeeYoungHan/HLSL-Development-Cookbook/blob/1529c9854c35f82c3538cf9c30376c2b17b0047e/Chpater%203%20-%20Shadow%20Mapping/Cascaded%20Shadow%20Maps/CascadedMatrixSet.cpp#L64

        for( UINT CascadeIdx = 0; CascadeIdx < mCascadeNum; CascadeIdx++ ) {
            mfloat4x4 mCascadeTrans, mCascadeScale;

            if( args._Antiflicker ) {
                // To avoid anti flickering we need to make the transformation invariant to camera rotation and translation
                // By encapsulating the cascade frustum with a sphere we achive the rotation invariance
                auto[NewCenter, fRadius2] = 
                    FrustrumBoundingSphere(args._CascadeRange[CascadeIdx], args._CascadeRange[CascadeIdx + 1]);
                mShadowBoundRadiusC[CascadeIdx] = std::max(mShadowBoundRadiusC[CascadeIdx], fRadius2);

                // Only update the cascade bounds if it moved at least a full pixel unit
                // This makes the transformation invariant to translation
                auto[Offset, Res] = CascadeNeedsUpdate(mShadowView, CascadeIdx, NewCenter);
                    
                if( Res ) {
                    // To avoid flickering we need to move the bound center in full units
                    mfloat3 OffsetOut = XMVector3TransformNormal(Offset, mShadowInvView);
                    mShadowBoundCenterC[CascadeIdx] += OffsetOut;
                }

                // Get the cascade center in shadow space
                mfloat3 CenterShadowSpace = XMVector3TransformCoord(mShadowBoundCenterC[CascadeIdx], World2Shadow);

                // Update the translation from shadow to cascade space
                mCascadeOffset[CascadeIdx] = { -CenterShadowSpace.m128_f32[0], 
                                               -CenterShadowSpace.m128_f32[1] };

                // 
                mfloat3 forw = { mCascadeOffset[CascadeIdx].x, mCascadeOffset[CascadeIdx].y, 0.f };
                // { 0.f, 0.f, args._CascadeRange[CascadeIdx] };

                mCascadeTrans = XMMatrixTranslation(forw.m128_f32[0], forw.m128_f32[1], forw.m128_f32[2]);
                //mCascadeTrans = XMMatrixTranslation(mCascadeOffset[CascadeIdx].x, 0.f, mCascadeOffset[CascadeIdx].y);

                // Update the scale from shadow to cascade space
                mCascadeScaleV[CascadeIdx] = mShadowBoundRadius / mShadowBoundRadiusC[CascadeIdx];
                mCascadeScale = XMMatrixScaling(0+1*mCascadeScaleV[CascadeIdx], 0+1*mCascadeScaleV[CascadeIdx], 1.f);
            } else {
                // Since we don't care about flickering we can make the cascade fit tightly around the frustum
                // Extract the bounding box


                // Transform to shadow space and extract the minimum andn maximum


                // Update the translation from shadow to cascade space


                // Update the scale from shadow to cascade space


            }

            // Combine the matrices to get the transformation from world to cascade space
            inst->_ViewProj[0*2 + CascadeIdx] = (World2Shadow * mCascadeTrans * mCascadeScale);
        }

        // Set the values for the unused slots to someplace outside the shadow space
        for( int i = mCascadeNum; i < 4; i++ ) {
            mCascadeOffset[i] = { 250.f, 250.f };
            mCascadeScaleV[i] = .1f;
        }

        // Update constant buffer
        cbCSM->Unmap();

        // Bind resources
        cbCSM->Bind(Shader::Geometry, 0); // CB

        // 
        rtCSM->Clear(0.f, 0);
        rtCSM->Bind();

        shCSM->Bind();
    }

    // Bind your default rendering shader for used mesh
    // It shouldn't use geometry shader 
    // Before calling
    void Prepare() {
        // Bind geometry shader
        //shCSM->Bind();
    }


};
