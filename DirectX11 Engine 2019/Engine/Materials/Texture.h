#pragma once

#include "Vendor/STB/stbi_image.h"
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

public:
    Texture();
    Texture(std::string fname, UINT bpc = 8);
    Texture(std::string fname, DXGI_FORMAT format);

    void Load(std::string fname, UINT bpc=8);
    void Load(std::string fname, DXGI_FORMAT format);
    void Create(void* data, DXGI_FORMAT format, UINT bpp);
    void Bind(Shader::ShaderType type, UINT slot=0);
    bool IsCreated();
    void Release();

    void SetName(const char* name) { _SetName(pTexture, name); }

    int GetWidth();
    int GetHeight();
    ID3D11ShaderResourceView* GetSRV();
    ID3D11Texture2D* GetTexture();
};
