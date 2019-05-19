#pragma once

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/DirectX/Shader.h"

#include <vector>

typedef enum {
    RTV = 1, 
    DSV = 2, 
    Is2D = 4, 
    Is3D = 8, 
} RenderBufferFlags;

struct sRenderBuffer {
    // Target unit
    union {
        ID3D11DepthStencilView *pDSV;
        ID3D11RenderTargetView *pRTV;
    };

    // Texture unit
    union {
        ID3D11Texture2D *pTexture2D;
        ID3D11Texture3D *pTexture3D;
    };

    // Resource unit
    ID3D11ShaderResourceView *pSRV;
    char Flags; // Flags

    void Release() {
        if( ((Flags & Is3D) == Is3D) && pTexture3D != nullptr ) { pTexture3D->Release(); }
        if( ((Flags & Is2D) == Is2D) && pTexture2D != nullptr ) { pTexture2D->Release(); }
        
        if( ((Flags & DSV) == DSV) && pDSV != nullptr ) { pDSV->Release(); }
        if( ((Flags & RTV) == RTV) && pRTV != nullptr ) { pRTV->Release(); }

        if( pSRV ) { pSRV->Release(); }
    }
};

class RenderBufferBase: public DirectXChild {
protected:
    int Width, Height, Depth;

public:
    sRenderBuffer* CreateRTV2D(int W, int H, DXGI_FORMAT format);
    sRenderBuffer* CreateDSV2D(int W, int H, UINT bpp=32);
    sRenderBuffer* CreateRTV3D(int W, int H, int D, DXGI_FORMAT format);
    sRenderBuffer* CreateDSV3D(int W, int H, int D, UINT bpp=32);

    void BindResource(sRenderBuffer* data, Shader::ShaderType type, UINT slot = 0);

    void BindTarget(sRenderBuffer* dRTV, sRenderBuffer* dDSV);
    void BindTarget(sRenderBuffer* data);
    void BindTarget(std::vector<sRenderBuffer*> dRTV, sRenderBuffer* dDSV);
    void BindTarget(std::vector<sRenderBuffer*> dRTV);
    void BindTarget(sRenderBuffer* dRTV, ID3D11DepthStencilView* pDSV);
    void BindTarget(ID3D11RenderTargetView* pRTV, sRenderBuffer* dDSV);
};
