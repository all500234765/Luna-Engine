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

public:
    OrderIndendentTransparency() {
        EffectBase::AddRef(this);
        
        // 
        shOITCreateLinkedLists = new Shader();
        shOITCreateLinkedLists->LoadFile("VS.cso", Shader::Vertex);
        shOITCreateLinkedLists->LoadFile("OrderIndependentTransparencyPreparePS.cso", Shader::Pixel);

        shOITFinal = new Shader();
        
        shOITFinal->LoadFile("OrderIndependentTransparencyFinalPS.cso", Shader::Pixel);

        shOITCreateLinkedLists->ReleaseBlobs();
        shOITFinal->ReleaseBlobs();

        // Create resources with default size
        uint32_t Width = 1366;
        uint32_t Height = 768;

        rwListHead = Texture(Width, Height, DXGI_FORMAT_R32_UINT, true);
        sbLinkedLists.CreateDefault(MAX_ELEMENTS * Width * Height, nullptr, true);

        // Topology state
        tState = new TopologyState();
        tState->Create(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Raster state
        rsNoCulling = new RasterState();
        D3D11_RASTERIZER_DESC rsDesc;
        rsDesc.CullMode = D3D11_CULL_NONE;
        
        rsNoCulling->Create(rsDesc);

        // Depth stencil state
        dssNoWrite = new DepthStencilState();
        D3D11_DEPTH_STENCIL_DESC dssDesc{};
        

        dssNoWrite->Create(dssDesc);

        // Blend state
        bsMaskZero = new BlendState();
        D3D11_BLEND_DESC bsDesc;
        bsDesc.IndependentBlendEnable = false;
        bsDesc.RenderTarget[0].BlendEnable = true;
        bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

        bsMaskZero->Create(bsDesc, { 0.f, 0.f, 0.f, 0.f });
    }

    ~OrderIndendentTransparency() {
        rwListHead.Release();
        sbLinkedLists.Release();

        SAFE_RELEASE(dssNoWrite);
        SAFE_RELEASE(bsMaskZero);
        SAFE_RELEASE(rsNoCulling);
        SAFE_DELETE(tState);
    }

    void Resize(UINT Width, UINT Height) override {
        sbLinkedLists.Release();
        sbLinkedLists.CreateDefault(MAX_ELEMENTS * Width * Height, nullptr, true);

        rwListHead.Resize(Width, Height);
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
        gDirectX->gContext->ClearUnorderedAccessViewUint(rwListHead.GetUAV(), 0xFFFFFFFF);

        // Reset counter and bind render target with depth buffer
        gDirectX->gContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &RB.GetBufferRTV<0>(), RB.GetDSV(), 1, 1, &sbLinkedLists.GetUAV(), 0);

    }

    // Render transparency

    void End() {

        // 



        // Restore old states
        DepthStencilState::Pop();
        //TopologyState::Pop();
        RasterState::Pop();
        BlendState::Pop();
    }

};
