#pragma once

#include "pc.h"

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Extensions/Safe.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine/Utility/Utils.h"

enum class TResizeFlag {
    // Keep at top-left corner
    Keep, 

    // Stretch to new size
    Stretch,

    // Clear with 0, 0, 0, 0
    Clear

};

enum TextureFlags: uint32_t {
    // Texture dimension
    tf_dim_1 = 1, tf_dim_2 = 2, tf_dim_3 = 3,
    tf_dim_mask = 3, 

    // Misc
    tf_Array = 4, tf_UAV = 8,
    tf_MSAA = 16, tf_Cube = 32, tf_MipMaps = 64,
    tf_ClampMip = 512, tf_Immutable = 4096,
    tf_IsTilePool = 8192, tf_IsTiled = 16384,

    // CPU Access
    tf_CPURead = 64, tf_CPUWrite = 256,

    // Restricted content
    tf_RestrictedContent = 1024, tf_HWRestrictedContent = 2048,

};

struct implTexture {
    // Texture unit
    std::variant<bool, ID3D11Texture1D*, ID3D11Texture2D*, ID3D11Texture3D*> pTexture;

    // Format unit
    DXGI_FORMAT mFormat;

    // Flags
    union {
        uint32_t mFlags;
        struct {
            uint32_t dim        : 2; // 0-1
            uint32_t IsArray    : 1; // 2
            uint32_t HasUAV     : 1; // 3
            uint32_t MSAA       : 1; // 4 // ?
            uint32_t IsCube     : 1; // 5
            uint32_t HasMipMaps : 1; // 6
            uint32_t CPURead    : 1; // 7
            uint32_t CPUWrite   : 1; // 8
            uint32_t ClampMip   : 1; // 9
            uint32_t RContent   : 1; // 10
            uint32_t HWRContent : 1; // 11
            uint32_t Immutable  : 1; // 12
            uint32_t IsTilePool : 1; // 13
            uint32_t IsTiled    : 1; // 14
        };
    };

    // Resource units
    ID3D11UnorderedAccessView *pUAV;
    ID3D11ShaderResourceView  *pSRV;

    inline ID3D11Resource* GetTexture1D() const { return std::get<ID3D11Texture1D*>(pTexture); }
    inline ID3D11Resource* GetTexture2D() const { return std::get<ID3D11Texture2D*>(pTexture); }
    inline ID3D11Resource* GetTexture3D() const { return std::get<ID3D11Texture3D*>(pTexture); }

    void Release();
};

class Texture: public DirectXChild {
protected:
    static uint32_t mMaxMipMapLevels;

private:
    union {
        uint32_t mFlags;
        struct {
            uint32_t dim        : 2; // 0-1
            uint32_t IsArray    : 1; // 2
            uint32_t HasUAV     : 1; // 3
            uint32_t MSAA       : 1; // 4 // ?
            uint32_t IsCube     : 1; // 5
            uint32_t HasMipMaps : 1; // 6
            uint32_t CPURead    : 1; // 7
            uint32_t CPUWrite   : 1; // 8
            uint32_t ClampMip   : 1; // 9
            uint32_t RContent   : 1; // 10
            uint32_t HWRContent : 1; // 11
            uint32_t Immutable  : 1; // 12
            uint32_t IsTilePool : 1; // 13
            uint32_t IsTiled    : 1; // 14
        };
    };

    // Texture dimensions
    uint32_t mWidth, mHeight, mDepth;
    uint32_t mArraySize{};

    // 
    float mMinLod = 0.f;
    uint32_t mMipMaps = 0u;
    implTexture* mTextureUnit{};

    bool bUndefined = true;

    // Name
    std::string mName;

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

    static ID3D11Resource* ChooseS(implTexture* tex) {
        if( tex->dim == 1 ) return std::get<ID3D11Texture1D*>(tex->pTexture);
        if( tex->dim == 2 ) return std::get<ID3D11Texture2D*>(tex->pTexture);
        return std::get<ID3D11Texture3D*>(tex->pTexture);
    }

    implTexture* CreateTexture(DXGI_FORMAT format, D3D11_SUBRESOURCE_DATA *SubResource, 
                               uint32_t ArraySize=1, implTexture* Out=nullptr) const;

public:
    // Deleted call
    Texture() {};

    // Create empty texture
    Texture(UINT flags, DXGI_FORMAT format, uint32_t w, uint32_t h, uint32_t d=1u, 
             uint32_t ArraySize=1u, std::string_view name="UnnamedEmptyTexture");

    // Auto loading from file
    Texture(std::string_view fname, UINT flags=0u, std::string_view name="UnnamedTexture", 
             uint32_t ArraySize=1u, DXGI_FORMAT custom_format=DXGI_FORMAT_UNKNOWN);

    void Load(std::string_view fname, UINT flags=0u, uint32_t ArraySize=1u, 
              DXGI_FORMAT custom_format=DXGI_FORMAT_UNKNOWN);

    // Setters
    inline void SetMinLOD(float min_lod) {
        mMinLod = min_lod;
        if( mTextureUnit ) gDirectX->gContext->SetResourceMinLOD(mTextureUnit->GetTexture3D(), min_lod);
    }

    inline void SetName(const std::string& name) {
        mName = name;
        //mName.copy(const_cast<char*>(name.data()), name.length());
        using namespace std::string_literals;
        if( mTextureUnit ) {
            std::string str = std::string(name.data());
            _SetName(Choose(mTextureUnit->pTexture), ("[Texture]: "s + str).c_str());
        }
    }
    
    // Getters
    inline ID3D11ShaderResourceView*  GetSRV()         const { return mTextureUnit->pSRV;                }
    inline ID3D11UnorderedAccessView* GetUAV()         const { return mTextureUnit->pUAV;                }
    inline ID3D11Resource*            GetResource()    const { return Choose(mTextureUnit->pTexture);    }
    inline uint32_t                   GetFlags()       const { return mFlags;                            }
    inline implTexture*               GetTexture()     const { return mTextureUnit;                      }
    inline std::string_view           GetName()        const { return mName;                             }
    inline uint32_t                   GetWidth()       const { return mWidth;                            }
    inline uint32_t                   GetHeight()      const { return mHeight;                           }
    inline uint32_t                   GetDepth()       const { return mDepth;                            }
    inline uint32_t                   GetArraySize()   const { return mArraySize;                        }
    inline DXGI_FORMAT                GetFormat()      const { return mTextureUnit->mFormat;             }
    inline uint32_t                   GetCubeNum()     const { return (uint32_t)floor(mArraySize / 6.f); }
    inline uint32_t                   GetChannelNum()  const { return Format2Ch(mTextureUnit->mFormat);  }

    inline uint32_t                   GetSliceLength() const { return GetLineLength() * GetHeight();     };
    inline uint32_t                   GetLineLength()  const { return GetWidth() *  Format2BPP(GetFormat()); }
    inline uint32_t                   GetWHDCN()       const { return GetWidth() * GetHeight() * GetDepth() * GetChannelNum(); }

    // Copy data from another texture
    inline void Copy(Texture         *src) const { gDirectX->gContext->CopyResource(GetResource(), src->GetResource()); }
    inline void Copy(implTexture     *src) const { gDirectX->gContext->CopyResource(GetResource(), Choose(src->pTexture)); }
    inline void Copy(ID3D11Texture2D *src) const { gDirectX->gContext->CopyResource(GetResource(), src); }

    static inline void Copy(Texture         *dest, Texture         *src) { gDirectX->gContext->CopyResource(dest->GetResource(), src->GetResource()); }
    static inline void Copy(Texture         *dest, implTexture     *src) { gDirectX->gContext->CopyResource(dest->GetResource(), ChooseS(src)); }
    static inline void Copy(Texture         *dest, ID3D11Texture2D *src) { gDirectX->gContext->CopyResource(dest->GetResource(), src); }

    static inline void Copy(implTexture     *dest, Texture         *src) { gDirectX->gContext->CopyResource(ChooseS(dest), src->GetResource()); }
    static inline void Copy(implTexture     *dest, implTexture     *src) { gDirectX->gContext->CopyResource(ChooseS(dest), ChooseS(src)); }
    static inline void Copy(implTexture     *dest, ID3D11Texture2D *src) { gDirectX->gContext->CopyResource(ChooseS(dest), src); }

    static inline void Copy(ID3D11Texture2D *dest, Texture         *src) { gDirectX->gContext->CopyResource(dest, src->GetResource()); }
    static inline void Copy(ID3D11Texture2D *dest, implTexture     *src) { gDirectX->gContext->CopyResource(dest, ChooseS(src)); }
    static inline void Copy(ID3D11Texture2D *dest, ID3D11Texture2D *src) { gDirectX->gContext->CopyResource(dest, src); }

    void Copy(Texture* src, uint32_t dst_x, uint32_t dst_y, uint32_t dst_z, uint32_t dst_array, uint32_t dst_mip,
              uint32_t src_mip, uint32_t src_array);

    static void CopyS(Texture* dest, Texture* src, uint32_t dst_x, uint32_t dst_y, uint32_t dst_z,
                      uint32_t dst_array, uint32_t dst_mip, uint32_t src_mip, uint32_t src_array);

    static void CopyS(implTexture* dest, Texture* src, uint32_t dst_x, uint32_t dst_y, uint32_t dst_z,
                      uint32_t dst_array, uint32_t dst_mip, uint32_t src_mip, uint32_t src_array);

    void ClearStg(float4 color);
    void ClearStg(uint4 color);

    void ClearRtv(const float color[4]) const;

    /*template<typename T>
    void Clear(T color) {
        if( Immutable ) return;
        ID3D11Resource* stg = CreateStaging();
        D3D11_MAPPED_SUBRESOURCE pMapped{};
        if( gDirectX->gContext->Map(stg, 0, D3D11_MAP_WRITE, 0, &pMapped) == S_OK ) {
            memcpy_s(&pMapped.pData, GetWidth() * GetHeight() * GetDepth() * GetChannelNum(), &color, GetChannelNum() * sizeof(T::x));

            gDirectX->gContext->Unmap(stg, 0);
        }

        stg->Release();
    }*/

    implTexture* CreateStaging() const;
    implTexture* CreateStaging(float4 color);
    implTexture* CreateStaging(uint4 color);

    static Texture* CreateStaging(uint32_t flags, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, uint32_t array_size);

    void SetSubresource(const D3D11_SUBRESOURCE_DATA* resource, UINT mip=0, UINT array=0);
    void Resize(uint32_t w, uint32_t h, uint32_t d=1u, TResizeFlag SaveContent= TResizeFlag::Clear);
    void Bind(UINT type, UINT slot=0u, bool UAV=false);
    void Release();
};
