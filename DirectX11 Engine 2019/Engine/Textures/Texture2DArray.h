#pragma once

#include "pc.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/DirectXChild.h"

// Texture2DArray for shader resource
// //You can write to texture with CPU or GPU /* nope */
class Texture2DResource: public DirectXChild {
private:
    int Width, Height, channels;

    bool isLoaded = false;
    bool isDepthMap = false;

    ID3D11Texture2D *pTexture;
    ID3D11ShaderResourceView *pSRV;

    // Use depth buffer, either color buffer
    union {
        //ID3D11RenderTargetView *pRTVs[6]; // Not avalilable when loaded, nor when isDepthMap == true
        //ID3D11DepthStencilView *pDSVs[6]; // Not avalilable when loaded, nor when isDepthMap == false
    };
public:
    void CreateFromFiles(std::vector<std::string> files, bool bDepth, DXGI_FORMAT format);
    
    void Bind(Shader::ShaderType type, UINT slot=0);
    void Release();
};
