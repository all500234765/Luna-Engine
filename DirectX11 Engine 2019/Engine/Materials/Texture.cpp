#include "Texture.h"

Texture::Texture() {
}

Texture::Texture(std::string fname, UINT bpc) {
    Load(fname, bpc);
}

Texture::Texture(std::string fname, DXGI_FORMAT format) {
    Load(fname, format);
}

void Texture::Load(std::string fname, UINT bpc) {
    // Load texture
    void* data = stbi_load(fname.c_str(), &w, &h, &channels, 0);
    if( !data ) {
        std::cout << "Failed to load texture. (" << fname.c_str() << ")" << std::endl;
        return;
    }

    // Select format
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    switch( channels ) {
        case 1:
            // Red channel
            switch( bpc ) {
                case 8 : format = DXGI_FORMAT_R8_UNORM; break;
                case 16: format = DXGI_FORMAT_R16_FLOAT; break;
                case 32: format = DXGI_FORMAT_R32_FLOAT; break;
            }
            break;

        case 2:
            // Red, Green channels
            switch( bpc ) {
                case 8 : format = DXGI_FORMAT_R8G8_UNORM; break;
                case 16: format = DXGI_FORMAT_R16G16_FLOAT; break;
                case 32: format = DXGI_FORMAT_R32G32_FLOAT; break;
            }
            break;

        case 3:
            // Red, Green, Blue channels
            if( bpc == 32 ) format = DXGI_FORMAT_R32G32B32_FLOAT;
            else channels = 4;
            break;

        case 4:
            // Red, Green, Blue, Alpha channels
            switch( bpc ) {
                case 8 : format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
                case 16: format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
                case 32: format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
            }
            break;
    }

    // Create Texture and SRV
    Create(data, format, channels * bpc);

    stbi_image_free(data);
}

void Texture::Load(std::string fname, DXGI_FORMAT format) {
    // Load texture
    void* data = stbi_load(fname.c_str(), &w, &h, &channels, 0);
    if( !data ) {
        std::cout << "Failed to load texture. (" << fname.c_str() << ")" << std::endl;
        return;
    }

    // Create Texture and SRV
    Create(data, format, Format2BPP(format));

    stbi_image_free(data);
}

void Texture::Create(void* data, DXGI_FORMAT format, UINT bpp) {
    bool bGenMips = true;

    // Create texture
    D3D11_TEXTURE2D_DESC pDesc;
    pDesc.Width = w;
    pDesc.Height = h;
    pDesc.MipLevels = bGenMips ? 0 : 1;
    pDesc.ArraySize = 1;
    pDesc.Format = format;
    pDesc.SampleDesc.Count = 1;
    pDesc.SampleDesc.Quality = 0;
    pDesc.Usage = D3D11_USAGE_DEFAULT;
    pDesc.BindFlags = (bGenMips ? D3D11_BIND_RENDER_TARGET : 0) | D3D11_BIND_SHADER_RESOURCE;
    pDesc.CPUAccessFlags = 0;
    pDesc.MiscFlags = bGenMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
    
    D3D11_SUBRESOURCE_DATA pData;
    pData.pSysMem = data;
    pData.SysMemPitch = channels * w; // UINT(bpp * w / 8);
    pData.SysMemSlicePitch = 0;

    // Create texture
    auto res = gDirectX->gDevice->CreateTexture2D(&pDesc, bGenMips ? nullptr : (&pData), &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create texture!" << std::endl;
        return;
    }

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
    ZeroMemory(&pSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    pSRVDesc.Format = format;
    pSRVDesc.Texture2D.MipLevels = -1;
    pSRVDesc.Texture2D.MostDetailedMip = 0;
    pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    
    res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res) ) {
        std::cout << "Failed to create shader resource view!" << std::endl;
        return;
    }

    // Put initial sub resource data to texture if we
    // Auto-Generating mip maps
    if( bGenMips ) {
        ID3D11Texture2D *pStaging = 0;
        CD3D11_TEXTURE2D_DESC pStagingDesc(format, w, h, 1, 1, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ, 1, 0, 0);

        res = gDirectX->gDevice->CreateTexture2D(&pStagingDesc, &pData, &pStaging);
        if( FAILED(res) ) {
            std::cout << "Failed to create staging texture!" << std::endl;
            return;
        }

        // 
        gDirectX->gContext->CopySubresourceRegion(pTexture, 0, 0, 0, 0, pStaging, 0, nullptr);
        pStaging->Release();

        // 
        gDirectX->gContext->UpdateSubresource(pTexture, 0, nullptr, data, pData.SysMemPitch, 0);

        // Generate mips for texture
        gDirectX->gContext->GenerateMips(pSRV);
    }
}

void Texture::Bind(Shader::ShaderType type, UINT slot) {
    if( !pSRV ) { return; }
    switch( type ) {
        case Shader::Vertex  : gDirectX->gContext->VSSetShaderResources(slot, 1, &pSRV); break;
        case Shader::Pixel   : gDirectX->gContext->PSSetShaderResources(slot, 1, &pSRV); break;
        case Shader::Geometry: gDirectX->gContext->GSSetShaderResources(slot, 1, &pSRV); break;
        case Shader::Hull    : gDirectX->gContext->HSSetShaderResources(slot, 1, &pSRV); break;
        case Shader::Domain  : gDirectX->gContext->DSSetShaderResources(slot, 1, &pSRV); break;
        case Shader::Compute : gDirectX->gContext->CSSetShaderResources(slot, 1, &pSRV); break;
    }
}

bool Texture::IsCreated() {
    return (pSRV != 0) && (pTexture != 0);
}

void Texture::Release() {
    if( pTexture ) pTexture->Release();
    if( pSRV ) pSRV->Release();
}

int Texture::GetWidth() {
    return w;
}

int Texture::GetHeight() {
    return h;
}

ID3D11ShaderResourceView* Texture::GetSRV() {
    return pSRV;
}

ID3D11Texture2D* Texture::GetTexture() {
    return pTexture;
}
