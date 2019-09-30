#pragma once

#include "Vendor/STB/stbi_image.h"
#include "Vendor/TinyDDSLoader/TinyDDSLoader.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Utility/Utils.h"

#include <string>

class Texture: public DirectXChild {
private:
    int w, h, channels;
    UINT flags;
    ID3D11Texture2D *pTexture = 0;
    ID3D11ShaderResourceView *pSRV = 0;
    ID3D11UnorderedAccessView *pUAV = 0;

    D3D11_TEXTURE2D_DESC pDesc;
    bool bDepth;

public:
    Texture();
    Texture(UINT Width, UINT Height, DXGI_FORMAT format, bool UAV=false, bool Depth=false);
    Texture(std::string fname, UINT bpc=8, bool UAV=false);
    Texture(std::string fname, DXGI_FORMAT format, bool UAV=false);

    void Load(std::string fname, UINT bpc=8, bool UAV=false, bool bGenMips=false);
    void Load(std::string fname, DXGI_FORMAT format, bool UAV=false, bool bGenMips=false);
    void Create(void* data, DXGI_FORMAT format, UINT bpp, UINT SlicePitch=0, bool UAV=false, bool bGenMips2=false);
    void Bind(Shader::ShaderType type, UINT slot=0, bool UAV=false);
    bool IsCreated();
    void Release();

    void SetName(const char* name) { _SetName(pTexture, name); }

    void Resize(UINT Width, UINT Height, bool Save=false);

    int GetWidth();
    int GetHeight();
    ID3D11ShaderResourceView* GetSRV();
    ID3D11Texture2D* GetTexture();
    inline ID3D11UnorderedAccessView* GetUAV() const { return pUAV; };
};

// TODO: Different amount of channels support
