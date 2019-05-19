#include "CubemapTexture.h"

void CubemapTexture::CreateFromFiles(std::string folder, bool bDepth, DXGI_FORMAT format) {
    isLoaded = true;
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
    desc.MiscFlags = 0;
    desc.CPUAccessFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    D3D11_SUBRESOURCE_DATA *pSubres = new D3D11_SUBRESOURCE_DATA[6];

    // Load subresource data
    for( int i = 0; i < 6; i++ ) {
        // Load file
        std::string fname = folder + sSideNames[i];
        stbi_set_flip_vertically_on_load(true); // Flip texture
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
    pSRVDesc.Texture2DArray.MipLevels       = 1;
    pSRVDesc.Texture2DArray.MostDetailedMip = 0;
    pSRVDesc.Texture2DArray.ArraySize       = 1;
    pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

    // Create SRVs
    for( int i = 0; i < 6; i++ ) {
        pSRVDesc.Texture2DArray.FirstArraySlice = i;
        
        res = gDirectX->gDevice->CreateShaderResourceView(pTexture, &pSRVDesc, &pSRV[i]);
        if( FAILED(res) ) {
            std::cout << "Failed to create shader resource view!" << std::endl;
            return;
        }
    }
}

void CubemapTexture::Release() {
    if( pTexture ) pTexture->Release();
    for( int i = 0; i < 6; i++ ) {
        if( pSRV[i] ) pSRV[i]->Release();
        
        if( isLoaded ) { continue; }
        if( isDepthMap ) {
            if( pDSVs[i] ) pDSVs[i]->Release();
        } else {
            if( pRTVs[i] ) pRTVs[i]->Release();
        }
    }
}
