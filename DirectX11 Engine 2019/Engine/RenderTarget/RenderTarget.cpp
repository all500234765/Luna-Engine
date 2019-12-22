#include "RenderTarget.h"

// MSAA Resolve
Shader*         RenderTargetMSAA::g_shMSAADepthResolve = 0;
ConstantBuffer* RenderTargetMSAA::g_MSAAConstantBuffer = 0;
//Texture* RenderTargetMSAA::g_MSAATextureUAV = 0;

void implRenderTarget::Release() {
    if( pTexture.index() > 0 ) {
        if( (mFlags & dim_1) && std::get<ID3D11Texture1D*>(pTexture) ) { std::get<ID3D11Texture1D*>(pTexture)->Release(); }
        if( (mFlags & dim_2) && std::get<ID3D11Texture2D*>(pTexture) ) { std::get<ID3D11Texture2D*>(pTexture)->Release(); }
        if( (mFlags & dim_3) && std::get<ID3D11Texture3D*>(pTexture) ) { std::get<ID3D11Texture3D*>(pTexture)->Release(); }
    }

    if( pView.index() > 0 ) {
        if( mFlags & _Depth ) {
            if( pUAV == nullptr ) {
                auto q = std::get<ID3D11DepthStencilView*>(pView);
                if( q ) { q->Release(); }
            }
        } else {
            auto q = std::get<ID3D11RenderTargetView*>(pView);
            if( q ) { q->Release(); }
        }
    }

    if( pSRV ) { pSRV->Release(); }
    if( pUAV ) { pUAV->Release(); }

    pSRV = nullptr;
    pUAV = nullptr;
    pView = false;
    pTexture = false;
}

void RenderTargetMSAA::GlobalInit() {
    g_shMSAADepthResolve = new Shader();
    g_shMSAADepthResolve->LoadFile("shMSAADepthResolveCS.cso", Shader::Compute);
    g_shMSAADepthResolve->ReleaseBlobs();

    g_MSAAConstantBuffer = new ConstantBuffer();
    g_MSAAConstantBuffer->CreateDefault(sizeof(_MSAA_DepthResolve));
}

void RenderTargetMSAA::GlobalRelease() {
    if( g_shMSAADepthResolve ) {
        g_shMSAADepthResolve->Release();
        delete g_shMSAADepthResolve;
    }

    if( g_MSAAConstantBuffer ) {
        g_MSAAConstantBuffer->Release();
        delete g_MSAAConstantBuffer;
    }
}

