#include "RenderBufferBase.h"

#include <iostream>
#include <vector>

sRenderBuffer* RenderBufferBase::CreateRTV2D(int W, int H, DXGI_FORMAT format) {
    ID3D11RenderTargetView *pRTV = 0;
    ID3D11Texture2D *pTexture = 0;
    ID3D11ShaderResourceView *pSRV = 0;

    // Create Texture 2D
    D3D11_TEXTURE2D_DESC pTexDesc;
    pTexDesc.ArraySize = 1;
    pTexDesc.MipLevels = 1;
    pTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    pTexDesc.Usage = D3D11_USAGE_DEFAULT;
    pTexDesc.CPUAccessFlags = 0;
    pTexDesc.MiscFlags = 0;
    pTexDesc.Format = format;
    pTexDesc.Width = W;
    pTexDesc.Height = H;
    pTexDesc.SampleDesc.Count = 1;
    pTexDesc.SampleDesc.Quality = 0;

    auto res = gDirectX->gDevice->CreateTexture2D(&pTexDesc, NULL, &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create 2D texture for render target." << std::endl;
    }

    // Create RTV
    D3D11_RENDER_TARGET_VIEW_DESC pRTVDesc;
    pRTVDesc.Format = format;
    pRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    pRTVDesc.Texture2D.MipSlice = 0;

    res = gDirectX->gDevice->CreateRenderTargetView(pTexture, &pRTVDesc, &pRTV);
    if( FAILED(res) ) {
        std::cout << "Failed to create render target." << std::endl;
    }

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
    pSRVDesc.Format = format;
    pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    pSRVDesc.Texture2D.MipLevels = 1;
    pSRVDesc.Texture2D.MostDetailedMip = 0;

    res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res) ) {
        std::cout << "Failed to create shader resource view." << std::endl;
    }

    // Output
    sRenderBuffer *Out = new sRenderBuffer;
        Out->pRTV       = pRTV;
        Out->pTexture2D = pTexture;
        Out->pSRV       = pSRV;
        Out->Flags      = RTV | Is2D;
        Out->format     = format;
    return Out;
}

sRenderBuffer* RenderBufferBase::CreateDSV2D(int W, int H, UINT bpp) {
    ID3D11DepthStencilView *pDSV = 0;
    ID3D11Texture2D *pTexture = 0;
    ID3D11ShaderResourceView *pSRV = 0;

    DXGI_FORMAT formatTex, formatDSV, formatSRV;
    switch( bpp ) {
        case 32:
            formatTex = DXGI_FORMAT_R24G8_TYPELESS;
            formatDSV = DXGI_FORMAT_D24_UNORM_S8_UINT;
            formatSRV = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            break;

        case 16:
            formatTex = DXGI_FORMAT_R16_TYPELESS;
            formatDSV = DXGI_FORMAT_D16_UNORM;
            formatSRV = DXGI_FORMAT_R16_UNORM;
            break;
    }

    // Create Texture 2D
    D3D11_TEXTURE2D_DESC pTexDesc;
    pTexDesc.ArraySize = 1;
    pTexDesc.MipLevels = 1;
    pTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    pTexDesc.CPUAccessFlags = 0;
    pTexDesc.MiscFlags = 0;
    pTexDesc.Format = formatTex;
    pTexDesc.Width = W;
    pTexDesc.Height = H;
    pTexDesc.Usage = D3D11_USAGE_DEFAULT;
    pTexDesc.SampleDesc.Count = 1;
    pTexDesc.SampleDesc.Quality = 0;

    auto res = gDirectX->gDevice->CreateTexture2D(&pTexDesc, NULL, &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create 2D texture for render target." << std::endl;
    }

    // Create DSV
    D3D11_DEPTH_STENCIL_VIEW_DESC pDSVDesc;
    pDSVDesc.Flags = 0;
    pDSVDesc.Format = formatDSV;
    pDSVDesc.Texture2D.MipSlice = 0;
    pDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    res = gDirectX->gDevice->CreateDepthStencilView(pTexture, &pDSVDesc, &pDSV);
    if( FAILED(res) ) {
        std::cout << "Failed to create depth stencil view." << std::endl;
    }

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
    pSRVDesc.Format = formatSRV;
    pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    pSRVDesc.Texture2D.MipLevels = 1;
    pSRVDesc.Texture2D.MostDetailedMip = 0;

    res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res) ) {
        std::cout << "Failed to create shader resource view." << std::endl;
    }

    // Output
    sRenderBuffer *Out = new sRenderBuffer();
        Out->pDSV       = pDSV;
        Out->pTexture2D = pTexture;
        Out->pSRV       = pSRV;
        Out->Flags      = DSV | Is2D;
        Out->bpp        = bpp;
    return Out;
}

sRenderBuffer* RenderBufferBase::CreateRTV3D(int W, int H, int D, DXGI_FORMAT format) {
    ID3D11RenderTargetView *pRTV;
    ID3D11Texture3D *pTexture;
    ID3D11ShaderResourceView *pSRV;

    // Create Texture 3D
    D3D11_TEXTURE3D_DESC pTexDesc;
    pTexDesc.MipLevels = 1;
    pTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    pTexDesc.CPUAccessFlags = 0;
    pTexDesc.MiscFlags      = 0;
    pTexDesc.Format = format;
    pTexDesc.Width  = W;
    pTexDesc.Height = H;
    pTexDesc.Depth  = D;
    pTexDesc.Usage = D3D11_USAGE_DEFAULT;

    auto res = gDirectX->gDevice->CreateTexture3D(&pTexDesc, NULL, &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create 3D texture for render target." << std::endl;
    }

    // Create RTV
    D3D11_RENDER_TARGET_VIEW_DESC pRTVDesc;
    pRTVDesc.Format = format;
    pRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
    pRTVDesc.Texture3D.MipSlice = 0;
    pRTVDesc.Texture3D.FirstWSlice = 0;
    pRTVDesc.Texture3D.WSize = -1;

    res = gDirectX->gDevice->CreateRenderTargetView(pTexture, &pRTVDesc, &pRTV);
    if( FAILED(res) ) {
        std::cout << "Failed to create render target." << std::endl;
    }

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
    pSRVDesc.Format = format;
    pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    pSRVDesc.Texture3D.MipLevels = 1;
    pSRVDesc.Texture3D.MostDetailedMip = 0;

    res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res) ) {
        std::cout << "Failed to create shader resource view." << std::endl;
    }

    // Output
    sRenderBuffer *Out = new sRenderBuffer;
        Out->pRTV       = pRTV;
        Out->pTexture3D = pTexture;
        Out->pSRV       = pSRV;
        Out->Flags      = RTV | Is3D;
    return Out;
}

sRenderBuffer* RenderBufferBase::CreateDSV3D(int W, int H, int D, UINT bpp) {
    // TODO: Creation of 3D DSV
    // ...

    // Output
    sRenderBuffer *Out = new sRenderBuffer;
        /*Out->pDSV       = pDSV;
        Out->pTexture3D = pTexture;
        Out->pSRV       = pSRV;
        Out->Flags      = DSV | Is3D;*/
    return Out;
}

void RenderBufferBase::BindResource(sRenderBuffer* data, Shader::ShaderType type, UINT slot) {
    switch( type ) {
        case Shader::Vertex  : gDirectX->gContext->VSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Pixel   : gDirectX->gContext->PSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Geometry: gDirectX->gContext->GSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Hull    : gDirectX->gContext->HSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Domain  : gDirectX->gContext->DSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Compute : gDirectX->gContext->CSSetShaderResources(slot, 1, &data->pSRV); break;
    }
}

void RenderBufferBase::SetSize(int w, int h) {
    Width = w;
    Height = h;
}

void RenderBufferBase::BindTarget(sRenderBuffer* dRTV, sRenderBuffer* dDSV) {
    gDirectX->gContext->OMSetRenderTargets(1, &dRTV->pRTV, dDSV->pDSV);
}

void RenderBufferBase::BindTarget(sRenderBuffer* data) {
    if( data->Flags & RenderBufferFlags::RTV ) {
        gDirectX->gContext->OMSetRenderTargets(1, &data->pRTV, nullptr);
    } else if( data->Flags & RenderBufferFlags::DSV ) {
        gDirectX->gContext->OMSetRenderTargets(0, nullptr, data->pDSV);
    }
}

void RenderBufferBase::BindTarget(std::vector<sRenderBuffer*> dRTV, sRenderBuffer* dDSV) {
    ID3D11RenderTargetView *pRTVs[8];
    for( int i = 0; i < dRTV.size(); i++ ) { pRTVs[i] = dRTV[i]->pRTV; }

    gDirectX->gContext->OMSetRenderTargets(dRTV.size(), pRTVs, dDSV->pDSV);
}

void RenderBufferBase::BindTarget(std::vector<sRenderBuffer*> dRTV) {
    ID3D11RenderTargetView *pRTVs[8];
    for( int i = 0; i < dRTV.size(); i++ ) { pRTVs[i] = dRTV[i]->pRTV; }

    gDirectX->gContext->OMSetRenderTargets(dRTV.size(), pRTVs, nullptr);
}

void RenderBufferBase::BindTarget(sRenderBuffer* dRTV, ID3D11DepthStencilView* pDSV) {
    gDirectX->gContext->OMSetRenderTargets(1, &dRTV->pRTV, pDSV);
}

void RenderBufferBase::BindTarget(ID3D11RenderTargetView* pRTV, sRenderBuffer* dDSV) {
    gDirectX->gContext->OMSetRenderTargets(1, &pRTV, dDSV->pDSV);
}
