#include "pc.h"

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "HighLevel/DirectX/Utlities.h"

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

    implTexture* CreateTexture(std::variant<> format, void* data=nullptr) {
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

        // 


    }

public:
    // Deleted call
    Texture() = delete;

    // Create empty texture
    Texture(TextureFlags flags, uint32_t w, uint32_t h, uint32_t d=1u, const char* name="UnnamedEmptyTexture");

    // Auto loading from file
    Texture(const char* fname, const char* name = "UnnamedTexture");

    // Setters
    void SetMinLOD(uint32_t min_lod);



};
