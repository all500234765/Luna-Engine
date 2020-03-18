#include "pc.h"
#include "Texture.h"

Shader*         Texture::shTransformRTV{};
Shader*         Texture::shTransformUAV{};
ConstantBuffer* Texture::cbTransform{};
ConstantBuffer* Texture::cbCamera{};
Sampler*        Texture::gLinearSampler{};
Sampler*        Texture::gPointSampler{};

BlendState*        Texture::bsSimple{};
RasterState*       Texture::rsSimple{};
DepthStencilState* Texture::dsSimple{};

implTexture* Texture::CreateTexture(DXGI_FORMAT format, D3D11_SUBRESOURCE_DATA *SubResource, 
                                     uint32_t ArraySize, implTexture *Out) const {
    union local_texture {
        ID3D11Texture1D* _1D;
        ID3D11Texture2D* _2D;
        ID3D11Texture3D* _3D;
    } pTexture{};

    ID3D11ShaderResourceView  *pSRV = 0;
    ID3D11UnorderedAccessView *pUAV = 0;
    ID3D11RenderTargetView    *pRTV = 0;

    // MSAA
    uint32_t Quality = 0;
    uint32_t SampleCount = 1;

    // 
    HRESULT res = S_FALSE;
    D3D11_USAGE Usage = (CPURead ? D3D11_USAGE_STAGING : 
                            (CPUWrite ? D3D11_USAGE_DYNAMIC : 
                                (Immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT)));
    UINT BindFlags = D3D11_BIND_SHADER_RESOURCE | 
                     (HasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0)
                   | (((Usage != D3D11_USAGE_STAGING && Usage != D3D11_USAGE_IMMUTABLE) && 
                        !MSAA && !FormatBC(format)) ? D3D11_BIND_RENDER_TARGET : 0);
    UINT MipMaps      = MSAA ? 1 : (HasMipMaps ? ((mMipMaps == 1) ? 0 : mMipMaps) : 1);
    UINT CPUAccess    = (CPURead    ? D3D11_CPU_ACCESS_READ : 0) | (CPUWrite ? D3D11_CPU_ACCESS_WRITE : 0);
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

        // Create RTV
        if( BindFlags & D3D11_BIND_RENDER_TARGET ) {
            D3D11_RENDER_TARGET_VIEW_DESC desc{};
            desc.Format = format;
            desc.ViewDimension = D3D11_RTV_DIMENSION(
                (dim == 1) ? (D3D11_RTV_DIMENSION_TEXTURE1D + (mArraySize > 1)) :
                (dim == 2) ? (D3D11_RTV_DIMENSION_TEXTURE2D + (mArraySize > 1)) :
                (dim == 3) ? D3D11_RTV_DIMENSION_TEXTURE3D :
                D3D11_RTV_DIMENSION_UNKNOWN
            );

            if( dim == 1 ) {
                desc.Texture1D.MipSlice = 0;

                if( mArraySize > 1 ) {
                    desc.Texture1DArray.FirstArraySlice = 0;
                    desc.Texture1DArray.ArraySize = mArraySize;
                    desc.Texture1DArray.MipSlice = 0;
                }
            } else if( dim == 2 ) {
                desc.Texture2D.MipSlice = 0;

                if( mArraySize > 1 ) {
                    desc.Texture2DArray.MipSlice = 0;
                    desc.Texture2DArray.ArraySize = mArraySize;
                    desc.Texture2DArray.FirstArraySlice = 0;
                }
            } else if( dim == 3 ) {
                desc.Texture3D.FirstWSlice = 0;
                desc.Texture3D.MipSlice = 0;
                desc.Texture3D.WSize = GetDepth();
            }

            // 
            res = gDirectX->gDevice->CreateRenderTargetView(Choose(pTexture._1D, pTexture._2D, pTexture._3D), &desc, &pRTV);
            if( FAILED(res) ) {
                printf_s("[Texture::Create]: Failed to create RTV! [%s]\n", mName.data());
            }
        }
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
    Out->pRTV     = pRTV;
    Out->mFlags   = mFlags;

    return Out;
}

void Texture::GlobalInit() {
    // Buffers
    cbTransform = new ConstantBuffer();
    cbTransform->CreateDefault(sizeof(TransformBuffer));

    cbCamera = new ConstantBuffer();
    cbCamera->CreateDefault(sizeof(CameraBuffer));

    // Shaders
    shTransformRTV = new Shader();
    shTransformRTV->LoadFile("shTexturedQuadAutoVS.cso", Shader::Vertex);
    shTransformRTV->LoadFile("shTexturedQuadPS.cso", Shader::Pixel);

    shTransformRTV->ReleaseBlobs();

    // Samplers
    gLinearSampler = new Sampler();
    gPointSampler  = new Sampler();

    {
        D3D11_SAMPLER_DESC pDesc{};
        pDesc.Filter         = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        pDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        pDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        pDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        pDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        pDesc.MaxLOD         = D3D11_FLOAT32_MAX;
        pDesc.MinLOD         = 0;
        pDesc.MipLODBias     = 0;
        pDesc.MaxAnisotropy  = 16;

        // Point sampler
        gPointSampler->Create(pDesc);

        // Linear sampler
        pDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        gLinearSampler->Create(pDesc);
    }

    // Raster
    rsSimple = new RasterState();

    {
        D3D11_RASTERIZER_DESC pDesc;
        ZeroMemory(&pDesc, sizeof(D3D11_RASTERIZER_DESC));
        pDesc.AntialiasedLineEnable = true;
        pDesc.CullMode              = D3D11_CULL_NONE;
        pDesc.DepthBias             = 0;
        pDesc.DepthBiasClamp        = 0.0f;
        pDesc.DepthClipEnable       = false;
        pDesc.FillMode              = D3D11_FILL_SOLID;
        pDesc.FrontCounterClockwise = false;
        pDesc.MultisampleEnable     = false;
        pDesc.ScissorEnable         = false;
        pDesc.SlopeScaledDepthBias  = 0.0f;

        // Normal
        rsSimple->Create(pDesc);
    }

    // Blend
    bsSimple = new BlendState();

    {
        D3D11_BLEND_DESC pDesc;
        pDesc.RenderTarget[0].BlendEnable           = true;
        pDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;

        pDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        
        pDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_SRC_ALPHA;
        
        pDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        pDesc.AlphaToCoverageEnable  = false;
        pDesc.IndependentBlendEnable = false;

        bsSimple->Create(pDesc, { 1.f, 1.f, 1.f, 1.f });
    }

    // Depth Stencil
    dsSimple = new DepthStencilState();

    {
        D3D11_DEPTH_STENCIL_DESC pDesc;
        pDesc.DepthEnable = true;
        pDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        pDesc.DepthFunc = D3D11_COMPARISON_GREATER; //D3D11_COMPARISON_LESS

        pDesc.StencilEnable = !true;
        pDesc.StencilReadMask = 0xFF;
        pDesc.StencilWriteMask = 0xFF;

        // Stencil operations if pixel is front-facing.
        pDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        pDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        pDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        pDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        // Stencil operations if pixel is back-facing.
        pDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        pDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        pDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        pDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        // No RW
        pDesc.DepthEnable = false;
        pDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsSimple->Create(pDesc, 0);
    }
}

void Texture::GlobalRelease() {
    // Samplers
    SAFE_RELEASE(gPointSampler);

    // Shaders
    SAFE_RELEASE(shTransformUAV);
    SAFE_RELEASE(shTransformRTV);

    // Buffers
    SAFE_RELEASE(cbTransform);
    SAFE_RELEASE(cbCamera);

    // States
    SAFE_RELEASE(bsSimple);
    SAFE_RELEASE(rsSimple);
    SAFE_RELEASE(dsSimple);
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
    SetName(name.data());
}

Texture::Texture(std::string_view fname, UINT flags, std::string_view name, 
                   uint32_t ArraySize, DXGI_FORMAT custom_format): bUndefined(false) {
    std::string nm = ( (name == "UnnamedTexture") ? file_name(fname).data() : name.data() );
    SetName(nm);
    Load(fname, flags, ArraySize, custom_format);
    //SetName(mName.data());
}

void Texture::Load(std::string_view fname, UINT flags, uint32_t ArraySize, DXGI_FORMAT custom_format) {
    SAFE_RELEASE(mTextureUnit);

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

void Texture::Copy(Texture* src, uint32_t dst_x, uint32_t dst_y, uint32_t dst_z, uint32_t dst_array, uint32_t dst_mip,
                   uint32_t src_mip, uint32_t src_array) {
    D3D11_BOX box{};
    box.left = 0;
    box.right = mWidth >> src_mip;

    box.top = 0;
    box.bottom = mHeight >> src_mip;

    box.front = 0;
    box.back = mDepth >> src_mip;

    // Overflow test
    if( box.bottom == 0 || (box.right == 0 && (dim >= 2)) || (box.back == 0 && (dim == 3)) ) {
    
    } else {
        box.right = std::max(box.right, 1u); // 1D
        box.back = std::max(box.back, 1u); // 1D, 2D
    }

    uint32_t dst_sub = D3D11CalcSubresource(dst_mip, dst_array, mMipMaps);
    uint32_t src_sub = D3D11CalcSubresource(src_mip, src_array, src->mMipMaps);

    gDirectX->gContext->CopySubresourceRegion(GetResource(), dst_sub, dst_x, dst_y, dst_z, 
                                              src->GetResource(), src_sub, &box);
}

void Texture::CopyS(Texture* dest, Texture* src, uint32_t dst_x, uint32_t dst_y, uint32_t dst_z, 
                    uint32_t dst_array, uint32_t dst_mip, uint32_t src_mip, uint32_t src_array) {
    D3D11_BOX box{};
    box.left = 0;
    box.right = dest->GetWidth() >> src_mip;

    box.top = 0;
    box.bottom = dest->GetHeight() >> src_mip;

    box.front = 0;
    box.back = dest->GetDepth() >> src_mip;

    // Overflow test
    if( box.bottom == 0 || (box.right == 0 && (dest->dim >= 2)) || (box.back == 0 && (dest->dim == 3)) ) {

    } else {
        box.right = std::max(box.right, 1u); // 1D
        box.back = std::max(box.back, 1u); // 1D, 2D
    }

    uint32_t dst_sub = D3D11CalcSubresource(dst_mip, dst_array, dest->mMipMaps);
    uint32_t src_sub = D3D11CalcSubresource(src_mip, src_array, src->mMipMaps);

    gDirectX->gContext->CopySubresourceRegion(dest->GetResource(), dst_sub, dst_x, dst_y, dst_z,
                                              src->GetResource(), src_sub, &box);
}

void Texture::CopyS(implTexture* dest, Texture* src, uint32_t dst_x, uint32_t dst_y, uint32_t dst_z,
                    uint32_t dst_array, uint32_t dst_mip, uint32_t src_mip, uint32_t src_array) {
    if( !ChooseS(dest) || !src->GetTexture() ) return;

    D3D11_BOX box{};
    box.left = 0;
    box.right = src->GetWidth() >> src_mip;

    box.top = 0;
    box.bottom = src->GetHeight() >> src_mip;

    box.front = 0;
    box.back = src->GetDepth() >> src_mip;

    // Overflow test
    if( box.bottom == 0 || (box.right == 0 && (dest->dim >= 2)) || (box.back == 0 && (dest->dim == 3)) ) {

    } else {
        box.right = std::max(box.right, 1u); // 1D
        box.back = std::max(box.back, 1u); // 1D, 2D
    }

    uint32_t dst_sub = D3D11CalcSubresource(dst_mip, dst_array, dest->HasMipMaps * 15);
    uint32_t src_sub = D3D11CalcSubresource(src_mip, src_array, src->mMipMaps);

    gDirectX->gContext->CopySubresourceRegion(ChooseS(dest), dst_sub, dst_x, dst_y, dst_z,
                                              src->GetResource(), src_sub, &box);
}

void Texture::CopySRot(implTexture* dest, Texture* src, float x, float y, float angle, float total_width, float total_height) {
    if( dest->pRTV ) {
        gDirectX->gContext->OMSetRenderTargets(1, &dest->pRTV, nullptr);
        shTransformRTV->Bind();

        // Store states
        BlendState::Push();
        RasterState::Push();
        STopologyState::Push();
        DepthStencilState::Push();

        // Bind states
        bsSimple->Bind();
        rsSimple->Bind();
        dsSimple->Bind();
        STopologyState::Bind(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        D3D11_VIEWPORT vp;
        vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f;
        vp.Width  = total_width;
        vp.Height = total_height;
        vp.TopLeftX = 0.f;
        vp.TopLeftY = 0.f;

        gDirectX->gContext->RSSetViewports(1, &vp);

        {
            ScopeMapConstantBuffer<TransformBuffer> c(cbTransform);

            float rad = DirectX::XMConvertToRadians(angle + 0*90.f);
            float sc = sinf(rad), cc = cosf(rad);
            float W = src->GetWidth();
            float H = src->GetHeight();

            c.data->mWorld = DirectX::XMMatrixRotationZ(rad); // [-1; 1]
            c.data->mWorld *= DirectX::XMMatrixScaling(.5f, .5f, 1.f);
            c.data->mWorld *= DirectX::XMMatrixTranslation(.5f, .5f, 0.f); // [0; 1]

            c.data->mWorld *= DirectX::XMMatrixScaling(W, H, 1.f);
            c.data->mWorld *= DirectX::XMMatrixTranslation(x + 0*W * .25f, y + 0*H * .5f, 0.f);

            c.data->vPosition = {};
            c.data->vRotation = {};
            c.data->vScale    = {};
        }

        {
            ScopeMapConstantBuffer<CameraBuffer> c(cbCamera);

            c.data->mView = DirectX::XMMatrixIdentity();
            c.data->mProj = DirectX::XMMatrixOrthographicOffCenterLH(0.f, total_width, total_height, 0.f, .1f, 10.f);
        }

        cbTransform->Bind(Shader::Vertex, 0u); // CB
        cbCamera->Bind(Shader::Vertex, 1u);    // CB
        src->Bind(Shader::Pixel, 0u, false);   // SRV
        gPointSampler->Bind(Shader::Pixel, 0); // Sampler

        // Draw call
        DXDraw(6, 0);

        // Restore states
        BlendState::Pop();
        RasterState::Pop();
        STopologyState::Pop();
        DepthStencilState::Pop();
    } else if( dest->pUAV ) {

    }
}

void Texture::ClearStg(float4 color) {
    implTexture* stg = CreateStaging(color);
    Copy(stg);
    SAFE_RELEASE(stg);
}

void Texture::ClearStg(uint4 color) {
    implTexture* stg = CreateStaging(color);
    Copy(stg);
    SAFE_RELEASE(stg);
}

void Texture::ClearRtv(const float color[4]) const {
    // Clear
    gDirectX->gContext->ClearRenderTargetView(mTextureUnit->pRTV, color);
}

implTexture* Texture::CreateStaging() const {
    auto tex = CreateTexture(mTextureUnit->mFormat, nullptr, mArraySize);
    std::string str = std::string("stg.") + mName.data();
    _SetName(Choose(tex->pTexture), str.c_str());

    return tex;
}

implTexture* Texture::CreateStaging(float4 color) {
    // Remove mipmapping for staging texture
    uint mpsc = mMipMaps;
    mMipMaps = 0;

    // Load subresource data
    D3D11_SUBRESOURCE_DATA* sub = new D3D11_SUBRESOURCE_DATA[mArraySize];
    size_t index = 0;
    for( int i = 0; i < mArraySize; i++ ) {
        // Set data
        sub[index].pSysMem = malloc(GetWHDCN());
        sub[index].SysMemPitch = GetLineLength();
        sub[index].SysMemSlicePitch = GetSliceLength();

        memcpy_s(&sub[index].pSysMem, GetWHDCN(), &color, GetChannelNum() * sizeof(float));
        index++;
    }

    implTexture* tex = CreateTexture(mTextureUnit->mFormat, sub, mArraySize);
    mMipMaps = mpsc; // Restore mips
    _SetName(Choose(tex->pTexture), (std::string("stg.") + std::string(mName)).c_str());

    delete[] sub;
    return tex;
}

implTexture* Texture::CreateStaging(uint4 color) {
    // Remove mipmapping for staging texture
    uint mpsc = mMipMaps;
    mMipMaps = 0;

    // Load subresource data
    D3D11_SUBRESOURCE_DATA* sub = new D3D11_SUBRESOURCE_DATA[mArraySize];
    size_t index = 0;
    for( int i = 0; i < mArraySize; i++ ) {
        // Set data
        sub[index].pSysMem     = malloc(GetWHDCN());
        sub[index].SysMemPitch = GetLineLength();
        sub[index].SysMemSlicePitch = GetSliceLength();
        
        memcpy_s(&sub[index].pSysMem, GetWHDCN(), &color, GetChannelNum() * sizeof(uint));
        index++;
    }
    
    implTexture* tex = CreateTexture(mTextureUnit->mFormat, sub, mArraySize);
    mMipMaps = mpsc; // Restore mips
    _SetName(Choose(tex->pTexture), (std::string("stg.") + std::string(mName)).c_str());

    delete[] sub;
    return tex;
}

Texture* Texture::CreateStaging(uint32_t flags, DXGI_FORMAT format, uint32_t width,
                                uint32_t height, uint32_t depth, uint32_t array_size) {
    flags |= tf_CPURead | tf_CPUWrite;
    return new Texture(flags, format, width, height, depth, array_size, "stg");
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
    SAFE_RELEASE(mTextureUnit);
}

void implTexture::Release() {
    if( pUAV ) pUAV->Release();
    if( pSRV ) pSRV->Release();
    if( pRTV ) pRTV->Release();

    if( dim == 1 ) GetTexture1D()->Release();
    if( dim == 2 ) GetTexture2D()->Release();
    if( dim == 3 ) GetTexture3D()->Release();
}
