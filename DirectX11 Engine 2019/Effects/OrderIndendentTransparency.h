#pragma once

#include "pc.h"
#include "EffectBase.h"
#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/Scene/Texture.h"
#include "Engine/States/BlendState.h"
#include "Engine/States/DepthStencilState.h"
#include "Engine/States/RasterState.h"
#include "Engine/States/TopologyState.h"
#include "Engine/Scene/Sampler.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ScopedMapper.h"

struct OITSettings {
    mfloat4x4 mInvViewProj;
    float fMinFadeDist;
    float fMaxFadeDist;
};

class OrderIndendentTransparency: public CallbackResize, EffectBase {
private:
    Shader *shOITCreateLinkedLists;
    Shader *shOITFinal;

#include "Shaders/Common/OITCommon.h"

    struct DataBuffer {
        mfloat4x4 _InvViewProj;
        float3 _CameraPos;
        float1 _MinFadeDist2;
        float1 _MaxFadeDist2;
        float3 _Padding0;
    };

    ConstantBuffer *cbDataBuffer;

    DepthStencilState *dssNoWrite, *dssWrite;
    BlendState *bsMaskZero, *bsNoBlend;
    RasterState *rsNoCulling;
    TopologyState *tState;

    mfloat4x4 mViewTmp, mProjTmp;

public:
    OrderIndendentTransparency() {
        EffectBase::AddRef(this);

        // 
        cbDataBuffer = new ConstantBuffer();
        cbDataBuffer->CreateDefault(sizeof(DataBuffer));
        cbDataBuffer->SetName("[CB]: OIT Data Buffer 0");

        // Load shaders
        shOITCreateLinkedLists = new Shader();
        //shOITCreateLinkedLists->DontTouch({ Shader::Vertex });
        shOITCreateLinkedLists->LoadFile("shOrderIndependentTransparencyVS.cso", Shader::Vertex);
        shOITCreateLinkedLists->LoadFile("OrderIndependentTransparencyPreparePS.cso", Shader::Pixel);

        shOITFinal = new Shader();
        shOITFinal->LoadFile("shTexturedQuadAutoVS.cso", Shader::Vertex);
        shOITFinal->LoadFile("OrderIndependentTransparencyFinalPS.cso", Shader::Pixel);

        shOITCreateLinkedLists->ReleaseBlobs();
        shOITFinal->ReleaseBlobs();

        // Create resources with default size
        uint32_t Width = 1366;
        uint32_t Height = 768;

        rwListHead = Texture(tf_dim_2 | tf_UAV, DXGI_FORMAT_R32_UINT, Width, Height, 1u, 1u, "OIT Linked lists heads");
        sbLinkedLists.CreateDefault(MAX_ELEMENTS * Width * Height, nullptr, true);

        // Topology state
        tState = new TopologyState();
        tState->Create(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Raster state
        rsNoCulling = new RasterState();
        D3D11_RASTERIZER_DESC rsDesc{};
        rsDesc.CullMode = D3D11_CULL_NONE;
        rsDesc.FillMode = D3D11_FILL_SOLID;
        rsDesc.DepthBias = 0;
        rsDesc.DepthBiasClamp = 0.f;
        rsDesc.SlopeScaledDepthBias = 0.f;
        rsDesc.DepthClipEnable = true;
        rsDesc.ScissorEnable = false;
        rsDesc.MultisampleEnable = true;
        rsDesc.AntialiasedLineEnable = false;
        rsDesc.FrontCounterClockwise = false;
        
        rsNoCulling->Create(rsDesc);
        
        // Depth stencil state
        dssWrite = new DepthStencilState();
        dssNoWrite = new DepthStencilState();

        D3D11_DEPTH_STENCIL_DESC dssDesc {};
        dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dssDesc.DepthFunc = D3D11_COMPARISON_GREATER;
        dssDesc.StencilEnable = false;
        dssDesc.DepthEnable = true;

        dssNoWrite->Create(dssDesc);

        dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dssWrite->Create(dssDesc);

        // Blend state
        bsMaskZero = new BlendState();
        bsNoBlend = new BlendState();

        D3D11_BLEND_DESC bsDesc;
        bsDesc.IndependentBlendEnable = false;
        bsDesc.AlphaToCoverageEnable  = false;
        bsDesc.RenderTarget[0].BlendEnable = true;
        bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
        bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        bsDesc.RenderTarget[0].RenderTargetWriteMask = 0;

        bsMaskZero->Create(bsDesc, { 1.f, 1.f, 1.f, 1.f });

        bsDesc.RenderTarget[0].BlendEnable = false;
        bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        bsNoBlend->Create(bsDesc, { 1.f, 1.f, 1.f, 1.f });
    }

    ~OrderIndendentTransparency() {
        rwListHead.Release();
        sbLinkedLists.Release();

        SAFE_RELEASE(cbDataBuffer);
        SAFE_RELEASE(dssWrite);
        SAFE_RELEASE(dssNoWrite);
        SAFE_RELEASE(bsMaskZero);
        SAFE_RELEASE(bsNoBlend);
        SAFE_RELEASE(rsNoCulling);

        SAFE_DELETE(tState);

        SAFE_RELEASE(shOITCreateLinkedLists);
        SAFE_RELEASE(shOITFinal);
    }

    void Resize(UINT Width, UINT Height) override {
        sbLinkedLists.Release();
        sbLinkedLists.CreateDefault(MAX_ELEMENTS * Width * Height, nullptr, true);

        rwListHead.Resize(Width, Height);
    }
    
    // Must have:
    //  - Depth buffer w/ all opaque geometry
    template<size_t dim, size_t BufferNum, bool DepthBuffer=false,
             size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps
                                                         to create per RT buffer   */
             bool WillHaveMSAA=false, bool Cube=false>
    void Begin(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube> *RB) {
        if constexpr( !DepthBuffer  ) return;
        //if constexpr( BufferNum < 2 ) return;

        RangeProfiler::Begin(L"Order Independent Transparency");
        ScopedRangeProfiler s0(L"Begin");

        // Save current states
        DepthStencilState::Push();
        //TopologyState::Push();
        RasterState::Push();
        BlendState::Push();

        // Set new states
        rsNoCulling->Bind();
        dssNoWrite->Bind();
        bsMaskZero->Bind();
        //tState->Bind();

        // Clear head list to -1
        const UINT ValFF[4] = { 0xFFFFFFFF, 0, 0, 0 };
        gDirectX->gContext->ClearUnorderedAccessViewUint(rwListHead.GetUAV(), ValFF);

        // Reset counter and bind render target with depth buffer
        ID3D11UnorderedAccessView *UAVs[2] = { sbLinkedLists.GetUAV(), rwListHead.GetUAV() };
        ID3D11RenderTargetView *rtv = RB->GetBufferRTV<0, false>();
        const UINT InitCounts[2] = { 0, 0 };

        gDirectX->gContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &rtv, RB->GetDSV<0, false>(), 1, 2, UAVs, InitCounts);

        // Bind shader
        shOITCreateLinkedLists->Bind();
    }

    // Resolve MSAA; Render transparency
    
    // Depth, Albedo, Normal at least
    template<size_t dim, size_t BufferNum, bool DepthBuffer=false,
             size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps
                                                         to create per RT buffer   */
             bool WillHaveMSAA=false, bool Cube=false>
    void End(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube>* rtTransparent, const OITSettings& params) {
        ScopedRangeProfiler s0(L"End");
        Scene* mScene = Scene::Current();

        {
            ScopeMapConstantBuffer<DataBuffer> q(cbDataBuffer);

            q.data->_CameraPos    = mScene->GetCamera(mScene->GetActiveCamera())->cTransf->vPosition;
            q.data->_InvViewProj  = params.mInvViewProj;
            q.data->_MinFadeDist2 = params.fMinFadeDist * params.fMinFadeDist;
            q.data->_MaxFadeDist2 = params.fMaxFadeDist * params.fMaxFadeDist;
        }
        
        // Re-bind RTV and UAV slots
        ID3D11UnorderedAccessView *uav[2] = { nullptr, nullptr };
        ID3D11RenderTargetView *rtv[2] = { rtTransparent->GetBufferRTV<0, false>(), rtTransparent->GetBufferRTV<1, false>() };
        ID3D11DepthStencilView *dsv = rtTransparent->GetDSV<0, false>();

        const float Clear0[4] = { 0.f, 0.f, 0.f, 0.f };

        gDirectX->gContext->OMSetRenderTargetsAndUnorderedAccessViews(2, rtv, dsv, 2, 2, uav, 0);
        gDirectX->gContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.f, 1);
        gDirectX->gContext->ClearRenderTargetView(rtv[0], Clear0);
        gDirectX->gContext->ClearRenderTargetView(rtv[1], Clear0);

        // Begin final render pass
        shOITFinal->Bind();

        // 
        dssWrite->Bind();
        //bsNoBlend->Bind();

        // Store states
        mScene->DefineCameraOrtho(2, .1f, 10.f, 1.f, 1.f);
        mProjTmp = mScene->GetCamera(2)->cCam->mProj;
        mViewTmp = mScene->GetCamera(2)->cCam->mView;
        RasterState::Pop();
        BlendState::Pop();

        // Bind camera
        mScene->GetCamera(2)->ProjIdentity();
        mScene->GetCamera(2)->ViewIdentity();
        mScene->BindCamera(2, Shader::Vertex, 1);

        // Bind resources
        cbDataBuffer->Bind(Shader::Pixel, 0);
        sbLinkedLists.Bind(Shader::Pixel, 1);
        rwListHead.Bind(Shader::Pixel, 2);

        DXDraw(6, 0);

        // Unbind
        ID3D11UnorderedAccessView *pEmptyUAV[2] = { nullptr, nullptr };
        ID3D11RenderTargetView *pEmptyRTV[2] = { nullptr, nullptr };
        gDirectX->gContext->OMSetRenderTargetsAndUnorderedAccessViews(2, pEmptyRTV, nullptr, 2, 2, pEmptyUAV, 0);

        // Resolve MSAA
        rtTransparent->MSAAResolve();

        // Restore old states
        mScene->GetCamera(2)->SetProj(mProjTmp);
        mScene->GetCamera(2)->SetView(mViewTmp);
        //TopologyState::Pop();
        DepthStencilState::Pop();

        // L"Order Independent Transparency"
        RangeProfiler::End();
    }

    inline ID3D11ShaderResourceView *GetHeadSRV() const {
        return rwListHead.GetSRV();
    }

};
