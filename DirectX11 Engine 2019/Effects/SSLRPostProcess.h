#pragma once

#include "pc.h"
#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Scene/Texture.h"
#include "Engine/Utility/Utils.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine/States/BlendState.h"
#include "Engine/States/DepthStencilState.h"
#include "Engine/Scene/Sampler.h"
#include "HighLevel/DirectX/Utlities.h"
#include "EffectBase.h"

struct SSLRArgs {
    float1 _ViewAngleThreshold;
    float1 _EdgeDistThreshold;
    float1 _DepthBias;
    float1 _ReflScale;
    float4 _ProjValues;
};

class SSLRPostProcess: public CallbackResize, EffectBase {
private:
    struct xSSLRSettings {
        #include "../Shaders/Shaders/Common/SSLRSettings.h"
    };

    ConstantBuffer *cbSSLRSettings;

    Shader *shSSLR;

    Texture *mSSLRTex;

    // Saves last RT's size.
    // So we can do something later
    float fWidth;
    float fHeight;
public:
    SSLRPostProcess() {
        EffectBase::AddRef(this);

        // Default screen size
        UINT Width = 1366;
        UINT Height = 768;

        // Create constant buffer
        cbSSLRSettings = new ConstantBuffer();
        cbSSLRSettings->CreateDefault(sizeof(xSSLRSettings));
        cbSSLRSettings->SetName("[CB]: SSLR Settings");

        // Load shaders
        shSSLR = new Shader();
        shSSLR->LoadFile("shSSLRCS.cso", Shader::Compute);
        shSSLR->ReleaseBlobs();

        // Create texture
        mSSLRTex = new Texture(tf_dim_2 | tf_UAV, DXGI_FORMAT_R16G16B16A16_FLOAT, Width, Height, 1u, 1u, "[PostProcess]: SSLR Buffer");
    }

    ~SSLRPostProcess() {
        SAFE_RELEASE(cbSSLRSettings);
        SAFE_RELEASE(mSSLRTex);
        SAFE_RELEASE(shSSLR);
    }

    void Resize(UINT Width, UINT Height) override {
        mSSLRTex->Resize(Width, Height);
    }

    // RB must contain depth buffer
    // RB must contain albedo buffer
    // RB must contain normal buffer
    // RB must contain shading buffer
    template<size_t dim, size_t BufferNum, bool DepthBuffer=false,
             size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps
                                                         to create per RT buffer   */
             bool WillHaveMSAA=false, bool Cube=false>
    void Begin(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube> *RB, const SSLRArgs& args) {
        // Get depth buffer size
        fWidth  = (float)RB->GetWidth();
        fHeight = (float)RB->GetHeight();

        // Prepare constant buffers
        {
            ScopeMapConstantBuffer<xSSLRSettings> q(cbSSLRSettings);
            q.data->_ProjValues         = args._ProjValues;
            q.data->_ViewAngleThreshold = args._ViewAngleThreshold;
            q.data->_EdgeDistThreshold  = args._EdgeDistThreshold;
            q.data->_DepthBias          = args._DepthBias;
            q.data->_ReflScale          = args._ReflScale;
            q.data->_ScreenSize         = { fHeight, fWidth };
        }

        // Bind resources
        cbSSLRSettings->Bind(Shader::Compute, 0);  // CB
        RB->Bind(0u, Shader::Compute, 0);          // SRV // Depth
        RB->Bind(1u, Shader::Compute, 1);          // SRV // Albedo
        RB->Bind(2u, Shader::Compute, 2);          // SRV // Normals
        RB->Bind(3u, Shader::Compute, 3);          // SRV // Shading (Metalness, Roughness, ...)
        mSSLRTex->Bind(Shader::Compute, 0u, true); // UAV

        // Dispatch
        uint X = (uint32_t)(ceil(mSSLRTex->GetWidth() / 8.f + .5f));
        uint Y = (uint32_t)(ceil(mSSLRTex->GetHeight() / 4.f + .5f));

        shSSLR->Dispatch(X, Y, 1u);

        // Discard
        LunaEngine::CSDiscardUAV<1>();
        LunaEngine::CSDiscardSRV<4>();
        LunaEngine::CSDiscardCB<2>();
    }

    inline ID3D11ShaderResourceView* GetSRV() const { return mSSLRTex->GetSRV(); }
};
