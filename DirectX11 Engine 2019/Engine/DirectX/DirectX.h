#pragma once

// 3D Rendering (primarely)
#include <d3d11_4.h>
#include <dxgi1_6.h>
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
    // HDR / WCG
    bool gUseHDR; // Is HDR Supported by user's windows version?
    DXGI_OUTPUT_DESC1 pDescHDR;

    // Main
    IDXGISwapChain1 *gSwapchain;
    IDXGISwapChain4 *gSwapchain4;
    ID3D11CommandList* pCommandList = NULL;
    IDXGIOutput  *gOutput;
    IDXGIOutput6 *gOutput6;

    IDXGIAdapter3 *gAdapter3 = nullptr;

    // RTVs
    ID3D11RenderTargetView *gRTV; ID3D11ShaderResourceView *gRTV_SRV;
    ID3D11DepthStencilView *gDSV; ID3D11ShaderResourceView *gDSV_SRV;

    // Textures
    ID3D11Texture2D *gDSVTex;

    // States
    ID3D11DepthStencilState *pDSS_Default, *pDSS_Default_NoDepthWrite, *pDSS_Default_InvDepth;
    ID3D11RasterizerState *gRSDefault, *gRSDefaultWriteframe;

    // CFG
    DirectXConfig cfg;

    // Descs
    D3D11_DEPTH_STENCIL_VIEW_DESC   pDesc2;
    D3D11_TEXTURE2D_DESC            pTex2DDesc;
    D3D11_DEPTH_STENCIL_DESC        pDSD;
    DXGI_SWAP_CHAIN_DESC1           scd;
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
    ~_DirectX() {
        if( gAdapter3   ) gAdapter3->Release();
        if( gOutput6    ) gOutput6->Release();
        if( gOutput     ) gOutput->Release();
        if( gSwapchain4 ) gSwapchain4->Release();
    };
    
    // Setup DirectX
    int Create(const DirectXConfig& config);
    bool ShowError(int id);

    const DirectXConfig& GetConfig();

    DXGI_QUERY_VIDEO_MEMORY_INFO GPUUsage() const {
        DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
        HRESULT hr = gAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
        return info;
    }

    DXGI_QUERY_VIDEO_MEMORY_INFO CPUUsage() const {
        DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
        HRESULT hr = gAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &info);
        return info;
    }

    UINT64 GPUUsageCurrent()     const { return GPUUsage().CurrentUsage; }
    UINT64 GPUUsageAvaliable()   const { return GPUUsage().AvailableForReservation; }
    UINT64 GPUUsageBudget()      const { return GPUUsage().Budget; }
    UINT64 GPUUsageReservation() const { return GPUUsage().CurrentReservation; }

    UINT64 CPUUsageCurrent()     const { return CPUUsage().CurrentUsage; }
    UINT64 CPUUsageAvaliable()   const { return CPUUsage().AvailableForReservation; }
    UINT64 CPUUsageBudget()      const { return CPUUsage().Budget; }
    UINT64 CPUUsageReservation() const { return CPUUsage().CurrentReservation; }

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
