#include "pc.h"

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "HighLevel/DirectX/Utlities.h"
#include "Engine/Utility/Utils.h"

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
    tf_dim_1 = 1, tf_dim_2 = 2, tf_dim_3 = 4,

    // Misc
    tf_Depth = 8, tf_Array = 16, tf_UAV = 32,
    tf_MSAA = 64, tf_Cube = 128, tf_MipMaps = 256,
    tf_ClampMip = 2048, tf_Immutable = 16384,
    tf_IsTilePool = 32768, tf_IsTiled = 65536, 


    // CPU Access
    tf_CPURead = 512, tf_CPUWrite = 1024,

    // Restricted content
    tf_RestrictedContent = 4096, tf_HWRestrictedContent = 8192,

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
            uint32_t HasMipMaps : 1;
            uint32_t CPURead    : 1;
            uint32_t CPUWrite   : 1;
            uint32_t ClampMip   : 1;
            uint32_t RContent   : 1;
            uint32_t HWRContent : 1;
            uint32_t Immutable  : 1;
            uint32_t IsTilePool : 1;
            uint32_t IsTiled    : 1;
        };
    };

    // Resource units
    ID3D11UnorderedAccessView *pUAV;
    ID3D11ShaderResourceView  *pSRV;

    inline ID3D11Resource* GetTexture1() const { return std::get<ID3D11Texture1D*>(pTexture); }
    inline ID3D11Resource* GetTexture2() const { return std::get<ID3D11Texture2D*>(pTexture); }
    inline ID3D11Resource* GetTexture3() const { return std::get<ID3D11Texture3D*>(pTexture); }

    void Release();
};

class Texture2: public DirectXChild {
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
            uint32_t HasMipMaps : 1;
            uint32_t CPURead    : 1;
            uint32_t CPUWrite   : 1;
            uint32_t ClampMip   : 1;
            uint32_t RContent   : 1;
            uint32_t HWRContent : 1;
            uint32_t Immutable  : 1;
            uint32_t IsTilePool : 1;
            uint32_t IsTiled    : 1;
        };
    };

    // Texture dimensions
    uint32_t mWidth, mHeight, mDepth;

    // 
    uint32_t mMinLod = 0u, mMipMaps = 0u;
    implTexture* mTextureUnit;

    // Name
    std::string_view mName;

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

    implTexture* CreateTexture(std::variant<DXGI_FORMAT, UINT> format, D3D11_SUBRESOURCE_DATA *SubResource, 
                               uint32_t ArraySize=1, implTexture* Out=nullptr) ;

public:
    // Deleted call
    Texture2() = delete;

    // Create empty texture
    Texture2(UINT flags, std::variant<DXGI_FORMAT, UINT> format, uint32_t w, uint32_t h, uint32_t d=1u, 
             uint32_t ArraySize=1u, std::string_view name="UnnamedEmptyTexture");

    // Auto loading from file
    Texture2(std::string_view fname, UINT flags=0u, std::string_view name="UnnamedTexture", uint32_t ArraySize=1u);

    void Load(std::string_view fname, UINT flags=0u, uint32_t ArraySize=1u);

    // Setters
    inline void SetMinLOD(uint32_t min_lod) {
        mMinLod = min_lod;
        if( mTextureUnit ) gDirectX->gContext->SetResourceMinLOD(mTextureUnit->GetTexture3(), min_lod);
    }

    inline void SetName(std::string_view name) {
        mName = name;
        if( mTextureUnit ) _SetName(mTextureUnit->GetTexture3(), name.data());
    }
    
    // Getters
    inline ID3D11ShaderResourceView*  GetSRV()      const { return mTextureUnit->pSRV;          }
    inline ID3D11UnorderedAccessView* GetUAV()      const { return mTextureUnit->pUAV;          }
    inline ID3D11Resource*            GetResource() const { return mTextureUnit->GetTexture3(); }
    inline uint32_t                   GetFlags()    const { return mFlags;                      }
    inline implTexture*               GetTexture()  const { return mTextureUnit;                }
    inline std::string_view           GetName()     const { return mName;                       }

    // 
    void SetSubresource(const D3D11_SUBRESOURCE_DATA* resource, UINT mip=0, UINT array=0);
    void Copy(ID3D11Resource* src);
    void Resize(uint32_t w, uint32_t h, uint32_t d=1u, ResizeFlag SaveContent=ResizeFlag_Keep);
    void Bind(Shader::ShaderType type, UINT slot=0u);
    void Release();
};
