#pragma once

#include "Engine Includes/MainInclude.h"

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
    
    // Total range for cascades
    // sum of _CascadeRange[]
    float _CascadeTotalRange;

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
    float3 mShadowBoundCenter;
    float1 mShadowBoundRadius;

    float3 mShadowBoundCenterC[CascadeMaxNum];
    float2 mCascadeOffset     [CascadeMaxNum];
    float1 mCascadeScale      [CascadeMaxNum];
    float1 mShadowBoundRadiusC[CascadeMaxNum];

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
        float3 Center;
        float1 Radius;
    };

    FBS&& FrustrumBoundingSphere(float1 fNear, float1 fFar) const {
        // 
        float3 Pos     = reinterpret_cast<const float3*>(&LightWorld._41);
        float3 Right   = reinterpret_cast<const float3*>(&LightWorld._11);
        float3 Up      = reinterpret_cast<const float3*>(&LightWorld._21);
        float3 Forward = reinterpret_cast<const float3*>(&LightWorld._31);

        float1 TanFoVx = tan(fAspect * fFOV);
        float1 TanFoVy = tan(fAspect);

        float3 Center = Pos + Forward * (fNear + .5f * (fNear + fFar));
        float1 Radius = DirectX::XMVector3Length(Pos - Center + fFar * (Forward + Up * TanFoVy - Right * TanFoVx));

        return { Center, Radius };
    }

    struct CNU {
        float3 Offset;
        bool Res;
    };

    CNU&& CascadeNeedsUpdate(const mfloat4x4& mShadowView, UINT CascadeIdx, const float3& NewCenter) const {
        float3 OldCenterInC = DirectX::XMVector3TransformCoord(mShadowBoundCenterC[CascadeIdx], mShadowView);
        float3 NewCenterInC = DirectX::XMVector3TransformCoord(NewCenter, mShadowView);

        float3 Diff = NewCenterInC - OldCenterInC;

        float1 PixelSize = rtCSM->GetWidth() / (2.f * mShadowBoundRadiusC[CascadeIdx]);

        float1 PixelOffX = Diff.x * PixelSize;
        float1 PixelOffY = Diff.y * PixelSize;

        bool Update = abs(PixelOffX) > .5f || abs(PixelOffY) > .5f;
        float3 Offset = 0.f;

        if( Update ) {
            Offset.x = floorf(.5f + PixelOffX) / PixelSize;
            Offset.y = floorf(.5f + PixelOffY) / PixelSize;
            Offset.z = Diff.z;
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
        shCSM->DontTouch({ Shader::Vertex, Shader::Hull, Shader::Domain });
        shCSM->LoadFile("shCSM"s + std::to_string(CascadeNum) + "GS.cso"s, Shader::Geometry);
        shCSM->SetNullShader(Shader::Pixel);
        shCSM->ReleaseBlobs();

        // Create constant buffer
        cbCSM = new ConstantBuffer();
    }

    ~CascadeShadowMapping() {
        SAFE_RELEASE(rtCSM);
        
        SAFE_RELEASE(shCSM);


        SAFE_RELEASE(cbCSM);


    }

    void Begin(const Camera* light, const CSMArgs& args) {
        using namespace DirectX;

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
        float3 Direction  = light->GetRotation();

        mfloat4x4 mLightView  = light->GetViewMatrix();
        mfloat4x4 mLightProj  = light->GetProjMatrix();
        mfloat4x4 mLightWorld = XMMatrixInverse(XMMatrixDeterminant(mLightView), mLightView);

        XMStoreFloat4x4(&LightWorld, mLightWorld);

        // Calculate view matrix
        // Eye pos + Eye Direction * Total Cascade Range * .5f
        float3 WorldCenter = reinterpret_cast<const float3*>(&LightWorld._41) + 
                             reinterpret_cast<const float3*>(&LightWorld._31) * args._CascadeTotalRange * .5f;
        float3 Pos         = WorldCenter;
        float3 LookAt      = WorldCenter + Direction * fFar;
        float3 Right       = { 1.f, 0.f, 0.f };

        float3 Up = XMVector3Normalize(XMVector3Cross(Direction, Right));

        mfloat4x4 mShadowView = XMMatrixLookAtLH(Pos, LookAt, Up);

        // Get shadow space bounds
        float1 fRadius;
        float3 vCenter;

        auto[vCenter, fRadius] = FrustrumBoundingSphere(args._CascadeRange[0], args._CascadeRange[mCascadeNum]);
        mShadowBoundRadius = std::max(mShadowBoundRadius, fRadius);

        // Calculate projection matrix
        mfloat4x4 mShadowProj = XMMatrixOrthographicLH(mShadowBoundRadius, mShadowBoundRadius, 
                                                      -mShadowBoundRadius, mShadowBoundRadius);

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
                float3 NewCenter;
                auto[NewCenter, fRadius] = 
                    FrustrumBoundingSphere(args._CascadeRange[CascadeIdx], args._CascadeRange[CascadeIdx + 1]);
                mShadowBoundRadiusC[CascadeIdx] = std::max(mShadowBoundRadiusC[CascadeIdx], fRadius);

                // Only update the cascade bounds if it moved at least a full pixel unit
                // This makes the transformation invariant to translation
                float3 Offset;
                bool Res;
                auto[Offset, Res] = CascadeNeedsUpdate(mShadowView, CascadeIdx, NewCenter);
                    
                if( Res ) {
                    // To avoid flickering we need to move the bound center in full units
                    float3 OffsetOut = XMVector3TransformNormal(Offset, mShadowInvView);
                    mShadowBoundRadiusC[CascadeIdx] += OffsetOut;
                }

                // Get the cascade center in shadow space
                float3 CenterShadowSpace = XMVector3TransformCoord(mShadowBoundCenterC[CascadeIdx], World2Shadow);

                // Update the translation from shadow to cascade space
                mCascadeOffset[CascadeIdx] = -CenterShadowSpace;
                mCascadeTrans = XMMatrixTranslation(mCascadeOffset[CascadeIdx].x, mCascadeOffset[CascadeIdx].y, 0.f);

                // Update the scale from shadow to cascade space
                mCascadeScale[CascadeIdx] = mShadowBoundRadius / mShadowBoundRadiusC[CascadeIdx];
                XMMatrixScaling(mCascadeScale[CascadeIdx], mCascadeScale[CascadeIdx], 1.f);
            } else {
                // Since we don't care about flickering we can make the cascade fit tightly around the frustum
                // Extract the bounding box


                // Transform to shadow space and extract the minimum andn maximum


                // Update the translation from shadow to cascade space


                // Update the scale from shadow to cascade space


            }

            // Combine the matrices to get the transformation from world to cascade space
            inst->_ViewProj[CascadeIdx] = XMMatrixTranspose(World2Shadow * mCascadeTrans * mCascadeScale);
        }

        // Set the values for the unused slots to someplace outside the shadow space
        for( int i = mCascadeNum; i < 4; i++ ) {
            mCascadeOffset[i] = 250.f;
            mCascadeScale[i]  = .1f;
        }

        // Update constant buffer
        cbCSM->Unmap();

        // Bind resources
        cbCSM->Bind(Shader::Geometry, 0); // CB
    }

    // Bind your default rendering shader for used mesh
    // It shouldn't use geometry shader 
    // Before calling
    void Prepare() {
        // Bind geometry shader
        shCSM->Bind();
    }


};
