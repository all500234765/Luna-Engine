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

    void Resize(ID3D11Device* dev, int w, int h) {
        if( ((Flags & DSV) == DSV) && pDSV != nullptr ) {
            // Resize if DSV
            D3D11_DEPTH_STENCIL_VIEW_DESC pDesc;
            pDSV->GetDesc(&pDesc);

            if( ((Flags & Is3D) == Is3D) ) {
                // 3D Texture
                //D3D11_SUBRESOURCE_DATA pData;
                D3D11_TEXTURE3D_DESC pDescTex;
                pTexture3D->GetDesc(&pDescTex);
                pTexture3D->Release();
                pTexture3D = 0;

                pDescTex.Width = w;
                pDescTex.Height = h;

                dev->CreateTexture3D(&pDescTex, NULL, &pTexture3D);
            } else {
                // 2D Texture
                //D3D11_SUBRESOURCE_DATA pData;
                D3D11_TEXTURE2D_DESC pDescTex;
                pTexture2D->GetDesc(&pDescTex);
                pTexture2D->Release();
                pTexture2D = 0;

                pDescTex.Width = w;
                pDescTex.Height = h;

                dev->CreateTexture2D(&pDescTex, NULL, &pTexture2D);
            }


        }

        if( ((Flags & RTV) == RTV) && pRTV != nullptr ) {
            // Resize if RTV
            D3D11_RENDER_TARGET_VIEW_DESC pDesc;
            pRTV->GetDesc(&pDesc);

            if( ((Flags & Is3D) == Is3D) ) {
                // 3D Texture
                //D3D11_SUBRESOURCE_DATA pData;
                D3D11_TEXTURE3D_DESC pDescTex;
                pTexture3D->GetDesc(&pDescTex);
                pTexture3D->Release();
                pTexture3D = 0;

                pDescTex.Width = w;
                pDescTex.Height = h;

                dev->CreateTexture3D(&pDescTex, NULL, &pTexture3D);
            } else {
                // 2D Texture
                //D3D11_SUBRESOURCE_DATA pData;
                D3D11_TEXTURE2D_DESC pDescTex;
                pTexture2D->GetDesc(&pDescTex);
                pTexture2D->Release();
                pTexture2D = 0;

                pDescTex.Width = w;
                pDescTex.Height = h;

                dev->CreateTexture2D(&pDescTex, NULL, &pTexture2D);
            }


        }
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
