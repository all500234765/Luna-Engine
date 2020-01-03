#include "pc.h"

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "HighLevel/DirectX/Utlities.h"

enum ResizeFlag {
    // Keep at top-left corner
    ResizeFlag_Keep, 

    // Stretch to new size
    ResizeFlag_Stretch,

    // Clear with 0, 0, 0, 0
    ResizeFlag_Clear, 

};

enum TextureFlags {
    // Texture dimension
    dim_1 = 1, dim_2 = 2, dim_3 = 4, 

    // Misc
    _Depth = 8, _Array = 16, _UAV = 32, 
    _MSAA = 64, _Cube = 128, _MipMaps = 256,
    _NoClampMip = 2048,

    // CPU Access
    _NoCPURead = 512, _NoCPUWrite = 1024, 

    // Restricted content
    _RestrictedContent = 4096, _HWRestrictedContent = 8192
};

struct implTexture {
    // Texture unit
    std::variant<bool, ID3D11Texture1D*, ID3D11Texture2D*, ID3D11Texture3D*> pTexture;

    // Format unit
    std::variant<DXGI_FORMAT, UINT> mFormat;

    // Flags
    union {
        uint32_t mFlags;
        struct {
            uint32_t dim        : 2;
            uint32_t IsDepth    : 1;
            uint32_t IsArray    : 1;
            uint32_t HasUAV     : 1;
            uint32_t MSAA       : 1; // ?
            uint32_t IsCube     : 1;
            uint32_t MipMaps    : 1;
            uint32_t NoCPURead  : 1;
            uint32_t NoCPUWrite : 1;
            uint32_t NoClampMip : 1;
            uint32_t RContent   : 1;
            uint32_t HWRContent : 1;
        };
    };

    // Resource units
    ID3D11UnorderedAccessView *pUAV;
    ID3D11ShaderResourceView  *pSRV;

    inline ID3D11Resource* GetTexture1() const; // { return std::get<ID3D11Texture1D*>(pTexture); }
    inline ID3D11Resource* GetTexture2() const; // { return std::get<ID3D11Texture2D*>(pTexture); }
    inline ID3D11Resource* GetTexture3() const; // { return std::get<ID3D11Texture3D*>(pTexture); }

    void Release();
};

class Texture: public DirectXChild {
protected:
    static uint32_t mMaxMipMapLevels;

private:
    union {
        uint32_t mFlags;
        struct {
            uint32_t dim        : 2;
            uint32_t IsDepth    : 1;
            uint32_t IsArray    : 1;
            uint32_t HasUAV     : 1;
            uint32_t MSAA       : 1; // ?
            uint32_t IsCube     : 1;
            uint32_t MipMaps    : 1;
            uint32_t NoCPURead  : 1;
            uint32_t NoCPUWrite : 1;
            uint32_t NoClampMip : 1;
            uint32_t RContent   : 1;
            uint32_t HWRContent : 1;
        };
    };

    // Texture dimensions
    uint32_t mWidth, mHeight, mDepth;

    // 
    implTexture* mTextureUnit;

    // Name
    const char* mName;

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

    implTexture* CreateTexture(std::variant<DXGI_FORMAT, UINT> format, void* data=nullptr, UINT ArraySize=1) {
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

        // Cheese the texture format
        DXGI_FORMAT formatTex, formatDSV, formatSRV;
        if( IsDepth ) {
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
        uint32_t Quality = 0;
        uint32_t SampleCount = 1;



        // Create sub-resource data
        D3D11_SUBRESOURCE_DATA *SubResource = new D3D11_SUBRESOURCE_DATA[ArraySize * (5u * IsCube + 1u)];



        // Display warning
        if( HasUAV && IsDepth ) {
            // TODO: Test
            static const char* name[] = { "R16_TYPELESS", "R24G8_TYPELESS", "R32_TYPELESS" };
            
            printf_s("[Texture::CreateTexture]: Warning: can't create DSV for texture with UAV! Creating %s type texture with UAV\n", 
                     name[(formatTex == DXGI_FORMAT_R32_TYPELESS) * 2 + (formatTex == DXGI_FORMAT_R24G8_TYPELESS)]);
        }

        // 
        UINT BindFlags = D3D11_BIND_SHADER_RESOURCE
                       | (HasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0)
                       | (IsDepth ? (HasUAV ? 0 : D3D11_BIND_DEPTH_STENCIL) : D3D11_BIND_RENDER_TARGET);
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

            res = gDirectX->gDevice->CreateTexture1D(&pTexDesc, SubResource, &pTexture._1D);
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

            res = gDirectX->gDevice->CreateTexture2D(&pTexDesc, SubResource, &pTexture._2D);
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

            res = gDirectX->gDevice->CreateTexture3D(&pTexDesc, SubResource, &pTexture._3D);
        }

        if( FAILED(res) ) {
            std::cout << "[RT]: Failed to create " << dim << "D texture" << std::endl;
        }

        // Free memory
        delete[] SubResource;



    }

public:
    // Deleted call
    Texture() = delete;

    // Create empty texture
    Texture(UINT flags, std::variant<DXGI_FORMAT, UINT> format,
            uint32_t w, uint32_t h, uint32_t d=1u, const char* name="UnnamedEmptyTexture");

    // Auto loading from file
    Texture(const char* fname, UINT flags=0u, const char* name="UnnamedTexture");

    // Setters
    void SetMinLOD(uint32_t min_lod);


    // Getters
    

    // 
    void Resize(uint32_t w, uint32_t h, uint32_t d=1u, ResizeFlag SaveContent=ResizeFlag_Keep);

};
