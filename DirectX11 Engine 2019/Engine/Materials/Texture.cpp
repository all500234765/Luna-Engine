#include "pc.h"
#include "Texture3.h"

Texture3::Texture3() {
}

Texture3::Texture3(UINT Width, UINT Height, DXGI_FORMAT format, bool UAV, bool Depth, bool CPURead): w(Width), h(Height) {
    channels    = Format2Ch(format);
    bDepth      = Depth;
    bCPURead    = CPURead;
    UINT bpp    = Format2BPP(format);
    UINT mpitch = UINT((uint64_t(w) * bpp + 7) / 8);
    Create(nullptr, format, bpp, 0, mpitch, UAV);
}

Texture3::Texture3(std::wstring fname, bool UAV, bool bGenMips2) {
    //DirectX::CreateDDSTextureFromFile(gDirectX->gDevice);
    DirectX::CreateDDSTextureFromFile(gDirectX->gDevice, fname.c_str(), (ID3D11Resource**)&pTexture, &pSRV);

    // Create UAV
    if( UAV ) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC pUAVDesc = {};
        pUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        pUAVDesc.Format = pDesc.Format;
        pUAVDesc.Texture2D.MipSlice = 0;

        HRESULT hr = S_OK;
        if( (hr = gDirectX->gDevice->CreateUnorderedAccessView(pTexture, &pUAVDesc, &pUAV)) != S_OK ) {
            std::cout << "Failed to create UAV for Texture2D (error=" << hr << ")" << std::endl;
            return;
        }

        std::cout << "Successfully created UAV for Texture2D" << std::endl;
    }
}

Texture3::Texture3(std::string fname, UINT bpc, bool UAV) {
    Load(fname, bpc, UAV);
}

Texture3::Texture3(std::string fname, DXGI_FORMAT format, bool UAV) {
    Load(fname, format, UAV);
}

void Texture3::Load(std::string fname, UINT bpc, bool UAV, bool bGenMips) {
    // Load texture
    std::string ext = GetFileExtension(fname);

    void* data;
    bool bStbi      = true;
    UINT bpp        = 32;
    UINT SlicePitch = 0;
    UINT MemPitch   = 0;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    if( ext == "dds" ) {
        bStbi = false;

        // Load DDS Separatly
        using namespace tinyddsloader;
        DDSFile dds;
        
        auto res = dds.Load(fname.c_str());
        if( res != Result::Success ) {
            std::cout << "Failed to load texture. (" << fname.c_str() << ")" << std::endl;
            return;
        }

        auto data_1 = dds.GetImageData(0, 0);

        format     = (DXGI_FORMAT)dds.GetFormat();
        channels   = Format2Ch(format);
        w          = data_1->m_width;
        h          = data_1->m_height;
        data       = (void*)&data_1->m_mem;
        MemPitch   = data_1->m_memPitch;
        SlicePitch = data_1->m_memSlicePitch;
        bpp        = Format2BPP(format);
    } else {
        // Load PNG, BMP, JPG, JPEG, HDR, GIF, TGA and etc...
        data = stbi_load(fname.c_str(), &w, &h, &channels, 0);
        if( !data ) {
            std::cout << "Failed to load texture. (" << fname.c_str() << ")" << std::endl;
            return;
        }

        // Select format
        switch( channels ) {
            case 1:
                // Red channel
                switch( bpc ) {
                    case 8: format = DXGI_FORMAT_R8_UNORM; break;
                    case 16: format = DXGI_FORMAT_R16_FLOAT; break;
                    case 32: format = DXGI_FORMAT_R32_FLOAT; break;
                }
                break;

            case 2:
                // Red, Green channels
                switch( bpc ) {
                    case 8: format = DXGI_FORMAT_R8G8_UNORM; break;
                    case 16: format = DXGI_FORMAT_R16G16_FLOAT; break;
                    case 32: format = DXGI_FORMAT_R32G32_FLOAT; break;
                }
                break;

            case 3:
                // Red, Green, Blue channels
                if( bpc == 32 ) format = DXGI_FORMAT_R32G32B32_FLOAT;
                else channels = 4; // ?..
                break;

            case 4:
                // Red, Green, Blue, Alpha channels
                switch( bpc ) {
                    case 8: format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
                    case 16: format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
                    case 32: format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
                }
                break;
        }

        bpp = Format2BPP(format);
        MemPitch = UINT((uint64_t(w) * bpp + 7) / 8);
    }

    // Create Texture3 and SRV
    Create(data, format, bpp, SlicePitch, MemPitch, UAV, bGenMips);

    if( bStbi ) {
        stbi_image_free(data);
    }
}

void Texture3::Load(std::string fname, DXGI_FORMAT format, bool UAV, bool bGenMips) {
    // Load texture
    std::string ext = GetFileExtension(fname);

    void* data;
    bool bStbi = true;
    UINT bpp   = 32;
    UINT SlicePitch = 0;
    UINT MemPitch   = 0;
    if( ext == "dds" ) {
        bStbi = false;

        // Load DDS Separatly
        using namespace tinyddsloader;
        DDSFile dds;
        
        auto res = dds.Load(fname.c_str());
        if( res != Result::Success ) {
            std::cout << "Failed to load texture. (" << fname.c_str() << ")" << std::endl;
            return;
        }

        auto data_1 = dds.GetImageData(0, 0);

        format = (DXGI_FORMAT)dds.GetFormat();
        bpp = Format2BPP(format);

        w          = data_1->m_width;
        h          = data_1->m_height;
        data       = data_1->m_mem;
        SlicePitch = data_1->m_memSlicePitch;
        channels   = Format2Ch(format);
        MemPitch   = data_1->m_memPitch;

        MemPitch = UINT((uint64_t(w) * bpp + 7) / 8);
    } else {
        // Load PNG, BMP, JPG, JPEG, HDR, GIF, TGA and etc...
        data = stbi_load(fname.c_str(), &w, &h, &channels, 0);
        if( !data ) {
            std::cout << "Failed to load texture. (" << fname.c_str() << ")" << std::endl;
            return;
        }

        bpp = Format2BPP(format);
        MemPitch = UINT((uint64_t(w) * bpp + 7) / 8);
    }

    // Create Texture3 and SRV
    Create(data, format, bpp, SlicePitch, MemPitch, UAV, bGenMips);

    if( bStbi ) {
        stbi_image_free(data);
    }
}


void Texture3::Load(std::string fname, bool UAV, bool bGenMips2) {
    // Load texture
    std::string ext = GetFileExtension(fname);

    void* data;
    bool bStbi         = true;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    UINT bpp           = 32;
    UINT SlicePitch    = 0;
    UINT MemPitch      = 0;
    if( ext == "dds" ) {
        bStbi = false;

        // Load DDS Separatly
        using namespace tinyddsloader;
        DDSFile dds;

        auto res = dds.Load(fname.c_str());
        if( res != Result::Success ) {
            std::cout << "Failed to load Texture2D. (" << fname.c_str() << ")" << std::endl;
            return;
        }

        const DDSFile::ImageData* data_1 = dds.GetImageData(0, 0);

        format     = (DXGI_FORMAT)dds.GetFormat();
        bpp        = Format2BPP(format);
        channels   = Format2Ch(format);
        w          = data_1->m_width;
        h          = data_1->m_height;
        data       = (void*)(&data_1->m_mem);
        MemPitch   = data_1->m_memPitch;
        SlicePitch = data_1->m_memSlicePitch;

        // Convert string to wstring
        std::wstring wfname;
        wfname.assign(fname.begin(), fname.end());

        HRESULT hr = DirectX::CreateDDSTextureFromFile(gDirectX->gDevice, wfname.c_str(),
                                                         (ID3D11Resource**)&pTexture, &pSRV);
        if( FAILED(hr) ) {
            std::cout << "Failed to create DDS Texture2D from memory." << std::endl;
        }

        return;

        //MemPitch = UINT((uint64_t(w) * bpp + 7) / 8);
    } else {
        // Load PNG, BMP, JPG, JPEG, HDR, GIF, TGA and etc...
        data = stbi_load(fname.c_str(), &w, &h, &channels, 0);
        if( !data ) {
            std::cout << "Failed to load Texture2D. (" << fname.c_str() << ")" << std::endl;
            return;
        }

        format   = Channel2Format(channels, 8);
        bpp      = Format2BPP(format);
        MemPitch = UINT((uint64_t(w) * bpp + 7) / 8);
    }

    // Create Texture3 and SRV
    bool bGenMips = (data != nullptr) ? bGenMips2 : false;

    // if bDepth is set to true, then we must 
    // Use next formats, based on given bpp
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

    // Create texture
    pDesc.Width              = w;
    pDesc.Height             = h;
    pDesc.MipLevels          = bGenMips ? 0 : 1;
    pDesc.ArraySize          = 1;
    pDesc.Format             = bDepth ? formatTex : format;
    pDesc.SampleDesc.Count   = 1;
    pDesc.SampleDesc.Quality = 0;
    pDesc.Usage              = D3D11_USAGE_DEFAULT;
    pDesc.BindFlags          = (bGenMips ? D3D11_BIND_RENDER_TARGET : 0) | D3D11_BIND_SHADER_RESOURCE | (UAV ? D3D11_BIND_UNORDERED_ACCESS : 0);
    pDesc.CPUAccessFlags     = 0;
    pDesc.MiscFlags          = bGenMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
    
    // Allocate memory for empty texture
    if( !data ) {
        data = malloc(w * h * bpp / 8);
    }

    D3D11_SUBRESOURCE_DATA pData;
    //pData = new D3D11_SUBRESOURCE_DATA[1];
    pData.pSysMem          = data;
    pData.SysMemPitch      = MemPitch;
    pData.SysMemSlicePitch = SlicePitch;

    // Create texture
    auto res = gDirectX->gDevice->CreateTexture2D(&pDesc, bGenMips ? nullptr : &pData, &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create texture!" << std::endl;
        return;
    }

    // Create UAV
    if( UAV ) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC pUAVDesc = {};
        pUAVDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
        pUAVDesc.Format              = pDesc.Format;
        pUAVDesc.Texture2D.MipSlice  = 0;

        HRESULT hr = S_OK;
        if( (hr = gDirectX->gDevice->CreateUnorderedAccessView(pTexture, &pUAVDesc, &pUAV)) != S_OK ) {
            std::cout << "Failed to create UAV for Texture2D (error=" << hr << ")" << std::endl;
            return;
        }

        std::cout << "Successfully created UAV for Texture2D" << std::endl;
    }

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
    ZeroMemory(&pSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    pSRVDesc.Format                    = bDepth ? formatSRV : format;
    pSRVDesc.Texture2D.MipLevels       = -1;
    pSRVDesc.Texture2D.MostDetailedMip = 0;
    pSRVDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    
    res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res) ) {
        std::cout << "Failed to create shader resource view!" << std::endl;
        return;
    }

    // Put initial sub resource data to texture if we
    // Auto-Generating mip maps
    if( bGenMips ) {
        /*D3D11_TEXTURE2D_DESC desc2;
        desc2.Format             = format;
        desc2.ArraySize          = 1;
        desc2.MipLevels          = -1;
        desc2.BindFlags          = flags;
        desc2.Usage              = D3D11_USAGE_IMMUTABLE; // GPU Read only?
        desc2.MiscFlags          = 0;
        desc2.CPUAccessFlags     = 0;
        desc2.SampleDesc.Count   = 1;
        desc2.SampleDesc.Quality = 0;
        desc2.Width              = w;
        desc2.Height             = h;*/

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
        //gDirectX->gContext->UpdateSubresource(pTexture, 0, nullptr, data, pData.SysMemPitch, 0);

        // Generate mips for texture
        gDirectX->gContext->GenerateMips(pSRV);
    }

    std::cout << "Successfully created Texture2D" << std::endl;

    if( bStbi ) {
        stbi_image_free(data);
    } else {
        //dds.;
    }
}

void Texture3::Create(void* data, DXGI_FORMAT format, UINT bpp, UINT SlicePitch, UINT MemPitch, bool UAV, bool bGenMips2) {
    bool bGenMips = (data != nullptr) ? bGenMips2 : false;

    // if bDepth is set to true, then we must 
    // Use next formats, based on given bpp
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

    // Create texture
    pDesc.Width              = w;
    pDesc.Height             = h;
    pDesc.MipLevels          = bGenMips ? 0 : 1;
    pDesc.ArraySize          = 1;
    pDesc.Format             = bDepth ? formatTex : format;
    pDesc.SampleDesc.Count   = 1;
    pDesc.SampleDesc.Quality = 0;
    pDesc.Usage              = bCPURead ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
    pDesc.BindFlags          = bCPURead ? 0 : ((bGenMips ? D3D11_BIND_RENDER_TARGET : 0) | D3D11_BIND_SHADER_RESOURCE | (UAV ? D3D11_BIND_UNORDERED_ACCESS : 0));
    pDesc.CPUAccessFlags     = bCPURead ? D3D11_CPU_ACCESS_READ : 0;
    pDesc.MiscFlags          = bCPURead ? 0 : (bGenMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
    
    // Allocate memory for empty texture
    if( !data ) {
        data = malloc(w * h * bpp / 8);
    }

    D3D11_SUBRESOURCE_DATA pData;
    pData.pSysMem          = data;
    pData.SysMemPitch      = MemPitch;
    pData.SysMemSlicePitch = SlicePitch;

    // Create texture
    auto res = gDirectX->gDevice->CreateTexture2D(&pDesc, bGenMips ? nullptr : &pData, &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create texture!" << std::endl;
        return;
    }

    // Create UAV
    if( UAV && !bCPURead ) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC pUAVDesc = {};
        pUAVDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
        pUAVDesc.Format              = pDesc.Format;
        pUAVDesc.Texture2D.MipSlice  = 0;

        HRESULT hr = S_OK;
        if( (hr = gDirectX->gDevice->CreateUnorderedAccessView(pTexture, &pUAVDesc, &pUAV)) != S_OK ) {
            std::cout << "Failed to create UAV for Texture2D (error=" << hr << ")" << std::endl;
            return;
        }

        std::cout << "Successfully created UAV for Texture2D" << std::endl;
    }

    // Create SRV
    if( !bCPURead ) {
        D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
        ZeroMemory(&pSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        pSRVDesc.Format                    = bDepth ? formatSRV : format;
        pSRVDesc.Texture2D.MipLevels       = -1;
        pSRVDesc.Texture2D.MostDetailedMip = 0;
        pSRVDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        
        res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
        if( FAILED(res) ) {
            std::cout << "Failed to create shader resource view!" << std::endl;
            return;
        }
    }

    // Put initial sub resource data to texture if we
    // Auto-Generating mip maps
    if( bGenMips && !bCPURead ) {
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

    std::cout << "Successfully created Texture2D" << std::endl;
}

void Texture3::Bind(Shader::ShaderType type, UINT slot, bool UAV) {
    if( !pSRV ) { return; }

    if( type & Shader::Vertex   ) gDirectX->gContext->VSSetShaderResources(slot, 1, &pSRV);
    if( type & Shader::Pixel    ) gDirectX->gContext->PSSetShaderResources(slot, 1, &pSRV);
    if( type & Shader::Geometry ) gDirectX->gContext->GSSetShaderResources(slot, 1, &pSRV);
    if( type & Shader::Hull     ) gDirectX->gContext->HSSetShaderResources(slot, 1, &pSRV);
    if( type & Shader::Domain   ) gDirectX->gContext->DSSetShaderResources(slot, 1, &pSRV);
    if( type & Shader::Compute  ) {
        if( UAV ) {
            UINT pInitial = { 0 };
            gDirectX->gContext->CSSetUnorderedAccessViews(slot, 1, &pUAV, &pInitial);
        } else {
            gDirectX->gContext->CSSetShaderResources(slot, 1, &pSRV);
        }
    }
}

void Texture3::Release() {
    if( pTexture ) pTexture->Release();
    if( pSRV ) pSRV->Release();
    if( pUAV ) pUAV->Release();
}

void Texture3::Resize(UINT Width, UINT Height, bool Save) {
    bool bGenMips = !pDesc.MipLevels;

    pDesc.Width = Width;
    pDesc.Height = Height;

    // if bDepth is set to true, then we must 
    // Use next formats, based on given bpp
    DXGI_FORMAT formatTex, formatDSV, formatSRV;
    switch( Format2BPP(pDesc.Format) ) {
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

    // Recreate texture
    if( pTexture ) {
        // Save old data
        ID3D11Texture2D *pTextureTemp;
        gDirectX->gDevice->CreateTexture2D(&pDesc, nullptr, &pTextureTemp);
        gDirectX->gContext->CopyResource(pTextureTemp, pTexture);

        // Recreate texture
        pTexture->Release();
        gDirectX->gDevice->CreateTexture2D(&pDesc, nullptr, &pTexture);

        // Re-create SRV
        if( pSRV ) {
            pSRV->Release();

            D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
            ZeroMemory(&pSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            pSRVDesc.Format                    = bDepth ? formatSRV : pDesc.Format;
            pSRVDesc.Texture2D.MipLevels       = -1;
            pSRVDesc.Texture2D.MostDetailedMip = 0;
            pSRVDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;

            HRESULT res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
            if( FAILED(res) ) {
                std::cout << "Failed to create shader resource view!" << std::endl;
                return;
            }

            // Restore old data
            if( !bGenMips ) {
                gDirectX->gContext->CopyResource(pTexture, pTextureTemp);
            } else {
                ID3D11Texture2D *pStaging = 0;
                CD3D11_TEXTURE2D_DESC pStagingDesc(pDesc.Format, w, h, 1, 1, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ, 1, 0, 0);

                res = gDirectX->gDevice->CreateTexture2D(&pStagingDesc, nullptr, &pStaging);
                if( FAILED(res) ) {
                    std::cout << "Failed to create staging texture!" << std::endl;
                    return;
                }

                // 
                gDirectX->gContext->CopyResource(pStaging, pTextureTemp);

                // 
                gDirectX->gContext->CopyResource(pStaging, pTexture);
                //gDirectX->gContext->CopySubresourceRegion(pTexture, 0, 0, 0, 0, pStaging, 0, nullptr);
                pStaging->Release();

                // 
                //gDirectX->gContext->UpdateSubresource(pTexture, 0, nullptr, data, pData.SysMemPitch, 0);

                // Generate mips for texture
                gDirectX->gContext->GenerateMips(pSRV);
            }

            // Delete temp texture
            pTextureTemp->Release();
        }
    }
    if( pUAV ) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC pUAVDesc = {};
        pUAVDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
        pUAVDesc.Format             = pDesc.Format;
        pUAVDesc.Texture2D.MipSlice = 0;

        pUAV->Release();

        HRESULT hr = S_OK;
        if( (hr = gDirectX->gDevice->CreateUnorderedAccessView(pTexture, &pUAVDesc, &pUAV)) != S_OK ) {
            std::cout << "Failed to re-create UAV for Texture2D (error=" << hr << ")" << std::endl;
            return;
        }

        std::cout << "Successfully re-created UAV for Texture2D" << std::endl;
    }
}
