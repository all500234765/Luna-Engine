#include "RenderBufferBase.h"

#include <iostream>
#include <vector>

// MSAA Resolve
Shader*         RenderBufferBase::g_shMSAADepthResolve = 0;
ConstantBuffer* RenderBufferBase::g_MSAAConstantBuffer = 0;
Texture*        RenderBufferBase::g_MSAATextureUAV     = 0;

sRenderBuffer* RenderBufferBase::CreateRTV2D(int W, int H, DXGI_FORMAT format, bool UAV, UINT RTIndex) {
    ID3D11RenderTargetView    *pRTV     = 0;
    ID3D11Texture2D           *pTexture = 0;
    ID3D11ShaderResourceView  *pSRV     = 0;
    ID3D11UnorderedAccessView *pUAV     = 0;

    // MSAA
    UINT SampleCount = 1, Quality = 0;
    if( !UAV ) MSAACheck(format, SampleCount, Quality);

    // Store sample count
    mMSAASamples[RTIndex] = SampleCount;

    // Create Texture 2D
    D3D11_TEXTURE2D_DESC pTexDesc = {};
    pTexDesc.ArraySize          = 1;
    pTexDesc.MipLevels          = 1;
    pTexDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | (UAV ? D3D11_BIND_UNORDERED_ACCESS : 0);
    pTexDesc.Usage              = D3D11_USAGE_DEFAULT;
    pTexDesc.CPUAccessFlags     = 0;
    pTexDesc.MiscFlags          = 0;
    pTexDesc.Format             = format;
    pTexDesc.Width              = W;
    pTexDesc.Height             = H;
    pTexDesc.SampleDesc.Count   = SampleCount;
    pTexDesc.SampleDesc.Quality = (UINT)max((int)Quality - 1, 0);

    auto res = gDirectX->gDevice->CreateTexture2D(&pTexDesc, NULL, &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create 2D texture for render target." << std::endl;
    }

    // Create RTV
    D3D11_RENDER_TARGET_VIEW_DESC pRTVDesc = {};
    pRTVDesc.Format             = format;
    pRTVDesc.ViewDimension      = Quality ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
    pRTVDesc.Texture2D.MipSlice = 0;

    res = gDirectX->gDevice->CreateRenderTargetView(pTexture, &pRTVDesc, &pRTV);
    if( FAILED(res) ) {
        std::cout << "Failed to create render target." << std::endl;
    }

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc = {};
    pSRVDesc.Format                    = format;
    pSRVDesc.ViewDimension             = Quality ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
    pSRVDesc.Texture2D.MipLevels       = 1;
    pSRVDesc.Texture2D.MostDetailedMip = 0;

    res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res) ) {
        std::cout << "Failed to create shader resource view." << std::endl;
    }

    // Create UAV
    if( UAV ) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC pUAVDesc = {};
        pUAVDesc.Format = format;
        pUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        pUAVDesc.Texture2D.MipSlice = 0;

        res = gDirectX->gDevice->CreateUnorderedAccessView(pTexture, &pUAVDesc, &pUAV);
        if( FAILED(res) ) {
            std::cout << "Failed to create unordered access view." << std::endl;
        }
    }
    
    // Output
    sRenderBuffer *Out = new sRenderBuffer;
        Out->pRTV       = pRTV;
        Out->pTexture2D = pTexture;
        Out->pSRV       = pSRV;
        Out->pUAV       = pUAV;
        Out->Flags      = RTV | Is2D | (Quality ? IsMSAA : 0);
        Out->format     = format;
    return Out;
}

sRenderBuffer* RenderBufferBase::CreateDSV2D(int W, int H, UINT bpp, bool UAV) {
    ID3D11DepthStencilView *pDSV    = 0;
    ID3D11Texture2D *pTexture       = 0;
    ID3D11ShaderResourceView *pSRV  = 0;
    ID3D11UnorderedAccessView *pUAV = 0;

    DXGI_FORMAT formatTex, formatDSV, formatSRV;
    switch( bpp ) {
        case 32:
            formatTex = DXGI_FORMAT_R32_TYPELESS;
            formatDSV = DXGI_FORMAT_D32_FLOAT;
            formatSRV = DXGI_FORMAT_R32_FLOAT;
            break;
            
        case 24:
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

    // MSAA
    UINT SampleCount = 1, Quality = 0;
    if( !UAV ) MSAACheck(formatTex, SampleCount, Quality);
    
    // Store sample count
    mMSAASamples[UAV ? 1 : 0] = SampleCount;

    // Create Texture 2D
    D3D11_TEXTURE2D_DESC pTexDesc = {};
    pTexDesc.ArraySize          = 1;
    pTexDesc.MipLevels          = 1;
    pTexDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | (UAV ? D3D11_BIND_UNORDERED_ACCESS : D3D11_BIND_DEPTH_STENCIL);
    pTexDesc.CPUAccessFlags     = 0;
    pTexDesc.MiscFlags          = 0;
    pTexDesc.Format             = formatTex;
    pTexDesc.Width              = W;
    pTexDesc.Height             = H;
    pTexDesc.Usage              = D3D11_USAGE_DEFAULT;
    pTexDesc.SampleDesc.Count   = SampleCount;
    pTexDesc.SampleDesc.Quality = (UINT)max((int)Quality - 1, 0);

    auto res = gDirectX->gDevice->CreateTexture2D(&pTexDesc, NULL, &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create 2D texture for render target." << std::endl;
    }

    if( !UAV ) {
        // Create DSV
        D3D11_DEPTH_STENCIL_VIEW_DESC pDSVDesc = {};
        pDSVDesc.Flags              = 0;
        pDSVDesc.Format             = formatDSV;
        pDSVDesc.Texture2D.MipSlice = 0;
        pDSVDesc.ViewDimension      = Quality ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

        res = gDirectX->gDevice->CreateDepthStencilView(pTexture, &pDSVDesc, &pDSV);
        if( FAILED(res) ) {
            std::cout << "Failed to create depth stencil view." << std::endl;
        }
    }

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc = {};
    pSRVDesc.Format                    = formatSRV;
    pSRVDesc.ViewDimension             = Quality ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
    pSRVDesc.Texture2D.MipLevels       = 1;
    pSRVDesc.Texture2D.MostDetailedMip = 0;

    res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res) ) {
        std::cout << "Failed to create shader resource view." << std::endl;
    }

    // Create UAV
    if( UAV ) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC pUAVDesc = {};
        pUAVDesc.Format = formatSRV;
        pUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        pUAVDesc.Texture2D.MipSlice = 0;

        res = gDirectX->gDevice->CreateUnorderedAccessView(pTexture, &pUAVDesc, &pUAV);
        if( FAILED(res) ) {
            std::cout << "Failed to create unordered access view." << std::endl;
        }
    }

    // 
    SetSize(W, H);

    // Output
    sRenderBuffer *Out = new sRenderBuffer();
        Out->pDSV       = pDSV;
        Out->pTexture2D = pTexture;
        Out->pSRV       = pSRV;
        Out->pUAV       = pUAV;
        Out->Flags      = DSV | Is2D | (Quality ? IsMSAA : 0);
        Out->bpp        = bpp;
    return Out;
}

sRenderBuffer* RenderBufferBase::CreateRTV3D(int W, int H, int D, DXGI_FORMAT format, bool UAV) {
    ID3D11RenderTargetView *pRTV    = 0;
    ID3D11Texture3D *pTexture       = 0;
    ID3D11ShaderResourceView *pSRV  = 0;
    ID3D11UnorderedAccessView *pUAV = 0;

    // Create Texture 3D
    D3D11_TEXTURE3D_DESC pTexDesc;
    pTexDesc.MipLevels      = 1;
    pTexDesc.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    pTexDesc.CPUAccessFlags = 0;
    pTexDesc.MiscFlags      = 0;
    pTexDesc.Format         = format;
    pTexDesc.Width          = W;
    pTexDesc.Height         = H;
    pTexDesc.Depth          = D;
    pTexDesc.Usage          = D3D11_USAGE_DEFAULT;

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

sRenderBuffer* RenderBufferBase::CreateDSV3D(int W, int H, int D, UINT bpp, bool UAV) {
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

void RenderBufferBase::BindResource(sRenderBuffer* data, Shader::ShaderType type, UINT slot, bool UAV) {
    switch( type ) {
        case Shader::Vertex  : gDirectX->gContext->VSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Pixel   : gDirectX->gContext->PSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Geometry: gDirectX->gContext->GSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Hull    : gDirectX->gContext->HSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Domain  : gDirectX->gContext->DSSetShaderResources(slot, 1, &data->pSRV); break;
        case Shader::Compute :
            if( UAV ) {
                UINT InitCount = { 0 };
                gDirectX->gContext->CSSetUnorderedAccessViews(slot, 1, &data->pUAV, &InitCount);
            } else {
                gDirectX->gContext->CSSetShaderResources(slot, 1, &data->pSRV);
            }
            break;
    }
}

void RenderBufferBase::SetSize(int w, int h) {
    Width = w;
    Height = h;

    mVP.TopLeftX = 0.f;
    mVP.TopLeftY = 0.f;
    mVP.MinDepth = 0.f;
    mVP.MaxDepth = 1.f;
    mVP.Width    = w;
    mVP.Height   = h;
}

int RenderBufferBase::GetWidth() {
    return Width;
}

int RenderBufferBase::GetHeight() {
    return Height;
}

int RenderBufferBase::GetDepth() {
    return Depth;
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
