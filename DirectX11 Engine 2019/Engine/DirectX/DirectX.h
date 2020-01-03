#pragma once

// 3D Rendering (primarely)
#include "pc.h"

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
    friend class RendererBase;
    friend class RendererDeferred;

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

    void Load() {
        InitGameData();
        CreateResources();
        PostCreateResources(false);
    }

    // Game tick function
    void Tick(float fDeltaTime);
    
    // Render frame
    bool Render();
    
    // Render UI; maybe soon deprecated
    void ComposeUI();

    // Handle resize event
    void Resize();
    
    // Game initialization
    void InitGameData();
    void CreateResources();
    void PostCreateResources(bool Recreated=false);

    // Game unloading
    void FreeResources();
    void Unload();

    void Present(UINT SyncInterval, UINT Flags) {
        // If the device was removed either by a disconnect or a driver upgrade, we 
        // must recreate all device resources.
        if( gSwapchain->Present(SyncInterval, Flags) == DXGI_ERROR_DEVICE_REMOVED ) {
            printf_s("[DirectX]: Device was removed or drivers were updated.\n");
            printf_s("[DirectX]: Re-creating all device resources.\n");

            // Re-create resources
            FreeResources();
            CreateResources();
            PostCreateResources(true);

            // 
            printf_s("[DirectX]: Done\n");
        }
    }

    void AnselEnable(DirectX::XMMATRIX view);
    void AnselSession();

    friend class HighLevel;
};
