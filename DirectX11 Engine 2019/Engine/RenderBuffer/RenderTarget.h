#pragma once

#include "Engine/DirectX/DirectXChild.h"
#include <variant>

template<typename A, typename ...B> 
constexpr bool is_all_same() {
    return (typeid(A) == typeid(B));
}

// TODO: 
//  TextureCube
//  TextureCubeArray
//  

enum RenderTargetFlags {
    dim_1 = 1, dim_2 = 2, dim_3 = 4, 

    _Depth = 8, _Array = 16, _UAV = 32, 

    _MSAA = 64, _Cube = 128
};

struct implRenderTarget {
    // Target unit
    std::variant<ID3D11DepthStencilView*, ID3D11RenderTargetView*> pView;

    // Texture unit
    std::variant<ID3D11Texture1D*, ID3D11Texture2D*, ID3D11Texture3D*> pTexture;

    // Format unit
    std::variant<DXGI_FORMAT, UINT> mFormat;

    uint32_t mFlags;

    // Resource units
    ID3D11UnorderedAccessView *pUAV;
    ID3D11ShaderResourceView  *pSRV;

    void Release() {
        if( mFlags & dim_1 && std::get<ID3D11Texture1D*>(pTexture) ) { std::get<ID3D11Texture1D*>(pTexture)->Release(); }
        if( mFlags & dim_2 && std::get<ID3D11Texture2D*>(pTexture) ) { std::get<ID3D11Texture2D*>(pTexture)->Release(); }
        if( mFlags & dim_3 && std::get<ID3D11Texture3D*>(pTexture) ) { std::get<ID3D11Texture3D*>(pTexture)->Release(); }

        if( mFlags & _Depth ) {
            if( std::get<ID3D11DepthStencilView*>(pView) ) { std::get<ID3D11DepthStencilView*>(pView)->Release(); }
        } else {
            if( std::get<ID3D11RenderTargetView*>(pView) ) { std::get<ID3D11RenderTargetView*>(pView)->Release(); }
        }

        if( pSRV ) { pSRV->Release(); }
        if( pUAV ) { pUAV->Release(); }
    }
};

template<size_t dim, size_t BufferNum, bool DepthBuffer=false, size_t ArraySize=1, bool WillHaveMSAA=false, bool Cube=false>
class RenderTarget: public DirectXChild {
private:
    UINT mWidth, mHeight, mDepth;
    D3D11_VIEWPORT mViewPort;

    // MSAA
    // 0, 1=Depth, 2=Color0, ..., N=ColorN
    UINT mMSAASamples[2 * DepthBuffer + BufferNum];

    implRenderTarget *mRenderTargets[(UINT(dim == 2) * WillHaveMSAA + 1) * (BufferNum + DepthBuffer)];

    const char* mName;

    // Is MSAA enabled
    bool mMSAA = false;

    ID3D11Resource* Choose(ID3D11Texture1D* _1D, ID3D11Texture2D* _2D, ID3D11Texture3D* _3D) const {
        if( dim == 1 ) return _1D;
        if( dim == 2 ) return _2D;
        return _3D;
    }

    ID3D11Resource* Choose(std::variant<ID3D11Texture1D*, ID3D11Texture2D*, ID3D11Texture3D*> pTexture) const {
        if( dim == 1 ) return std::get<ID3D11Texture1D*>(pTexture);
        if( dim == 2 ) return std::get<ID3D11Texture2D*>(pTexture);
        return std::get<ID3D11Texture3D*>(pTexture);
    }

    void SetSize(UINT w, UINT h, UINT d = 1) {
        mViewPort.TopLeftX = 0.f;
        mViewPort.TopLeftY = 0.f;
        mViewPort.MinDepth = 0.f;
        mViewPort.MaxDepth = (dim == 3) ? (float)d : 1.f;
        mViewPort.Width = w;
        mViewPort.Height = h;

        mWidth = w;
        mHeight = h;
        mDepth = d;
    }


    // 
    implRenderTarget* CreateRenderTarget(bool Depth, std::variant<DXGI_FORMAT, UINT> format, 
                                         bool UAV=false, UINT index=2+WillHaveMSAA) {
        union {
            ID3D11DepthStencilView* _DSV;
            ID3D11RenderTargetView* _RTV;
        } pView{};

        union {
            ID3D11Texture1D* _1D;
            ID3D11Texture2D* _2D;
            ID3D11Texture3D* _3D;
        } pTexture{};

        ID3D11ShaderResourceView  *pSRV = 0;
        ID3D11UnorderedAccessView *pUAV = 0;

        DXGI_FORMAT formatTex, formatDSV, formatSRV;
        if( Depth ) {
            switch( std::get<UINT>(format) ) {
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
        } else {
            formatTex = std::get<DXGI_FORMAT>(format);
            formatSRV = formatTex;
        }

        // MSAA
        UINT SampleCount = 1, Quality = 0;
        //if( !UAV && dim == 2 ) MSAACheck(format, SampleCount, Quality);

        // Store sample count
        if( (Depth && index < 2) || !Depth ) mMSAASamples[Depth ? (UAV ? 1 : 0) : index] = SampleCount;

        // Try to create a texture
        UINT BindFlags = D3D11_BIND_SHADER_RESOURCE
                       | (UAV   ? D3D11_BIND_UNORDERED_ACCESS : 0)
                       | (Depth ? D3D11_BIND_DEPTH_STENCIL    : D3D11_BIND_RENDER_TARGET);
        HRESULT res = S_FALSE;
        if( dim == 1 ) {
            // Create Texture 1D
            D3D11_TEXTURE1D_DESC pTexDesc = {};
            pTexDesc.ArraySize          = ArraySize;
            pTexDesc.MipLevels          = 1;
            pTexDesc.BindFlags          = BindFlags;
            pTexDesc.Usage              = D3D11_USAGE_DEFAULT;
            pTexDesc.CPUAccessFlags     = 0;
            pTexDesc.MiscFlags          = 0;
            pTexDesc.Format             = formatTex;
            pTexDesc.Width              = mWidth;

            res = gDirectX->gDevice->CreateTexture1D(&pTexDesc, NULL, &pTexture._1D);
        } else if( dim == 2 ) {
            // Create Texture 2D
            D3D11_TEXTURE2D_DESC pTexDesc = {};
            pTexDesc.ArraySize          = ArraySize;
            pTexDesc.MipLevels          = 1;
            pTexDesc.BindFlags          = BindFlags;
            pTexDesc.Usage              = D3D11_USAGE_DEFAULT;
            pTexDesc.CPUAccessFlags     = 0;
            pTexDesc.MiscFlags          = 0;
            pTexDesc.Format             = formatTex;
            pTexDesc.Width              = mWidth;
            pTexDesc.Height             = mHeight;
            pTexDesc.SampleDesc.Count   = SampleCount;
            pTexDesc.SampleDesc.Quality = (UINT)std::max((int)Quality - 1, 0);

            res = gDirectX->gDevice->CreateTexture2D(&pTexDesc, NULL, &pTexture._2D);
        } else if( dim == 3 ) {
            // Create Texture 3D
            D3D11_TEXTURE3D_DESC pTexDesc = {};
            pTexDesc.MipLevels          = 1;
            pTexDesc.BindFlags          = BindFlags;
            pTexDesc.Usage              = D3D11_USAGE_DEFAULT;
            pTexDesc.CPUAccessFlags     = 0;
            pTexDesc.MiscFlags          = 0;
            pTexDesc.Format             = formatTex;
            pTexDesc.Width              = mWidth;
            pTexDesc.Height             = mHeight;
            pTexDesc.Depth              = mDepth;

            res = gDirectX->gDevice->CreateTexture3D(&pTexDesc, NULL, &pTexture._3D);
        }

        if( FAILED(res) ) {
            std::cout << "[RT]: Failed to create " << dim << "D texture" << std::endl;
        }

        // Create RTV/DSV
        res = S_FALSE;
        if( Depth ) {
            if( dim == 3 ) {
                std::cout << "[RT]: Can't create DSV for 3D texture" << std::endl;
            } else {
                if( !UAV ) {
                    // Create DSV
                    D3D11_DEPTH_STENCIL_VIEW_DESC pDSVDesc = {};
                    pDSVDesc.Flags  = 0;
                    pDSVDesc.Format = formatDSV;

                    if( dim == 1 ) {
                        pDSVDesc.Texture1D.MipSlice = 0;

                        if( ArraySize > 1 ) {
                            pDSVDesc.Texture1DArray.ArraySize       = ArraySize;
                            pDSVDesc.Texture1DArray.FirstArraySlice = 0;
                            pDSVDesc.Texture1DArray.MipSlice        = 0;
                        }
                    } else if( dim == 2 ) {
                        pDSVDesc.Texture2D.MipSlice = 0;

                        if( ArraySize > 1 ) {
                            if( Quality > 0 ) {
                                pDSVDesc.Texture2DMSArray.ArraySize       = ArraySize;
                                pDSVDesc.Texture2DMSArray.FirstArraySlice = 0;
                            } else {
                                pDSVDesc.Texture2DArray.ArraySize       = ArraySize;
                                pDSVDesc.Texture2DArray.FirstArraySlice = 0;
                                pDSVDesc.Texture2DArray.MipSlice        = 0;
                            }
                        }
                    }

                    pDSVDesc.ViewDimension = D3D11_DSV_DIMENSION(
                                              (dim == 1) ? (D3D11_DSV_DIMENSION_TEXTURE1D + (ArraySize > 1)                    ) :
                                              (dim == 2) ? (D3D11_DSV_DIMENSION_TEXTURE2D + (ArraySize > 1) + 2 * (Quality > 0)) : 
                                              D3D11_DSV_DIMENSION_UNKNOWN);

                    res = gDirectX->gDevice->CreateDepthStencilView(Choose(pTexture._1D, pTexture._2D, pTexture._3D), 
                                                                    &pDSVDesc, &pView._DSV);
                    if( FAILED(res) ) {
                        std::cout << "[RT]: Failed to create DSV" << std::endl;
                    }
                } else {
                    std::cout << "[RT]: Can't create DSV with UAV" << std::endl;
                }
            }
        } else {
            if( dim == 3 && ArraySize > 1 ) {
                std::cout << "[RT]: Can't create 3D RTV with ArraySize > 1" << std::endl;
            } else {
                // Create RTV
                D3D11_RENDER_TARGET_VIEW_DESC pRTVDesc = {};
                pRTVDesc.Format        = formatTex;
                pRTVDesc.ViewDimension = D3D11_RTV_DIMENSION(
                                         (dim == 1) ? (D3D11_RTV_DIMENSION_TEXTURE1D + (ArraySize > 1)                    ) : 
                                         (dim == 2) ? (D3D11_RTV_DIMENSION_TEXTURE2D + (ArraySize > 1) + 2 * (Quality > 0)) : 
                                         (dim == 3) ? (D3D11_RTV_DIMENSION_TEXTURE3D) : D3D11_RTV_DIMENSION_UNKNOWN);
                
                if( dim == 1 ) {
                    pRTVDesc.Texture1D.MipSlice = 0;

                    if( ArraySize > 1 ) {
                        pRTVDesc.Texture1DArray.FirstArraySlice = 0;
                        pRTVDesc.Texture1DArray.MipSlice        = 0;
                        pRTVDesc.Texture1DArray.ArraySize       = ArraySize;
                    }
                } else if( dim == 2 ) {
                    pRTVDesc.Texture2D.MipSlice = 0;

                    if( ArraySize > 1 ) {
                        if( Quality > 0 ) {
                            pRTVDesc.Texture2DMSArray.ArraySize       = ArraySize;
                            pRTVDesc.Texture2DMSArray.FirstArraySlice = 0;
                        } else {
                            pRTVDesc.Texture2DArray.FirstArraySlice = 0;
                            pRTVDesc.Texture2DArray.MipSlice        = 0;
                            pRTVDesc.Texture2DArray.ArraySize       = ArraySize;
                        }
                    }
                } else if( dim == 3 ) {
                    pRTVDesc.Texture3D.MipSlice    = 0;
                    pRTVDesc.Texture3D.FirstWSlice = 0;
                    pRTVDesc.Texture3D.WSize       = mDepth;
                }
                
                
                res = gDirectX->gDevice->CreateRenderTargetView(Choose(pTexture._1D, pTexture._2D, pTexture._3D), 
                                                                &pRTVDesc, &pView._RTV);
                if( FAILED(res) ) {
                    std::cout << "[RT]: Failed to create RTV" << std::endl;
                }
            }
        }

        // Create UAV
        if( UAV ) {
            if( dim == 3 && ArraySize > 1 ) {
                std::cout << "[RT]: Can't create 3D UAV with ArraySize > 1" << std::endl;
            } else {
                D3D11_UNORDERED_ACCESS_VIEW_DESC pUAVDesc = {};
                pUAVDesc.Format        = formatSRV;
                pUAVDesc.ViewDimension = (D3D11_UAV_DIMENSION)((D3D11_UAV_DIMENSION_TEXTURE1D + (ArraySize > 1) * (dim < 3)) * dim);
                
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
                    std::cout << "[RT]: Failed to create UAV" << std::endl;
                }
            }
        }
        
        // Create SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC pSRVDesc = {};
        pSRVDesc.Format        = formatSRV;
        pSRVDesc.ViewDimension = D3D11_SRV_DIMENSION(
            (dim == 1) ? (D3D11_SRV_DIMENSION_TEXTURE1D + (ArraySize > 1)) : 
            (dim == 2) ? (D3D11_SRV_DIMENSION_TEXTURE2D + (ArraySize > 1) + 2 * (Quality > 0)) :
            (dim == 3) ? D3D11_SRV_DIMENSION_TEXTURE3D : 
            D3D11_SRV_DIMENSION_UNKNOWN
        ); // Quality ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;

        if( dim == 1 ) {
            pSRVDesc.Texture1D.MipLevels       = 1;
            pSRVDesc.Texture1D.MostDetailedMip = 0;

            if( ArraySize > 1 ) {
                pSRVDesc.Texture1DArray.ArraySize       = ArraySize;
                pSRVDesc.Texture1DArray.FirstArraySlice = 0;
            }
        } else if( dim == 2  ) {
            pSRVDesc.Texture2D.MipLevels       = 1;
            pSRVDesc.Texture2D.MostDetailedMip = 0;

            if( ArraySize > 1 ) {
                if( Quality > 0 ) {
                    pSRVDesc.Texture2DMSArray.ArraySize       = ArraySize;
                    pSRVDesc.Texture2DMSArray.FirstArraySlice = 0;
                } else {
                    pSRVDesc.Texture2DArray.ArraySize       = ArraySize;
                    pSRVDesc.Texture2DArray.FirstArraySlice = 0;
                }
            }
        } else if( dim == 3 ) {
            pSRVDesc.Texture3D.MipLevels       = 1;
            pSRVDesc.Texture3D.MostDetailedMip = 0;
        }

        //pSRVDesc.TextureCube.MipLevels       = 1;
        //pSRVDesc.TextureCube.MostDetailedMip = 0;

        //pSRVDesc.TextureCubeArray.MipLevels        = 0;
        //pSRVDesc.TextureCubeArray.MostDetailedMip  = 0;
        //pSRVDesc.TextureCubeArray.First2DArrayFace = 0;
        //pSRVDesc.TextureCubeArray.NumCubes         = floor(ArraySize / 6);

        res = gDirectX->gDevice->CreateShaderResourceView(Choose(pTexture._1D, pTexture._2D, pTexture._3D), 
                                                          &pSRVDesc, &pSRV);
        if( FAILED(res) ) {
            std::cout << "[RT]: Failed to create SRV" << std::endl;
        }

        implRenderTarget* Out = new implRenderTarget();

        // Set view
            if( Depth ) Out->pView = pView._DSV;
            else        Out->pView = pView._RTV;

        // Set resource
                 if( dim == 1 ) Out->pTexture = pTexture._1D;
            else if( dim == 2 ) Out->pTexture = pTexture._2D;
            else                Out->pTexture = pTexture._3D;

            Out->mFormat  = format;
            Out->pSRV     = pSRV;
            Out->pUAV     = pUAV;
            Out->mFlags   = (1 << (dim - 1)) | (Depth ? _Depth : 0) | (UAV ? _UAV : 0) | 
                            (Quality ? _MSAA : 0) | (ArraySize > 1 ? _Array : 0);
        return Out;
    }

public:
    RenderTarget() = delete;

    RenderTarget(UINT w, UINT h, UINT d=1, const char* name="") {
        SetSize(w, h, d);

        mName = name;
    }

    void SetName(const char* name, implRenderTarget* rt) {
        if( rt ) _SetName(Choose(rt->pTexture), (std::string(mName) + std::string(name)).c_str());
    }

    // MSAA
    inline void DisableMSAA() { mMSAA = false; }
    inline void EnableMSAA()  {
        if( dim == 2 ) {
            if( WillHaveMSAA ) {
                mMSAA = true;
            } else std::cout << "[RT]: Can't enable MSAA on RenderTarget<" << dim << ", ...> with no MSAA support\n";
        } else std::cout << "[RT]: Can't enable MSAA on RenderTarget<" << dim << ", ...>\n";
    }

    // Main
    void Create(UINT bpp) {
        mRenderTargets[0] = CreateRenderTarget(true, bpp, false, 0);
        SetName(" D0", mRenderTargets[0]);

        if( mMSAA ) {
            // Disable MSAA to create non-MSAA DSV
            DisableMSAA();

            // Create non-MSAA RTV
            mRenderTargets[1] = CreateRenderTarget(true, bpp, false, 1); SetName(" D1 No MSAA, No UAV", mRenderTargets[1]);
            mRenderTargets[2] = CreateRenderTarget(true, bpp, true , 2); SetName(" D2 No MSAA, w/ UAV", mRenderTargets[2]);

            // Eban MSAA back again
            EnableMSAA();
        }
    }

    void Create(DXGI_FORMAT format, UINT slot=0, bool UAV=false) {
        UINT index = slot + (1 + 2 * WillHaveMSAA);

        mRenderTargets[index] = CreateRenderTarget(false, format, UAV, index);
        SetName(UAV ? " w/ UAV" : " No UAV", mRenderTargets[index]);

        if( mMSAA ) {
            // Disable MSAA to create non-MSAA RTV
            DisableMSAA();

            mRenderTargets[index + BufferNum] = CreateRenderTarget(false, format, UAV, BufferNum + index);
            SetName(UAV ? " w/ UAV No MSAA" : " No UAV No MSAA", mRenderTargets[BufferNum + index]);

            // Eban MSAA back again
            EnableMSAA();
        }
    }

    // slot=0
    template<typename ...FORMAT>
    void Create(UINT slot=0, FORMAT... formats) {
        if( typeid(DXGI_FORMAT) == typeid(FORMAT) ) 
            static_assert("[RT::Create(FORMAT... formats)]: Arguments must be DXGI_FORMAT.");

        std::array<DXGI_FORMAT, sizeof...(formats)> mFormats{ formats... };

        for( UINT i = 0; i < sizeof...(formats); i++ ) {
            Create(mFormats[i], slot + i);
        }
    }

    void Release() {
        for( UINT i = 0; i < (UINT(dim == 2) * WillHaveMSAA + 1) * (BufferNum + DepthBuffer); i++ ) 
            mRenderTargets[i]->Release();
    }

    void Resize(UINT w, UINT h);

    // Clear RTV
    void Clear(const FLOAT Color0[4]) {
        for( UINT i = 0; i < BufferNum * (WillHaveMSAA + 1); i++ ) {
            auto rt = std::get<ID3D11RenderTargetView*>(mRenderTargets[1 + 2 * WillHaveMSAA + i]->pView);
            if( rt ) gDirectX->gContext->ClearRenderTargetView(rt, Color0);
        }
    }

    // Clear DSV
    void Clear(FLOAT Depth, UINT8 Stencil, UINT flags=D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL) {
        for( UINT i = 0; i < DepthBuffer * (1 + 2 * WillHaveMSAA); i++ ) {
            auto rt = std::get<ID3D11DepthStencilView*>(mRenderTargets[i]->pView);
            if( rt ) gDirectX->gContext->ClearDepthStencilView(rt, flags, Depth, Stencil);
        }
    }

    // Bind RTVs and DSV
    void Bind() {
        ID3D11RenderTargetView *ptrRTV[BufferNum];
        for( size_t i = 0; i < BufferNum; i++ )
            ptrRTV[i] = std::get<ID3D11RenderTargetView*>(mRenderTargets[1 + 2 * WillHaveMSAA + i]->pView);

        gDirectX->gContext->OMSetRenderTargets(BufferNum, ptrRTV, DepthBuffer ? 
                                               std::get<ID3D11DepthStencilView*>(mRenderTargets[0]->pView) : nullptr);
    }

    template<UINT T>
    inline implRenderTarget* GetBuffer() const { return mRenderTargets[1 + 2 * WillHaveMSAA + T]; }

    // Bind
    void BindResource(sRenderBuffer* data, Shader::ShaderType type, UINT slot=0, bool UAV=false);

    // Get
    inline size_t GetWidth()  const { return mWidth;  }
    inline size_t GetHeight() const { return mHeight; }
    inline size_t GetDepth()  const { return (dim == 3) ? mDepth : 0; }
    inline UINT GetSampleNum(UINT i) const { return mMSAASamples[i];  }

};

// RenderTarget[dim]D_[num buffers]_[depth]_[array size]_[MSAA support]
using RenderTarget2D_1_0_1 = RenderTarget<2, 1, false, 1, false>;
using RenderTarget2D_2_0_1 = RenderTarget<2, 2, false, 1, false>;
using RenderTarget2D_3_0_1 = RenderTarget<2, 3, false, 1, false>;
using RenderTarget2D_4_0_1 = RenderTarget<2, 4, false, 1, false>;
using RenderTarget2D_5_0_1 = RenderTarget<2, 5, false, 1, false>;
using RenderTarget2D_6_0_1 = RenderTarget<2, 6, false, 1, false>;
using RenderTarget2D_7_0_1 = RenderTarget<2, 7, false, 1, false>;
using RenderTarget2D_8_0_1 = RenderTarget<2, 8, false, 1, false>;

using RenderTarget2D_1_1_1 = RenderTarget<2, 1, true, 1, false>;
using RenderTarget2D_2_1_1 = RenderTarget<2, 2, true, 1, false>;
using RenderTarget2D_3_1_1 = RenderTarget<2, 3, true, 1, false>;
using RenderTarget2D_4_1_1 = RenderTarget<2, 4, true, 1, false>;
using RenderTarget2D_5_1_1 = RenderTarget<2, 5, true, 1, false>;
using RenderTarget2D_6_1_1 = RenderTarget<2, 6, true, 1, false>;
using RenderTarget2D_7_1_1 = RenderTarget<2, 7, true, 1, false>;
using RenderTarget2D_8_1_1 = RenderTarget<2, 8, true, 1, false>;

using RenderTarget2D_0_1_1 = RenderTarget<2, 0, true, 1, false>;

// 
using RenderTarget2DColor1 = RenderTarget<2, 1, false, 1, false>;
using RenderTarget2DColor2 = RenderTarget<2, 2, false, 1, false>;
using RenderTarget2DColor3 = RenderTarget<2, 3, false, 1, false>;
using RenderTarget2DColor4 = RenderTarget<2, 4, false, 1, false>;
using RenderTarget2DColor5 = RenderTarget<2, 5, false, 1, false>;
using RenderTarget2DColor6 = RenderTarget<2, 6, false, 1, false>;
using RenderTarget2DColor7 = RenderTarget<2, 7, false, 1, false>;
using RenderTarget2DColor8 = RenderTarget<2, 8, false, 1, false>;

using RenderTarget2DColor1Depth = RenderTarget<2, 1, true, 1, false>;
using RenderTarget2DColor2Depth = RenderTarget<2, 2, true, 1, false>;
using RenderTarget2DColor3Depth = RenderTarget<2, 3, true, 1, false>;
using RenderTarget2DColor4Depth = RenderTarget<2, 4, true, 1, false>;
using RenderTarget2DColor5Depth = RenderTarget<2, 5, true, 1, false>;
using RenderTarget2DColor6Depth = RenderTarget<2, 6, true, 1, false>;
using RenderTarget2DColor7Depth = RenderTarget<2, 7, true, 1, false>;
using RenderTarget2DColor8Depth = RenderTarget<2, 8, true, 1, false>;

using RenderTarget2DDepth = RenderTarget<2, 0, true, 1, false>;

// With MSAA
using RenderTarget2DDepthMSAA = RenderTarget<2, 0, true, 1, true>;

