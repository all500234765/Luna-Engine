#pragma once

#include "EffectBase.h"
#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/Materials/Texture.h"
#include "Engine/States/BlendState.h"
#include "Engine/States/DepthStencilState.h"
#include "Engine/States/RasterState.h"
#include "Engine/States/TopologyState.h"
#include "Engine/Materials/Sampler.h"
#include "Engine/Camera/Camera.h"
#include <iostream>
#include <vector>

class OrderIndendentTransparency: public CallbackResize, EffectBase {
private:
    Shader *shOITCreateLinkedLists;
    Shader *shOITFinal;

#include "Shaders/Common/OITCommon.h"

    DepthStencilState *dssNoWrite;
    BlendState *bsMaskZero;
    RasterState *rsNoCulling;
    TopologyState *tState;

    RenderTarget<2, 1, false, 1, true, false> *rtTransparency;

    Camera cam2D;

public:
    OrderIndendentTransparency() {
        EffectBase::AddRef(this);

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

        rtTransparency = new RenderTarget<2, 1, false, 1, true, false>(Width, Height, 1, "[OIT]: Transparency");
        rtTransparency->Create(DXGI_FORMAT_R8G8B8A8_UNORM);

        rwListHead = Texture(Width, Height, DXGI_FORMAT_R32_UINT, true);
        sbLinkedLists.CreateDefault(MAX_ELEMENTS * Width * Height, nullptr, true);

        // Camera
        cam2D = Camera();

        CameraConfig cam_cfg;
        cam_cfg.Ortho = true;
        cam_cfg.ViewW = Width;
        cam_cfg.ViewH = Height;
        cam_cfg.fFar = 10.f;
        cam_cfg.fNear = .1f;

        cam2D.Init();
        cam2D.SetParams(cam_cfg);
        cam2D.BuildProj();
        //cam2D.BuildView();
        cam2D.SetViewMatrix(DirectX::XMMatrixIdentity());
        cam2D.SetWorldMatrix(DirectX::XMMatrixIdentity());
        cam2D.BuildConstantBuffer();

        // Topology state
        tState = new TopologyState();
        tState->Create(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Raster state
        rsNoCulling = new RasterState();
        D3D11_RASTERIZER_DESC rsDesc{};
        rsDesc.CullMode = D3D11_CULL_NONE;
        rsDesc.FillMode = D3D11_FILL_SOLID;
        rsDesc.DepthBias = -0;
        rsDesc.DepthBiasClamp = 0.f;
        rsDesc.SlopeScaledDepthBias = 0.f;
        rsDesc.DepthClipEnable = true;
        rsDesc.ScissorEnable = true;
        rsDesc.MultisampleEnable = true;
        rsDesc.AntialiasedLineEnable = true;
        rsDesc.FrontCounterClockwise = false;
        
        rsNoCulling->Create(rsDesc);
        
        // Depth stencil state
        dssNoWrite = new DepthStencilState();
        D3D11_DEPTH_STENCIL_DESC dssDesc {};
        dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dssDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        dssDesc.StencilEnable = false;
        dssDesc.DepthEnable = true;

        dssNoWrite->Create(dssDesc);

        // Blend state
        bsMaskZero = new BlendState();
        D3D11_BLEND_DESC bsDesc;
        bsDesc.IndependentBlendEnable = false;
        bsDesc.AlphaToCoverageEnable  = false;
        bsDesc.RenderTarget[0].BlendEnable = false;
        bsDesc.RenderTarget[0].RenderTargetWriteMask = 0;

        bsMaskZero->Create(bsDesc, { 0.f, 0.f, 0.f, 0.f });
    }

    ~OrderIndendentTransparency() {
        rwListHead.Release();
        sbLinkedLists.Release();

        SAFE_RELEASE(dssNoWrite);
        SAFE_RELEASE(bsMaskZero);
        SAFE_RELEASE(rsNoCulling);
        SAFE_DELETE(tState);

        SAFE_RELEASE(shOITCreateLinkedLists);
        SAFE_RELEASE(shOITFinal);

        SAFE_RELEASE(rtTransparency);
    }

    void Resize(UINT Width, UINT Height) override {
        sbLinkedLists.Release();
        sbLinkedLists.CreateDefault(MAX_ELEMENTS * Width * Height, nullptr, true);

        rwListHead.Resize(Width, Height);

        rtTransparency->Resize(Width, Height);

        // Update camera
        CameraConfig cam_cfg;
        cam_cfg.Ortho = true;
        cam_cfg.ViewW = Width;
        cam_cfg.ViewH = Height;

        cam2D.SetParams(cam_cfg);
        cam2D.BuildProj();
        //cam2D.BuildView();
        cam2D.SetViewMatrix(DirectX::XMMatrixIdentity());
        cam2D.BuildConstantBuffer();
    }
    
    // Must have:
    //  - Depth buffer w/ all opaque geometry
    //  - Albedo buffer w/ all opaque geometry
    template<size_t dim, size_t BufferNum, bool DepthBuffer=false,
             size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps
                                                         to create per RT buffer   */
             bool WillHaveMSAA=false, bool Cube=false>
    void Begin(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube> *RB) {
        if constexpr( !DepthBuffer ) return;
        if constexpr( !BufferNum   ) return;

        RangeProfiler::Begin(L"Order Independent Transparency");
        ScopedRangeProfiler s0(L"Begin");

        // MSAA
        if( RB->IsMSAAEnabled() ) {
            if( !rtTransparency->IsMSAAEnabled() ) { // Enable MSAA
                rtTransparency->EnableMSAA();
                rtTransparency->SetMSAAMaxLevel(RB->GetMaxSampleCount());
                rtTransparency->Resize(rtTransparency->GetWidth(), rtTransparency->GetHeight());
            }
        } else {
            if( rtTransparency->IsMSAAEnabled() ) { // Disable MSAA
                rtTransparency->DisableMSAA();
                rtTransparency->Resize(rtTransparency->GetWidth(), rtTransparency->GetHeight());
            }
        }

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
        const UINT Val00[4] = { 0x00, 0x00, 0x00, 0x00 };
        gDirectX->gContext->ClearUnorderedAccessViewUint(rwListHead.GetUAV(), ValFF);
        //gDirectX->gContext->ClearUnorderedAccessViewUint(sbLinkedLists.GetUAV(), Val00);

        // Reset counter and bind render target with depth buffer
        ID3D11UnorderedAccessView *UAVs[2] = { sbLinkedLists.GetUAV(), rwListHead.GetUAV() };
        ID3D11RenderTargetView *rtv = RB->GetBufferRTV<0>();
        gDirectX->gContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &rtv, RB->GetDSV(), 1, 2, UAVs, 0);

        // Bind shader
        shOITCreateLinkedLists->Bind();
    }

    void Bind() {
        // Bind shader
        shOITCreateLinkedLists->Bind();
    }

    // Render transparency //w/ Bind callback after shaders

    void End() {
        ScopedRangeProfiler s0(L"End");

        // Re-bind RTV and UAV slots
        ID3D11UnorderedAccessView *uav[2] = { nullptr, rwListHead.GetUAV() };
        ID3D11RenderTargetView *rtv = rtTransparency->GetBufferRTV<0>();

        gDirectX->gContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &rtv, nullptr, 1, 2, uav, 0);

        // Begin final render pass
        shOITFinal->Bind();

        // 
        Camera::Push();

        // Bind camera
        cam2D.Bind();
        cam2D.BindBuffer(Shader::Vertex, 0);
        
        gDirectX->gContext->Draw(6, 0);

        // Unbind
        ID3D11UnorderedAccessView *pEmptyUAV[2] = { nullptr };
        ID3D11RenderTargetView *pEmptyRTV = nullptr;
        gDirectX->gContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &pEmptyRTV, nullptr, 1, 2, pEmptyUAV, 0);

        // Resolve MSAA
        rtTransparency->MSAAResolve();
        
        // Restore old states
        DepthStencilState::Pop();
        //TopologyState::Pop();
        RasterState::Pop();
        BlendState::Pop();
        Camera::Pop();

        // L"Order Independent Transparency"
        RangeProfiler::End();
    }

    inline ID3D11Resource *GetTexture() const {
        return rtTransparency->GetBufferTexture<0>();
    }

    inline ID3D11ShaderResourceView *GetSRV() const {
        return rtTransparency->GetBufferSRV<0>();
    }

};
