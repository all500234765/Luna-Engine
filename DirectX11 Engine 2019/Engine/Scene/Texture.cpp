#include "pc.h"
#include "Texture.h"

implTexture* Texture::CreateTexture(DXGI_FORMAT format, D3D11_SUBRESOURCE_DATA *SubResource, 
                                     uint32_t ArraySize, implTexture *Out) {
    union local_texture {
        ID3D11Texture1D* _1D;
        ID3D11Texture2D* _2D;
        ID3D11Texture3D* _3D;
    } pTexture{};

    ID3D11ShaderResourceView  *pSRV = 0;
    ID3D11UnorderedAccessView *pUAV = 0;

    // MSAA
    uint32_t Quality = 0;
    uint32_t SampleCount = 1;

    // 
    HRESULT res = S_FALSE;
    UINT BindFlags = D3D11_BIND_SHADER_RESOURCE | 
                     (HasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0)
                   | (HasMipMaps & !MSAA && !FormatBC(format) ? D3D11_BIND_RENDER_TARGET : 0);
    UINT MipMaps      = MSAA ? 1 : (HasMipMaps ? ((mMipMaps == 1) ? 0 : mMipMaps) : 1);
    UINT CPUAccess    = (CPURead    ? D3D11_CPU_ACCESS_READ : 0) | (CPUWrite ? D3D11_CPU_ACCESS_WRITE : 0);
    D3D11_USAGE Usage = (CPURead    ? D3D11_USAGE_STAGING : (CPUWrite ? D3D11_USAGE_DYNAMIC : (Immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT)));
    UINT Misc         = (HasMipMaps ? (MSAA ? 0 : ((MipMaps <= 1) && !FormatBC(format) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0)) : 0) |
                        (IsCube     ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0) | 
                        (ClampMip   ? D3D11_RESOURCE_MISC_RESOURCE_CLAMP : 0) | 
                        (RContent   ? D3D11_RESOURCE_MISC_RESTRICTED_CONTENT : 0) | 
                        (HWRContent ? D3D11_RESOURCE_MISC_HW_PROTECTED : 0) |
                        (IsTilePool ? D3D11_RESOURCE_MISC_TILE_POOL : 0) |
                        (IsTiled    ? D3D11_RESOURCE_MISC_TILED : 0);
    D3D11_SUBRESOURCE_DATA *subres = ((MipMaps > 1 || HasMipMaps) && (Usage != D3D11_USAGE_IMMUTABLE)) ? nullptr : SubResource;
    
    // No CPU access flags can be specified.
    // This flag cannot be used with the following D3D11_USAGE values:
    // - D3D11_USAGE_DYNAMIC
    // - D3D11_USAGE_STAGING
    if( HWRContent ) {
        CPUAccess = 0;
        Usage = (Immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT);
    }

    // Try to create a texture
    auto CreateTextureLocal = [format, &res]
               (uint32_t dim, uint32_t mWidth, uint32_t mHeight, uint32_t mDepth, uint32_t ArraySize, 
                uint32_t MipMaps, uint32_t BindFlags, uint32_t CPUAccess, D3D11_USAGE Usage,
                uint32_t SampleCount, uint32_t Quality, uint32_t Misc, D3D11_SUBRESOURCE_DATA* subres, 
                local_texture& pTexture) {

        res = S_FALSE;
        if( dim == 1 ) {
            // Create Texture 1D
            D3D11_TEXTURE1D_DESC pTexDesc = {};
            pTexDesc.ArraySize      = ArraySize;
            pTexDesc.MipLevels      = MipMaps;
            pTexDesc.BindFlags      = BindFlags;
            pTexDesc.Usage          = Usage;
            pTexDesc.CPUAccessFlags = CPUAccess;
            pTexDesc.MiscFlags      = Misc;
            pTexDesc.Format         = format;
            pTexDesc.Width          = mWidth;

            res = gDirectX->gDevice->CreateTexture1D(&pTexDesc, subres, &pTexture._1D);
        } else if( dim == 2 ) {
            // Create Texture 2D
            D3D11_TEXTURE2D_DESC pTexDesc = {};
            pTexDesc.ArraySize          = ArraySize;
            pTexDesc.MipLevels          = MipMaps;
            pTexDesc.BindFlags          = BindFlags;
            pTexDesc.Usage              = Usage;
            pTexDesc.CPUAccessFlags     = CPUAccess;
            pTexDesc.MiscFlags          = Misc;
            pTexDesc.Format             = format;
            pTexDesc.Width              = mWidth;
            pTexDesc.Height             = mHeight;
            pTexDesc.SampleDesc.Count   = SampleCount;
            pTexDesc.SampleDesc.Quality = (UINT)std::max((int)Quality - 1, 0);

            res = gDirectX->gDevice->CreateTexture2D(&pTexDesc, subres, &pTexture._2D);
        } else if( dim == 3 ) {
            // Create Texture 3D
            D3D11_TEXTURE3D_DESC pTexDesc = {};
            pTexDesc.MipLevels      = MipMaps;
            pTexDesc.BindFlags      = BindFlags;
            pTexDesc.Usage          = Usage;
            pTexDesc.CPUAccessFlags = CPUAccess;
            pTexDesc.MiscFlags      = Misc;
            pTexDesc.Format         = format;
            pTexDesc.Width          = mWidth;
            pTexDesc.Height         = mHeight;
            pTexDesc.Depth          = mDepth;
                
            res = gDirectX->gDevice->CreateTexture3D(&pTexDesc, subres, &pTexture._3D);
        }
    };

    CreateTextureLocal(dim, mWidth, mHeight, mDepth, ArraySize, MipMaps, BindFlags, CPUAccess, Usage,
                        SampleCount, Quality, Misc, subres, pTexture);

    if( FAILED(res) ) {
        printf_s("[Texture::Create]: Failed to create %uD texture! [%s]\n", dim, mName.data());
    } else if( (MipMaps > 1 || HasMipMaps) && SubResource ) {
        // Load mip data
        D3D11_BOX src;
        UINT mips = std::max(1u, MipMaps);

        for( UINT i = 0, j = ArraySize; i < j; i++ ) {
            local_texture pTexLocal{};

            CreateTextureLocal(dim, mWidth, mHeight, mDepth, 1, mips, 0, D3D11_CPU_ACCESS_READ,
                               D3D11_USAGE_STAGING, 1, 0, 0, &SubResource[i * mips], pTexLocal);
            
            if( FAILED(res) ) {
                printf_s("[Texture::Create::CopySubresourceRegion]: Failed to create %uD texture for %u array slice! [%s]\n", 
                            dim, i, mName.data());

                if( pTexLocal._3D ) {
                    if( dim == 1 ) pTexLocal._1D->Release();
                    if( dim == 2 ) pTexLocal._2D->Release();
                    if( dim == 3 ) pTexLocal._3D->Release();
                }

                continue;
            }

            // Debug
            //char buff[32];
            //sprintf_s(&buff[0], 32, "[Texture]: Test %u", i);
            //_SetName(pTexLocal._3D, buff);

            for( UINT mip = 0; mip < mips; mip++ ) {
                src.left   = 0;
                src.right  = mWidth >> mip;

                src.top    = 0;
                src.bottom = mHeight >> mip;

                src.front  = 0;
                src.back   = mDepth >> mip;

                // Overflow test
                if( src.bottom == 0 || (src.right == 0 && (dim >= 2)) || (src.back == 0 && (dim == 3)) ) { break; }
                src.right = std::max(src.right, 1u); // 1D
                src.back  = std::max(src.back , 1u); // 1D, 2D

                gDirectX->gContext->CopySubresourceRegion(pTexture._3D, D3D11CalcSubresource(mip, i, MipMaps),
                                                          0, 0, 0, pTexLocal._3D, mip, &src);
            }

            if( dim == 1 ) pTexLocal._1D->Release();
            if( dim == 2 ) pTexLocal._2D->Release();
            if( dim == 3 ) pTexLocal._3D->Release();
        }
    }

    // Create UAV
    if( HasUAV ) {
        if( Usage == D3D11_USAGE_STAGING ) {
            printf_s("[Texture::Create]: Can't create UAV for D3D11_USAGE_STAGING texture! [%s]\n", mName.data());
        } else {
            if( dim == 3 && ArraySize > 1 ) {
                printf_s("[Texture::Create]: Can't create 3D UAV with ArraySize > 1! [%s]\n", mName.data());
            } else {
                D3D11_UNORDERED_ACCESS_VIEW_DESC pUAVDesc = {};
                pUAVDesc.Format        = format;
                pUAVDesc.ViewDimension = (dim == 3) ? D3D11_UAV_DIMENSION_TEXTURE3D : 
                    (D3D11_UAV_DIMENSION)((D3D11_UAV_DIMENSION_TEXTURE1D + (ArraySize > 1) * (dim < 3)) * dim);

                if( dim == 1 ) {
                    pUAVDesc.Texture1D.MipSlice = 0;

                    if( ArraySize > 1 ) {
                        pUAVDesc.Texture1DArray.MipSlice        = 0;
                        pUAVDesc.Texture1DArray.FirstArraySlice = 0;
                        pUAVDesc.Texture1DArray.ArraySize       = ArraySize;
                    }
                } else if( dim == 2 ) {
                    pUAVDesc.Texture2D.MipSlice = 0;

                    if( ArraySize > 1 ) {
                        pUAVDesc.Texture2DArray.MipSlice        = 0;
                        pUAVDesc.Texture2DArray.FirstArraySlice = 0;
                        pUAVDesc.Texture2DArray.ArraySize       = ArraySize;
                    }
                } else if( dim == 3 ) {
                    pUAVDesc.Texture3D.MipSlice    = 0;
                    pUAVDesc.Texture3D.WSize       = mDepth;
                    pUAVDesc.Texture3D.FirstWSlice = 0;
                }

                res = gDirectX->gDevice->CreateUnorderedAccessView(Choose(pTexture._1D, pTexture._2D, pTexture._3D),
                                                                    &pUAVDesc, &pUAV);
                if( FAILED(res) ) {
                    printf_s("[Texture::Create]: Failed to create UAV! [%s]\n", mName.data());
                }
            }
        }
    }
    
    // Create SRV
    if( Usage != D3D11_USAGE_STAGING ) {
        D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc = {};
        pSRVDesc.Format        = format;
        pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION(
            (dim == 1) ? (D3D11_SRV_DIMENSION_TEXTURE1D + (ArraySize > 1)) : 
            (dim == 2) ? (IsCube ? (D3D11_SRV_DIMENSION_TEXTURECUBE + (ArraySize/6 > 1)) : (D3D11_SRV_DIMENSION_TEXTURE2D + (ArraySize > 1) + 2 * (Quality > 0))) :
            (dim == 3) ? D3D11_SRV_DIMENSION_TEXTURE3D : 
            D3D11_SRV_DIMENSION_UNKNOWN
        );

        uint32_t mip_levels = ((MipMaps <= 1) && HasMipMaps) ? -1 : std::max({ 1, (int)MipMaps - 1 });

        if( dim == 1 ) {
            pSRVDesc.Texture1D.MipLevels       = mip_levels;
            pSRVDesc.Texture1D.MostDetailedMip = 0;

            if( ArraySize > 1 ) {
                pSRVDesc.Texture1DArray.ArraySize       = ArraySize;
                pSRVDesc.Texture1DArray.FirstArraySlice = 0;
            }
        } else if( dim == 2  ) {
            pSRVDesc.Texture2D.MipLevels       = mip_levels;
            pSRVDesc.Texture2D.MostDetailedMip = 0;

            if( ArraySize > 1 ) {
                if( Quality > 0 ) {
                    pSRVDesc.Texture2DMSArray.ArraySize       = ArraySize;
                    pSRVDesc.Texture2DMSArray.FirstArraySlice = 0;
                } else {
                    pSRVDesc.Texture2DArray.MipLevels       = mip_levels;
                    pSRVDesc.Texture2DArray.MostDetailedMip = 0;
                    pSRVDesc.Texture2DArray.ArraySize       = ArraySize;
                    pSRVDesc.Texture2DArray.FirstArraySlice = 0;
                }
            }

            if( IsCube ) {
                if( ArraySize == 6 ) {
                    pSRVDesc.TextureCube.MipLevels       = mip_levels;
                    pSRVDesc.TextureCube.MostDetailedMip = 0;
                } else {
                    pSRVDesc.TextureCubeArray.MipLevels        = mip_levels;
                    pSRVDesc.TextureCubeArray.MostDetailedMip  = 0;
                    pSRVDesc.TextureCubeArray.First2DArrayFace = 0;
                    pSRVDesc.TextureCubeArray.NumCubes         = floor(ArraySize / 6);
                }
            }
        } else if( dim == 3 ) {
            pSRVDesc.Texture3D.MipLevels       = mip_levels;
            pSRVDesc.Texture3D.MostDetailedMip = 0;
        }

        res = gDirectX->gDevice->CreateShaderResourceView(Choose(pTexture._1D, pTexture._2D, pTexture._3D), &pSRVDesc, &pSRV);
        if( FAILED(res) ) {
            printf_s("[Texture::Create]: Failed to create SRV! [%s]\n", mName.data());
        }

        // Generate mips for texture
        if( (MipMaps <= 1) && HasMipMaps && SubResource && !FormatBC(format) )
            gDirectX->gContext->GenerateMips(pSRV);
    }
    
    // 
    if( Out == nullptr ) Out = new implTexture();

    // Set resource
         if( dim == 1 ) Out->pTexture = pTexture._1D;
    else if( dim == 2 ) Out->pTexture = pTexture._2D;
    else                Out->pTexture = pTexture._3D;

    Out->mFormat  = format;
    Out->pSRV     = pSRV;
    Out->pUAV     = pUAV;
    Out->mFlags   = mFlags;

    return Out;
}

Texture::Texture(UINT flags, DXGI_FORMAT format, 
                   uint32_t w, uint32_t h, uint32_t d, uint32_t ArraySize, 
                   std::string_view name): bUndefined(false) {
    mFlags = flags;
    SetName(name.data());

    /*D3D11_SUBRESOURCE_DATA *SubResource = new D3D11_SUBRESOURCE_DATA[ArraySize * (1 + 5 * IsCube)];
    for( uint32_t i = 0; i < n; i++ ) {
        SubResource[i].pSysMem = ();
    }
    
    for( uint32_t i = 0; i < n; i++ ) {
        delete;
    }

    delete[] SubResource;*/
    
    mArraySize = ArraySize * (1 + 5 * IsCube);
    mWidth = w; mHeight = h; mDepth = d; mMipMaps = 1;
    mTextureUnit = CreateTexture(format, nullptr, mArraySize, nullptr);
    //SetName(mName.data());
}

Texture::Texture(std::string_view fname, UINT flags, std::string_view name, 
                   uint32_t ArraySize, DXGI_FORMAT custom_format): bUndefined(false) {
    std::string nm = ( (name == "UnnamedTexture") ? file_name(fname).data() : name.data() );
    SetName(nm);
    Load(fname, flags, ArraySize, custom_format);
    //SetName(mName.data());
}

void Texture::Load(std::string_view fname, UINT flags, uint32_t ArraySize, DXGI_FORMAT custom_format) {
    if( mTextureUnit ) { mTextureUnit->Release(); }

    mFlags = flags;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    D3D11_SUBRESOURCE_DATA *SubResource;
    uint32_t n;
    
    // Ext
    const char* ext = path_ext(fname).data();

    // File loader specific
    using namespace tinyddsloader;
    DDSFile dds;
    void* stbi_image = 0;

    // Load texture
    if( !strcmp(ext, "dds") ) {
        // Load dds file
        auto res = dds.Load(fname.data());
        if( res != Result::Success ) {

            printf_s("[Texture::Create]: Failed to load texture! [%s, %s]\n", fname.data(), mName.data());
            return;
        }

        // Set size
        mWidth  = dds.GetWidth();
        mHeight = dds.GetHeight();
        mDepth  = dds.GetDepth();

        // 
        format    = (DXGI_FORMAT)dds.GetFormat();
        mMipMaps  = dds.GetMipCount();
        ArraySize = dds.GetArraySize();

        // Update flags
        mFlags |= (dds.GetTextureDimension() == DDSFile::TextureDimension::Texture3D) ? tf_dim_3 : (1 << (UINT(dds.GetTextureDimension()) - 2));
        mFlags |= mMipMaps != 1 ? tf_MipMaps : 0;
        mFlags |= dds.IsCubemap() ? tf_Cube : 0;

        n = ArraySize;
        SubResource = new D3D11_SUBRESOURCE_DATA[n * mMipMaps];

        // Load subresource data
        size_t index = 0;
        for( int i = 0; i < n; i++ ) {
            for( int mip = 0; mip < mMipMaps; mip++ ) {
                // Set data
                const DDSFile::ImageData *q = dds.GetImageData(mip, i);

                SubResource[index].SysMemSlicePitch = q->m_memSlicePitch;
                SubResource[index].SysMemPitch      = q->m_memPitch;
                SubResource[index].pSysMem          = q->m_mem;
                index++;
            }
        }
    } else {
        // Load png/jpeg/tga/hdr/etc
        n = 1 + 0*ArraySize * (1 + 5 * IsCube);

        // Load image
        int ch = 0;
        stbi_image = stbi_load(fname.data(), (int*)&mWidth, (int*)&mHeight, &ch, 
            (custom_format == DXGI_FORMAT_UNKNOWN) ? 0*4 : 0*Format2Ch(custom_format));

        if( !stbi_image ) {
            printf_s("[Texture::Load]: Failed to load texture! [%s]\n", fname.data());
            return;
        }

        // Set default dimension
        if( dim == 0 ) { dim = 2; }
        
        // 
        format   = (custom_format == DXGI_FORMAT_UNKNOWN) ? Channel2Format(ch, 8) : custom_format;
        UINT bpp = Format2BPP(format);
        
        // Create subresource
        SubResource = new D3D11_SUBRESOURCE_DATA[n];
        SubResource[0].pSysMem          = stbi_image;
        SubResource[0].SysMemPitch      = UINT((uint64_t(mWidth) * bpp + 7) / 8);
        SubResource[0].SysMemSlicePitch = mWidth * mHeight * bpp;
    }

    // Create texture unit
    mArraySize = n;
    mTextureUnit = CreateTexture(format, SubResource, n, nullptr);
    SetName(mName.data());

    // Free resources
    delete[] SubResource;

    // Not dds
    if( strcmp(ext, "dds") ) {
        stbi_image_free(stbi_image);
    }
}

void Texture::SetSubresource(const D3D11_SUBRESOURCE_DATA* resource, UINT mip, UINT array) {

}

void Texture::Resize(uint32_t w, uint32_t h, uint32_t d, TResizeFlag SaveContent) {

    // Recreate texture
    switch( SaveContent ) {
        case TResizeFlag::Clear:
        case TResizeFlag::Keep:
        case TResizeFlag::Stretch:
        {
            mWidth = w; mHeight = h; mDepth = d;
            if( mTextureUnit->pUAV ) mTextureUnit->pUAV->Release();
            if( mTextureUnit->pSRV ) mTextureUnit->pSRV->Release();
            CreateTexture(mTextureUnit->mFormat, nullptr, mArraySize, mTextureUnit);
        }
        break;

        default:
        //case ResizeFlag_Keep:
        //case ResizeFlag_Stretch:
        {
            // TODO: 

        }
        break;
    }
}

void Texture::Bind(UINT type, UINT slot, bool UAV) {
    if( !mTextureUnit || !mTextureUnit->pSRV ) { return; }

    // Can't bind stagging texture
    if( CPURead && CPUWrite ) {
        printf_s("[Texture::Bind]: Can't bind staging texture! [%s]\n", mName.data());
        return;
    }

    if( type & Shader::Vertex   ) gDirectX->gContext->VSSetShaderResources(slot, 1, &mTextureUnit->pSRV);
    if( type & Shader::Pixel    ) gDirectX->gContext->PSSetShaderResources(slot, 1, &mTextureUnit->pSRV);
    if( type & Shader::Geometry ) gDirectX->gContext->GSSetShaderResources(slot, 1, &mTextureUnit->pSRV);
    if( type & Shader::Hull     ) gDirectX->gContext->HSSetShaderResources(slot, 1, &mTextureUnit->pSRV);
    if( type & Shader::Domain   ) gDirectX->gContext->DSSetShaderResources(slot, 1, &mTextureUnit->pSRV);
    if( type & Shader::Compute  ) {
        if( UAV ) {
            UINT pInitial = { 0 };
            gDirectX->gContext->CSSetUnorderedAccessViews(slot, 1, &mTextureUnit->pUAV, &pInitial);
        } else {
            gDirectX->gContext->CSSetShaderResources(slot, 1, &mTextureUnit->pSRV);
        }
    }
}

void Texture::Release() {
    if( mTextureUnit ) mTextureUnit->Release();

}

void implTexture::Release() {
    if( pUAV ) pUAV->Release();
    if( pSRV ) pSRV->Release();

    if( dim == 1 ) GetTexture1D()->Release();
    if( dim == 2 ) GetTexture2D()->Release();
    if( dim == 3 ) GetTexture3D()->Release();
}
