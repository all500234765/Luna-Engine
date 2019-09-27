#pragma once

// 3D Rendering (primarely)
#include <d3d11_4.h>
#include <dxgi1_2.h>
#include <d3dcommon.h>
#include <DirectXMath.h>

//#include "HighLevel/DirectX/HighLevel.h"

#pragma comment(lib, "dxgi.lib")

#ifndef _BUILD_DX12_
#pragma comment(lib, "d3d11.lib")
#else
#pragma comment(lib, "d3d12.lib")
#endif

#ifndef ConvertPointSizeToDIP
#define ConvertPointSizeToDIP(x) ((x) * 1.3333333333333333333333333333333f)
#endif

// 
struct DirectXConfig {
    // Swapchain settings
    int BufferCount;
    int Width, Height;
    bool Windowed, UseHDR;
    bool DeferredContext;
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
    IDXGISwapChain1 *gSwapchain;
    ID3D11CommandList* pCommandList = NULL;
    IDXGIOutput *gOutput;

    // RTVs
    ID3D11RenderTargetView *gRTV;
    ID3D11DepthStencilView *gDSV; ID3D11ShaderResourceView *gDSV_SRV;

    // Textures
    ID3D11Texture2D *gDSVTex;

    // States
    ID3D11DepthStencilState *pDSS_Default, *pDSS_Default_NoDepthWrite, *pDSS_Default_InvDepth;
    ID3D11RasterizerState *gRSDefault, *gRSDefaultWriteframe;

    // CFG
    DirectXConfig cfg;

    // Descs
    D3D11_DEPTH_STENCIL_VIEW_DESC pDesc2;
    D3D11_TEXTURE2D_DESC pTex2DDesc;
    D3D11_DEPTH_STENCIL_DESC pDSD;
    DXGI_SWAP_CHAIN_DESC1 scd;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC pSCFDesc;

public:
    // Globals
    // 3D Rendering
    ID3D11Device         *gDevice;
    ID3D11DeviceContext  *gContext;

    // 11.1
    ID3D11Device1        *gDeviceN;
    ID3D11DeviceContext1 *gContextN;
    
    // Deferred context
    ID3D11DeviceContext1 *gContextImm;

#ifdef _DEBUG
    ID3D11Debug *gDebug;
#endif

    // Game state
    bool gPause, gAnselSessionIsActive;
    DirectX::XMMATRIX gViewBackup, gView;

    _DirectX();
    
    // Setup DirectX
    int Create(const DirectXConfig& config);
    bool ShowError(int id);

    const DirectXConfig& GetConfig();

    // 
    bool FrameFunction();
    void Tick(float fDeltaTime);
    void ComposeUI();
    void Resize();
    void Load();
    void Unload();

    void AnselEnable(DirectX::XMMATRIX view);
    void AnselSession();

    friend class HighLevel;
};
