#pragma once

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine Includes/Types.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/States/PipelineState.h"
#include "HighLevel/DirectX/Utlities.h"

#include <variant>
#include <algorithm>
#include <array>

// TODO: 
//   TextureCube
//   TextureCubeArray
// * Fix MSAA only 1 sample
//  

extern _DirectX *gDirectX;

enum RenderTargetFlags {
    dim_1 = 1, dim_2 = 2, dim_3 = 4, 

    _Depth = 8, _Array = 16, _UAV = 32, 

    _MSAA = 64, _Cube = 128
};

struct implRenderTarget {
    // Target unit
    std::variant<bool, ID3D11DepthStencilView*, ID3D11RenderTargetView*> pView;

    // Texture unit
    std::variant<bool, ID3D11Texture1D*, ID3D11Texture2D*, ID3D11Texture3D*> pTexture;

    // Format unit
    std::variant<DXGI_FORMAT, UINT> mFormat;

    uint32_t mFlags;

    // Resource units
    ID3D11UnorderedAccessView *pUAV;
    ID3D11ShaderResourceView  *pSRV;

    void Release();
};

class RenderTargetMSAA: public DirectXChild {
protected:
    struct _MSAA_DepthResolve {
        uint2 _Dim;       // Depth buffer dimensions
        uint1 _Samples;   // MSAA Sample count
        uint1 _Alignment; // Empty
    };

    // MSAA Resolve resources
    static Shader*         g_shMSAADepthResolve;
    static ConstantBuffer* g_MSAAConstantBuffer;
    //static Texture*        g_MSAATextureUAV;

public:
    static void GlobalInit();

    static void GlobalRelease();

};

/*class IRenderTarget: public PipelineState<void*> {
private:
    //     0-1  2-4
    /*size_t dim, BufferNum;
    //   5                    6                     7
    bool DepthBuffer = false, WillHaveMSAA = false, Cube = false;
    size_t ArraySize = 1; // 8-263*
    /

    static uint32_t gPacked;

public:
    static auto Current() {
        uint8_t dim = (gPacked & 0x03);
        uint8_t num = (gPacked & 0x1C) >> 2;
        bool    dba = (gPacked & 0x20) >> 5;
        bool    haa = (gPacked & 0x40) >> 6;
        bool    cub = (gPacked & 0x80) >> 7;
        uint8_t asz = (gPacked & 0xFF) >> 8;

#define RENDER_TARGET(DIM, NUM, DBA, HAA, CUB, ASZ) return (RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube>*)gState;

        switch( dim ) {
            case 1:
                if( dba && haa && !cub ) {
                    switch( num ) {
                        case 1:
                            switch( asz ) {
                                case 1:
                                    RENDER_TARGET(1, 1, 1, 1, 0, 1);
                            }
                            break;
                    }
                }
                break;
        }
        
#undef RENDER_TARGET
        return nullptr;
        //return (RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube>*)gState;
    }



};*/

template<size_t dim, size_t BufferNum, bool DepthBuffer=false, 
         size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps 
                                                     to create per RT buffer   */
         bool WillHaveMSAA=false, bool Cube=false>
class RenderTarget: public RenderTargetMSAA {
private:
    UINT mWidth, mHeight, mDepth;
    D3D11_VIEWPORT mViewPort;

    // Offset with respect to DepthBuffers count
    size_t mOffset = (DepthBuffer ? (1 + (WillHaveMSAA ? 2 : 0)) : 0);

    // MSAA
    // 0, 1=Depth, 2=Color0, ..., N=ColorN
    std::array<UINT, 2 * DepthBuffer + BufferNum> mMSAASamples;

    std::array<implRenderTarget*, 
               (DepthBuffer ? (1 + (WillHaveMSAA ? 2 : 0)) : 0) + 
                              (1 + (WillHaveMSAA ? 1 : 0)) * BufferNum> mRenderTargets;

    // Max MSAA level for this RT
    UINT mMSAAMaxLevel = 8;

    // RT Base Name
    const char* mName;

    // Is MSAA enabled
    bool mMSAA = false;

    // MSAA
    void MSAACheck(DXGI_FORMAT format, UINT& SampleCount, UINT& Quality) {
        if( mMSAA ) {
            // Check for MSAA Support
            UINT uQuality = 0;
            UINT Samples = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;

            // Find max number of samples and quality level
            for( ; Samples > 1; Samples /= 2 ) {
                gDirectX->gDevice->CheckMultisampleQualityLevels(format, Samples, &uQuality);
                if( uQuality > 0 ) { break; }
            }

            // Update
            if( uQuality > 0 ) {
                SampleCount = Samples;
                Quality     = uQuality;

                std::cout << "Found MSAA Quality and Sample Count combination (Samples="
                          << SampleCount << "; Quality=" << Quality << ")" << std::endl;
            } else {
                std::cout << "Failed to find MSAA Quality and Sample Count combination" << std::endl;
            }
        }
    }
    
    template<size_t dim, size_t BufferNum, bool DepthBuffer=false, 
             size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps 
                                                         to create per RT buffer   */
             bool WillHaveMSAA=false, bool Cube=false>
    static void MSAAResolveDepth(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube>* RT) {
        if constexpr( !DepthBuffer ) return;
        ScopedRangeProfiler s1(L"MSAA Resolve depth buffer");

        float Width  = (float)RT->GetWidth();
        float Height = (float)RT->GetHeight();

        // Resolve Depth Buffer MSAA
        // Update constant buffer
        _MSAA_DepthResolve* BuffInst = (_MSAA_DepthResolve*)g_MSAAConstantBuffer->Map();
            BuffInst->_Dim     = { (UINT)Width, (UINT)Height };
            BuffInst->_Samples = RT->GetSampleNum(1);
        g_MSAAConstantBuffer->Unmap();

        // Calculate dispatch dimensions
        UINT X = (UINT)ceil(Width  / 16.f);
        UINT Y = (UINT)ceil(Height / 16.f);

        // Bind resourses
        implRenderTarget* SRVOrig  = RT->GetDepthBuffer<0, false>();
        implRenderTarget* SRVNoUAV = RT->GetDepthBuffer<1, false>();
        implRenderTarget* UAV      = RT->GetDepthBuffer<1>();

        RT->Bind(UAV, Shader::Compute, 0, true);         // UAV
        RT->Bind(SRVOrig, Shader::Compute, 0);           // SRV
        g_MSAAConstantBuffer->Bind(Shader::Compute, 0);  // CB

        // Resolve MSAA
        g_shMSAADepthResolve->Dispatch(X, Y, 1);

        // Discard resources
        {
            ID3D11UnorderedAccessView *pEmpty = nullptr;
            gDirectX->gContext->CSSetUnorderedAccessViews(0, 1, &pEmpty, 0);
        }

        {
            ID3D11Buffer *pEmpty = nullptr;
            gDirectX->gContext->CSSetConstantBuffers(0, 1, &pEmpty);
        }

        {
            ID3D11ShaderResourceView *pEmpty = nullptr;
            gDirectX->gContext->CSSetShaderResources(0, 1, &pEmpty);
        }

        // Copy resources from UAV db to DSV db (depth buffer)
        gDirectX->gContext->CopyResource(ChooseS((UINT)RT->GetDim(), SRVNoUAV->pTexture),
                                         ChooseS((UINT)RT->GetDim(), UAV->pTexture));
    }

    // Choose texture
    ID3D11Resource* Choose(ID3D11Texture1D* _1D, ID3D11Texture2D* _2D, ID3D11Texture3D* _3D) const {
        if( dim == 1 ) return _1D;
        if( dim == 2 ) return _2D;
        return _3D;
    }

    ID3D11Resource* Choose(std::variant<bool, ID3D11Texture1D*, ID3D11Texture2D*, ID3D11Texture3D*> pTexture) const {
        if( dim == 1 ) return std::get<ID3D11Texture1D*>(pTexture);
        if( dim == 2 ) return std::get<ID3D11Texture2D*>(pTexture);
        return std::get<ID3D11Texture3D*>(pTexture);
    }

    static ID3D11Resource* ChooseS(UINT DIM, std::variant<bool, ID3D11Texture1D*, ID3D11Texture2D*, ID3D11Texture3D*> pTexture) {
        if( DIM == 1 ) return std::get<ID3D11Texture1D*>(pTexture);
        if( DIM == 2 ) return std::get<ID3D11Texture2D*>(pTexture);
        return std::get<ID3D11Texture3D*>(pTexture);
    }

    void SetSize(UINT w, UINT h, UINT d=1) {
        mViewPort.TopLeftX = 0.f;
        mViewPort.TopLeftY = 0.f;
        mViewPort.MinDepth = 0.f;
        mViewPort.MaxDepth = (dim == 3) ? (FLOAT)d : 1.f;
        mViewPort.Width  = (FLOAT)w;
        mViewPort.Height = (FLOAT)h;

        mWidth  = w;
        mHeight = h;
        mDepth  = d;
    }

    // 
    implRenderTarget* CreateRenderTarget(bool Depth, std::variant<DXGI_FORMAT, UINT> format, 
                                         bool UAV=false, UINT index=mOffset, implRenderTarget* Out=nullptr) {
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
        if( !UAV && dim == 2 ) {
            MSAACheck(formatTex, SampleCount, Quality);

            // Limit amount of samples depending on current setting
            SampleCount = std::min(mMSAAMaxLevel, SampleCount);
        }

        // Store sample count
        if( mMSAA && ((Depth && index < 2) || !Depth) ) {
            mMSAASamples[Depth ? (UAV ? 1 : 0) : (index - mOffset + (mOffset > 0))] = SampleCount;
        }

        // Try to create a texture
        UINT BindFlags = D3D11_BIND_SHADER_RESOURCE
                       | (UAV   ? D3D11_BIND_UNORDERED_ACCESS : 0)
                       | (Depth ? (UAV ? 0 : D3D11_BIND_DEPTH_STENCIL) : D3D11_BIND_RENDER_TARGET);
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

        if( Out == nullptr ) Out = new implRenderTarget();

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

    RenderTarget(UINT w, UINT h, UINT d=1, const char* name="UnnamedRT") {
        SetSize(w, h, d);

        mMSAAMaxLevel = 8;
        mName = name;
    }

    // Setters
    void SetName(const char* name, implRenderTarget* rt) {
        if( rt ) _SetName(Choose(rt->pTexture), (std::string(mName) + std::string(name)).c_str());
    }

    // [1 - 32]; Power of 2
    inline void SetMSAAMaxLevel(UINT level) {
        mMSAAMaxLevel = std::clamp(level, 1u, 32u);
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

    void MSAAResolve() {
        if( mMSAA ) {
            ScopedRangeProfiler s1(L"MSAA Resolve");

            // Unbind views
            ID3D11RenderTargetView* nullRTV = nullptr;
            gDirectX->gContext->OMSetRenderTargets(1, &nullRTV, nullptr);

            // Resolve color buffers
            for( size_t i = mOffset; i < mOffset + BufferNum; i++ ) {
                gDirectX->gContext->ResolveSubresource(Choose(mRenderTargets[i + BufferNum]->pTexture), 0, 
                                                       Choose(mRenderTargets[i + 0        ]->pTexture), 0, 
                                                       std::get<DXGI_FORMAT>(mRenderTargets[i]->mFormat));
            }

            // Resolve depth buffer
            MSAAResolveDepth(this);
        }
    }

    // Main
    void Create(UINT bpp, bool rct=false) {
        if constexpr( !DepthBuffer ) return;
        mRenderTargets[0] = CreateRenderTarget(true, bpp, false, 0, rct ? mRenderTargets[0] : nullptr);
        SetName(" D0", mRenderTargets[0]);

        if( mMSAA ) {
            // Disable MSAA to create non-MSAA DSV
            DisableMSAA();

            // Create non-MSAA RTV
            mRenderTargets[1] = CreateRenderTarget(true, bpp, false, 1, rct ? mRenderTargets[1] : nullptr);
            SetName(" D1 No MSAA, No UAV", mRenderTargets[1]);

            mRenderTargets[2] = CreateRenderTarget(true, bpp, true , 2, rct ? mRenderTargets[2] : nullptr);
            SetName(" D2 No MSAA, w/ UAV", mRenderTargets[2]);

            // Eban MSAA back again
            EnableMSAA();
        }
    }

    void Create(DXGI_FORMAT format, UINT slot=0, bool UAV=false, bool rct=false) {
        if constexpr( !BufferNum ) return;
        UINT index = UINT(slot + mOffset);

        mRenderTargets[index] = CreateRenderTarget(false, format, UAV, index, rct ? mRenderTargets[index] : nullptr);
        SetName(UAV ? " w/ UAV" : " No UAV", mRenderTargets[index]);

        if( mMSAA ) {
            // Disable MSAA to create non-MSAA RTV
            DisableMSAA();

            mRenderTargets[index + BufferNum] = CreateRenderTarget(false, format, UAV, index, 
                                                                   rct ? mRenderTargets[index + BufferNum] : nullptr);
            SetName(UAV ? " w/ UAV No MSAA" : " No UAV No MSAA", mRenderTargets[BufferNum + index]);

            // Eban MSAA back again
            EnableMSAA();
        }
    }

    // slot=0
    template<typename ...FORMAT>
    void CreateList(UINT slot=0, FORMAT... formats) {
        if constexpr( !BufferNum ) return;
        //if( typeid(DXGI_FORMAT) == typeid(FORMAT) ) 
        //    static_assert("[RT::Create(UINT slot=0, FORMAT... formats)]: Arguments must be DXGI_FORMAT.");

        std::array<DXGI_FORMAT, sizeof...(formats)> mFormats{ formats... };

        for( UINT i = 0; i < LunaEngine::Math::min(sizeof...(formats), BufferNum); i++ ) {
            Create(mFormats[i], slot + i);
        }
    }

    void Release() {
        for( UINT i = 0; i < (UINT(dim == 2) * WillHaveMSAA + 1) * (BufferNum + DepthBuffer); i++ ) {
            if( mRenderTargets[i] != nullptr ) {
                mRenderTargets[i]->Release();
                delete mRenderTargets[i];
            }
        }
    }

    void Resize(UINT w, UINT h, UINT d=1) {
        SetSize(w, h, d);

        // Store MSAA state
        bool oldMSAA = mMSAA;
        
        // Depth
        if constexpr( DepthBuffer ) {
            UINT bpp = std::get<UINT>(mRenderTargets[0]->mFormat);

            // Release resources
            if( mRenderTargets[0] != nullptr ) {
                mRenderTargets[0]->Release();
                ///delete mRenderTargets[0];
            }

            if( WillHaveMSAA ) {
                if( mRenderTargets[1] != nullptr ) {
                    mRenderTargets[1]->Release();
                    ///delete mRenderTargets[1];
                }

                if( mRenderTargets[2] != nullptr ) {
                    mRenderTargets[2]->Release();
                    ///delete mRenderTargets[2];
                }
            }

            // Re-create DSV
            Create(bpp, true);
        }

        if constexpr( BufferNum > 0 ) {
            for( size_t i = mOffset; i < mOffset + BufferNum; i++ ) {
                DXGI_FORMAT format = std::get<DXGI_FORMAT>(mRenderTargets[i]->mFormat);
                bool UAV = (mRenderTargets[i]->pUAV != nullptr);

                // Release resources
                if( mRenderTargets[i] != nullptr ) {
                    mRenderTargets[i]->Release();
                    ///delete mRenderTargets[i];
                }

                if constexpr( WillHaveMSAA ) {
                    if( mRenderTargets[i + BufferNum] != nullptr ) {
                        mRenderTargets[i + BufferNum]->Release();
                        ///delete mRenderTargets[i + BufferNum];
                    }
                }

                // Re-create RTV
                Create(format, (UINT)(i - mOffset), UAV, true);
            }
        }

        // Restore MSAA state
        mMSAA = oldMSAA;
    }

    // Clear RTV
    void Clear(const FLOAT Color0[4]) {
        if constexpr( !BufferNum ) return;
        for( UINT i = 0; i < BufferNum * (mMSAA + 1); i++ ) {
            auto rt = std::get<ID3D11RenderTargetView*>(mRenderTargets[mOffset + i]->pView);
            if( rt ) gDirectX->gContext->ClearRenderTargetView(rt, Color0);
        }
    }

    // Clear DSV
    void Clear(FLOAT Depth, UINT8 Stencil, UINT flags=D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL) {
        if constexpr( !DepthBuffer ) return;
        for( UINT i = 0; i < (mMSAA ? mOffset : 1); i++ ) {
            auto rt = std::get<ID3D11DepthStencilView*>(mRenderTargets[i]->pView);
            if( rt ) gDirectX->gContext->ClearDepthStencilView(rt, flags, Depth, Stencil);
        }
    }

    // 
    void SetViewportSize(float w, float h) {
        mViewPort.Width  = w;
        mViewPort.Height = h;
    }

    // Bind RTVs and DSV
    void Bind() {
        ID3D11DepthStencilView *ptrDSV = nullptr;

        // Get depth buffer view if avaliable
        if( DepthBuffer && (mRenderTargets[0]->pUAV == nullptr) ) {
            ptrDSV = std::get<ID3D11DepthStencilView*>(mRenderTargets[0]->pView);
        }

        if( BufferNum > 0 ) {
            ID3D11RenderTargetView *ptrRTV[std::max((size_t)1, BufferNum)];
            for( size_t i = 0; i < BufferNum; i++ )
                ptrRTV[i] = std::get<ID3D11RenderTargetView*>(mRenderTargets[mOffset + i]->pView);

            // 
            gDirectX->gContext->OMSetRenderTargets(BufferNum, ptrRTV, ptrDSV);
        } else {
            // Depth only
            gDirectX->gContext->OMSetRenderTargets(0, nullptr, ptrDSV);
        }
        
        // Bind viewport
        gDirectX->gContext->RSSetViewports(1, &mViewPort);

        // Bind as current
        //gState = this;
    }

    // Getters
    template<UINT index, bool noMSAA=true>
    inline implRenderTarget* GetBuffer() const {
        if constexpr( !BufferNum ) return nullptr;
        return mRenderTargets[mOffset + index + noMSAA * mMSAA * BufferNum];
    }

    template<UINT index, bool noMSAA=true>
    inline ID3D11ShaderResourceView* GetBufferSRV() const {
        if constexpr( !BufferNum ) return nullptr;
        return mRenderTargets[mOffset + index + noMSAA * mMSAA * BufferNum]->pSRV;
    }

    template<UINT index, bool noMSAA=true>
    inline ID3D11RenderTargetView* GetBufferRTV() const {
        if constexpr( !BufferNum ) return nullptr;
        return std::get<ID3D11RenderTargetView*>(mRenderTargets[mOffset + index + noMSAA * mMSAA * BufferNum]->pView);
    }

    template<UINT index=0, bool noMSAA=true>
    inline implRenderTarget* GetDepthBuffer() const {
        if constexpr( !DepthBuffer ) return nullptr;
        return mRenderTargets[index + 1 * noMSAA * mMSAA];
    }
    
    template<UINT index=0, bool noMSAA=true>
    inline ID3D11Resource* GetDepthBufferTexture() const {
        if constexpr( !DepthBuffer ) return nullptr;
        return Choose(mRenderTargets[index + 1 * noMSAA * mMSAA]->pTexture);
    }
    
    template<UINT index=0, bool noMSAA=true>
    inline ID3D11Resource* GetBufferTexture() const {
        if constexpr( !BufferNum ) return nullptr;
        return Choose(mRenderTargets[mOffset + index + noMSAA * mMSAA * BufferNum]->pTexture);
    }

    template<UINT index=0, bool noMSAA=true>
    inline ID3D11DepthStencilView* GetDSV() const {
        if constexpr( !DepthBuffer ) return nullptr;
        return std::get<ID3D11DepthStencilView*>(mRenderTargets[index + 1 * noMSAA * mMSAA]->pView);
    };
    
    template<UINT index=0, bool noMSAA=true>
    inline DXGI_FORMAT GetBufferFormat() const {
        if constexpr( !BufferNum ) return DXGI_FORMAT_UNKNOWN;
        return std::get<DXGI_FORMAT>(mRenderTargets[mOffset + index + noMSAA * mMSAA * BufferNum]->mFormat);
    };
    
    template<UINT index=0, bool noMSAA=true>
    inline DXGI_FORMAT GetDepthFormat() const {
        if constexpr( !DepthBuffer ) return DXGI_FORMAT_UNKNOWN;
        return BPP2DepthFormat(std::get<UINT>(mRenderTargets[mOffset + index + noMSAA * mMSAA * BufferNum]->mFormat));
    };

    // Bind resource
    void Bind(implRenderTarget* rt, Shader::ShaderType type, UINT slot=0, bool UAV=false) const {
        switch( type ) {
            case Shader::Vertex  : gDirectX->gContext->VSSetShaderResources(slot, 1, &rt->pSRV); break;
            case Shader::Pixel   : gDirectX->gContext->PSSetShaderResources(slot, 1, &rt->pSRV); break;
            case Shader::Geometry: gDirectX->gContext->GSSetShaderResources(slot, 1, &rt->pSRV); break;
            case Shader::Hull    : gDirectX->gContext->HSSetShaderResources(slot, 1, &rt->pSRV); break;
            case Shader::Domain  : gDirectX->gContext->DSSetShaderResources(slot, 1, &rt->pSRV); break;
            case Shader::Compute :
                if( UAV ) {
                    UINT InitCount = { 0 };
                    gDirectX->gContext->CSSetUnorderedAccessViews(slot, 1, &rt->pUAV, &InitCount);
                } else {
                    gDirectX->gContext->CSSetShaderResources(slot, 1, &rt->pSRV);
                }
                break;
        }
    }

    // index = [0; BufferNum + DepthBuffer]
    void Bind(UINT index, Shader::ShaderType type, UINT slot=0, bool UAV=false) const {
        if( index == 0 ) {
            // Bind depth
            if( DepthBuffer ) {
                Bind(mRenderTargets[mMSAA * 2], type, slot, UAV);
                return;
            }
        }

        // There is no depth buffer!
        // Bind a non-depth buffer
        Bind(mRenderTargets[mOffset + index - UINT(DepthBuffer) + mMSAA * BufferNum], type, slot, UAV);
    }

    void BindResources(Shader::ShaderType type, UINT slot=0) const {
        for( UINT i = 0; i < BufferNum + DepthBuffer; i++ ) Bind(i, type, slot);
    }

    // Get Dims
    inline size_t GetWidth()  const { return mWidth;  }
    inline size_t GetHeight() const { return mHeight; }
    inline size_t GetDepth()  const { return (dim == 3) ? mDepth : 0; }

    inline UINT GetSampleNum(UINT i) const { return mMSAASamples[i];  }

    // Template arguments and boolean checks
    inline bool HasMinDim(size_t minDim) const { return dim >= minDim; }
    inline bool HasMinBuff(size_t minB ) const { return BufferNum >= minB; }
    inline bool HasDepth()               const { return DepthBuffer; }
    inline bool HasMSAA()                const { return WillHaveMSAA; }
    inline bool IsMSAAEnabled()          const { return mMSAA; }
    inline size_t GetDim()               const { return dim; }
    inline UINT   GetMaxSampleCount()    const { return mMSAAMaxLevel; };
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

// MSAA variants
using RenderTarget2DColor1MSAA = RenderTarget<2, 1, false, 1, true>;
using RenderTarget2DColor2MSAA = RenderTarget<2, 2, false, 1, true>;
using RenderTarget2DColor3MSAA = RenderTarget<2, 3, false, 1, true>;
using RenderTarget2DColor4MSAA = RenderTarget<2, 4, false, 1, true>;
using RenderTarget2DColor5MSAA = RenderTarget<2, 5, false, 1, true>;
using RenderTarget2DColor6MSAA = RenderTarget<2, 6, false, 1, true>;
using RenderTarget2DColor7MSAA = RenderTarget<2, 7, false, 1, true>;
using RenderTarget2DColor8MSAA = RenderTarget<2, 8, false, 1, true>;

using RenderTarget2DColor1DepthMSAA = RenderTarget<2, 1, true, 1, true>;
using RenderTarget2DColor2DepthMSAA = RenderTarget<2, 2, true, 1, true>;
using RenderTarget2DColor3DepthMSAA = RenderTarget<2, 3, true, 1, true>;
using RenderTarget2DColor4DepthMSAA = RenderTarget<2, 4, true, 1, true>;
using RenderTarget2DColor5DepthMSAA = RenderTarget<2, 5, true, 1, true>;
using RenderTarget2DColor6DepthMSAA = RenderTarget<2, 6, true, 1, true>;
using RenderTarget2DColor7DepthMSAA = RenderTarget<2, 7, true, 1, true>;
using RenderTarget2DColor8DepthMSAA = RenderTarget<2, 8, true, 1, true>;

using RenderTarget2DDepthMSAA = RenderTarget<2, 0, true, 1, true>;


// Interface
typedef RenderTarget<1, 0> IRenderTarget;

