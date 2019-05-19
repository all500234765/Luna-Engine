#pragma once

#include <string>

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Materials/Texture.h"

static const std::string sSideNames[6] = {
    "Front.png", 
    "Right.png", 
    "Back.png", 
    "Left.png", 
    "Top.png", 
    "Bottom.png"
};

class CubemapTexture: public DirectXChild {
private:
    int Width, Height, channels;

    ID3D11Texture2D *pTexture;
    ID3D11ShaderResourceView *pSRV[6];

    // Use depth buffer, either color buffer
    union {
        ID3D11RenderTargetView *pRTVs[6]; // Not avalilable when loaded, nor when isDepthMap == true
        ID3D11DepthStencilView *pDSVs[6]; // Not avalilable when loaded, nor when isDepthMap == false
    };

    bool isLoaded   = false;
    bool isDepthMap = false;
public:
    void CreateFromFiles(std::string folder, bool bDepth, DXGI_FORMAT format);

    void CreateSide(int side, ID3D11Texture2D* tex);

    void Release();
};
