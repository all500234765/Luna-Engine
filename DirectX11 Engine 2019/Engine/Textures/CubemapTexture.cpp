#include "CubemapTexture.h"

void CubemapTexture::CreateFromFiles(std::string folder, bool bDepth, DXGI_FORMAT format) {
    isLoaded   = true;
    isDepthMap = bDepth;

    // 
    UINT flags = D3D11_BIND_SHADER_RESOURCE;
    //if( isDepthMap ) {
    //    flags |= D3D11_BIND_DEPTH_STENCIL;
    //} else {
    //    flags |= D3D11_BIND_RENDER_TARGET;
    //}

    // Create texture description
    D3D11_TEXTURE2D_DESC desc;
    desc.Format = format;
    desc.ArraySize = 6;
    desc.MipLevels = 1;
    desc.BindFlags = flags;
    desc.Usage = D3D11_USAGE_IMMUTABLE; // GPU Read only
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.CPUAccessFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    D3D11_SUBRESOURCE_DATA *pSubres = new D3D11_SUBRESOURCE_DATA[6];

    // Load subresource data
    for( int i = 0; i < 6; i++ ) {
        // Load file
        std::string fname = folder + sSideNames[i] + ".png";
        void* data = stbi_load(fname.c_str(), &Width, &Height, &channels, 0);

        // Set data
        pSubres[i].SysMemSlicePitch = 0;

        if( data ) {
            pSubres[i].pSysMem = data;
            pSubres[i].SysMemPitch = channels * Width;
        } else {
            pSubres[i].pSysMem = nullptr;
            pSubres[i].SysMemPitch = 0;
        }
    }

    // Set side size
    desc.Width = Width;
    desc.Height = Height;

    // Create texture
    auto res = gDirectX->gDevice->CreateTexture2D(&desc, pSubres, &pTexture);
    if( FAILED(res) ) {
        std::cout << "Failed to create texture!" << std::endl;
        return;
    }

    // Free memory
    for( int i = 0; i < 6; i++ ) {
        stbi_image_free((void*)pSubres[i].pSysMem);
    }

    // Freeeeeeeeeee moar mem
    delete[] pSubres;

    // Create SRV desc
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
    ZeroMemory(&pSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    pSRVDesc.Format = format;
    pSRVDesc.Texture2DArray.MipLevels       = desc.MipLevels;
    pSRVDesc.Texture2DArray.MostDetailedMip = 0;
    pSRVDesc.Texture2DArray.ArraySize       = 6;
    pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    pSRVDesc.Texture2DArray.FirstArraySlice = 0;

    // Create SRV
    res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res) ) {
        std::cout << "Failed to create shader resource view!" << std::endl;
        return;
    }
}

void CubemapTexture::CreateFromDDS(std::string fname, bool bDepth) {
    isLoaded = true;
    isDepthMap = bDepth;

    // Load dds
    using namespace tinyddsloader;
    DDSFile dds;

    auto res = dds.Load(fname.c_str());
    if( res != Result::Success ) {
        std::cout << "Failed to load cubemap. (" << fname.c_str() << ")" << std::endl;
        return;
    }

    // Must be cubemap
    if( !dds.IsCubemap() ) {
        std::cout << "Failed to load dds, that isn't cubemap. (" << fname.c_str() << ")" << std::endl;
        return;
    }

    // Flag control
    UINT flags        = D3D11_BIND_SHADER_RESOURCE;
    UINT misc         = D3D11_RESOURCE_MISC_TEXTURECUBE;
    D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE; // GPU Read only
    bool bMipMaps = false;
    UINT mips     = dds.GetMipCount();

    if( mips > 1 ) {
        //flags   |= D3D11_BIND_RENDER_TARGET;
        //misc    |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        usage = D3D11_USAGE_DEFAULT;
        bMipMaps = true;
    }

    // Create texture description
    D3D11_TEXTURE2D_DESC desc;
    desc.Format             = (DXGI_FORMAT)dds.GetFormat();
    desc.ArraySize          = 6;
    desc.MipLevels          = mips;
    desc.BindFlags          = flags;
    desc.Usage              = usage;
    desc.MiscFlags          = misc;
    desc.CPUAccessFlags     = 0;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;

    // 
    Width    = dds.GetWidth();
    Height   = dds.GetHeight();
    channels = int(Format2BPP(desc.Format) / 8); // TODO: Wrong channel calculation
    //                                                    Replace with Format2Ch(format);

    // Set side size
    desc.Width  = Width;
    desc.Height = Height;

    // 
    D3D11_SUBRESOURCE_DATA *pSubres;
    //if( !bMipMaps ) 
    {
        pSubres = new D3D11_SUBRESOURCE_DATA[6 * mips];

        // Load subresource data
        size_t index = 0;
        for( int i = 0; i < 6; i++ ) {
            for( int mip = 0; mip < mips; mip++ ) {
                // Set data
                const DDSFile::ImageData *q = dds.GetImageData(mip, i);

                pSubres[index].SysMemSlicePitch = q->m_memSlicePitch;
                pSubres[index].pSysMem = q->m_mem;
                pSubres[index].SysMemPitch = q->m_memPitch;
                index++;
            }
        }
    }

    // Create texture
    auto res2 = gDirectX->gDevice->CreateTexture2D(&desc, bMipMaps ? nullptr : pSubres, &pTexture);
    if( FAILED(res2) ) {
        std::cout << "Failed to create cubemap texture!" << std::endl;
        return;
    }

    // Load data
    if( bMipMaps ) {
        // Load textures
        ID3D11Texture2D *pTex[6];
        D3D11_TEXTURE2D_DESC desc2;
        desc2.Format = (DXGI_FORMAT)dds.GetFormat();
        desc2.ArraySize = 1;
        desc2.MipLevels = dds.GetMipCount();
        desc2.BindFlags = flags;
        desc2.Usage = usage; // GPU Read only?
        desc2.MiscFlags = 0;
        desc2.CPUAccessFlags = 0;
        desc2.SampleDesc.Count = 1;
        desc2.SampleDesc.Quality = 0;
        desc2.Width = Width;
        desc2.Height = Height;

        // Create texture
        for( UINT i = 0; i < 6; i++ ) {
            auto res2 = gDirectX->gDevice->CreateTexture2D(&desc2, &pSubres[i * mips], &pTex[i]);
            if( FAILED(res2) ) {
                std::cout << "Failed to create cubemap texture!" << std::endl;
                return;
            }
        }

        // Load mip data
        D3D11_BOX src;

        for( UINT i = 0; i < 6; i++ ) {
            for( UINT mip = 0; mip < mips; mip++ ) {
                src.left   = 0;
                src.right  = desc.Width >> mip;
                src.top    = 0;
                src.bottom = desc.Height >> mip;
                src.front  = 0;
                src.back   = 1;

                // Overflow test
                if( src.bottom == 0 || src.right == 0 ) { break; }

                gDirectX->gContext->CopySubresourceRegion(pTexture, D3D11CalcSubresource(mip, i, mips), 
                                                          0, 0, 0, pTex[i], mip, &src);
            }
        }
    }

    // Create SRV desc
    D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc;
    ZeroMemory(&pSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    pSRVDesc.Format                         = desc.Format;
    pSRVDesc.Texture2DArray.MipLevels       = desc.MipLevels;
    pSRVDesc.Texture2DArray.FirstArraySlice = 0;
    pSRVDesc.Texture2DArray.MostDetailedMip = 0;
    pSRVDesc.Texture2DArray.ArraySize       = 6;
    pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

    // Create SRV
    res2 = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV);
    if( FAILED(res2) ) {
        std::cout << "Failed to create shader resource view!" << std::endl;
        return;
    }
}

void CubemapTexture::Bind(Shader::ShaderType type, UINT slot) {
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

void CubemapTexture::Release() {
    if( pTexture ) pTexture->Release();
    if( pSRV ) pSRV->Release();

    if( isLoaded ) { return; }
    for( int i = 0; i < 6; i++ ) {
        if( isDepthMap ) {
            if( pDSVs[i] ) pDSVs[i]->Release();
        } else {
            if( pRTVs[i] ) pRTVs[i]->Release();
        }
    }
}
