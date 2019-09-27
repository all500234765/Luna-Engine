#pragma once

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/DirectX/Shader.h"

#include <vector>

typedef enum {
    RTV = 1, 
    DSV = 2, 
    Is2D = 4, 
    Is3D = 8, 
    IsCube = 16,  // TODO: Cube texture support

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

    // Format unit
    union {
        UINT bpp;
        DXGI_FORMAT format;
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
    D3D11_VIEWPORT mVP;

    sRenderBuffer* CreateRTV2D(int W, int H, DXGI_FORMAT format);
    sRenderBuffer* CreateDSV2D(int W, int H, UINT bpp=32);
    sRenderBuffer* CreateRTV3D(int W, int H, int D, DXGI_FORMAT format);
    sRenderBuffer* CreateDSV3D(int W, int H, int D, UINT bpp=32);

public:
    void BindResource(sRenderBuffer* data, Shader::ShaderType type, UINT slot = 0);
    void SetSize(int w, int h);
    int GetWidth();
    int GetHeight();
    int GetDepth();

    virtual sRenderBuffer* GetColor0() = 0;
    //virtual sRenderBuffer* GetDepth()  = 0;

    void BindTarget(sRenderBuffer* dRTV, sRenderBuffer* dDSV);
    void BindTarget(sRenderBuffer* data);
    void BindTarget(std::vector<sRenderBuffer*> dRTV, sRenderBuffer* dDSV);
    void BindTarget(std::vector<sRenderBuffer*> dRTV);
    void BindTarget(sRenderBuffer* dRTV, ID3D11DepthStencilView* pDSV);
    void BindTarget(ID3D11RenderTargetView* pRTV, sRenderBuffer* dDSV);
};
