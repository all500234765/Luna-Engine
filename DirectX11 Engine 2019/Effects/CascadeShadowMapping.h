#pragma once

#include "Engine Includes/MainInclude.h"

struct CSMArgs {
    // 2u < _CascadeNum <= CascadeMaxNum
    UINT _CascadeNum = 3;

    // Level = [1; 32]
    UINT _MSAALevel = 8;

    // WillHaveMSAA must be true
    bool MSAA = false;


};

// CascadeMaxNum = [2; 3]
template<UINT CascadeMaxNum=3u, bool WillHaveMSAA=false>
class CascadeShadowMapping {
private:
    struct CSM {
        mfloat4x4 _ViewProj[CascadeMaxNum];
    };

    RenderTarget<2, 0, true, CascadeMaxNum, WillHaveMSAA> *rtCSM;

    Shader *shCSMSurface;
    //Shader *shCSMAnimated;

    ConstantBuffer *cbCSM;

    UINT mCascadeNum = CascadeMaxNum;

    void SetCascadeNum(UINT Cascades) {
        UINT old = mCascadeNum;

        // Set & Clamp
        mCascadeNum = std::clamp(Cascades, 2u, CascadeMaxNum);

        // Reload shader
        if( mCascadeNum != old ) {
            using namespace std::string_literals;
            shCSMSurface->Reload("shCSM"s + std::to_string(mCascadeNum) + "GS.cso"s, Shader::Geometry);
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

public:
    CascadeShadowMapping(UINT w=2048u, UINT CascadeNum=CascadeMaxNum, bool MSAA=false) {
        // Create Render Target
        rtCSM = new RenderTarget<2, 0, true, CascadeMaxNum, WillHaveMSAA>(w, w, 1.f, "CSM");
        if( MSAA ) rtCSM->EnableMSAA();
        rtCSM->Create(32u);
        
        // Load shader
        using namespace std::string_literals;

        shCSMSurface = new Shader();
        shCSMSurface->LoadFile("shSurfaceVS.cso"s, Shader::Vertex);
        shCSMSurface->LoadFile("shCSM"s + std::to_string(CascadeNum) + "GS.cso"s, Shader::Geometry);
        shCSMSurface->SetNullShader(Shader::Pixel);
        shCSMSurface->ReleaseBlobs();

        // Create constant buffer
        cbCSM = new ConstantBuffer();
    }

    ~CascadeShadowMapping() {
        SAFE_RELEASE(rtCSM);
        
        SAFE_RELEASE(shCSMSurface);


        SAFE_RELEASE(cbCSM);


    }

    void BeginSurface(Camera* light, const CSMArgs& args) {
        // Reload shader
        SetCascadeNum(args._CascadeNum);

        // Bind shader
        shCSMSurface->Bind();

        // Calculate ViewProj matrices for each cascade
        cbCSM->Map();

        // Bind resources
        light->BindBuffer(Shader::Vertex, 0); // CB
        cbCSM->Bind(Shader::Geometry, 0);     // CB



    }

};
