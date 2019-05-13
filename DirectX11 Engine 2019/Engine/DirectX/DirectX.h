#pragma once

// 3D Rendering (primarely)
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

// 2D Rendering
/*#include <d2d1.h>
#include <dwrite.h>
#include <comdef.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib") */

#ifndef ConvertPointSizeToDIP
#define ConvertPointSizeToDIP(x) ((x) * 1.3333333333333333333333333333333f)
#endif

// 
struct DirectXConfig {
    // Swapchain settings
    int BufferCount;
    int Width, Height;
    bool Windowed, UseHDR;
    int RefreshRate;
    HWND m_hwnd;
    bool Ansel;
    
    bool MSAA;
    int MSAA_Samples,  // 
        MSAA_Quality;  // 

};

class _DirectX {
private:
    // Main
    IDXGISwapChain *gSwapchain;

    // RTVs
    ID3D11RenderTargetView *gRTV;
    ID3D11DepthStencilView *gDSV; ID3D11ShaderResourceView *gDSV_SRV;

    // Textures
    ID3D11Texture2D *gDSVTex;

    // States
    ID3D11DepthStencilState *pDSS_Default;

    ID3D11RasterizerState *gRSDefault;
    ID3D11RasterizerState *gRSDefaultWriteframe;

    // CFG
    DirectXConfig cfg;

    // Descs
    D3D11_DEPTH_STENCIL_VIEW_DESC pDesc2;
    D3D11_TEXTURE2D_DESC pTex2DDesc;
    D3D11_DEPTH_STENCIL_DESC pDSD;
public:
    // Globals
    // 3D Rendering
    ID3D11Device *gDevice;
    ID3D11DeviceContext *gContext;
    
    // 2D Rendering
    /*ID2D1HwndRenderTarget *g2DRT;

    ID2D1Factory *g2DFactory;
    IDWriteFactory *g2DWriteFactory;

    // Uniques
    IDWriteTextFormat *gTextFormat_Arial;
    ID2D1SolidColorBrush *gSolidLimeBrush;

    // Very uniques
    IDWriteTextLayout *gTextLayout; // Per string and format - unique*/

    // Game state
    bool gPause, gAnselSessionIsActive;
    DirectX::XMMATRIX gViewBackup, gView;

    _DirectX();
    
    // Setup DirectX
    int Create(DirectXConfig config);
    void ShowError(int id);

    // 
    bool FrameFunction();
    void Tick();
    void ComposeUI();
    void Resize();
    void Load();
    void Unload();

    void AnselEnable(DirectX::XMMATRIX view);
    void AnselSession();
};
