#pragma once

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Window/Window.h"
#include "../Audio Engine/Audio/AudioDeviceChild.h"
#include "Engine/Extensions/Safe.h"

//#include "Enums.h"
#include "Defines.h"

class HighLevel {
protected:


private:
    Window *gWindow;
    Input  *gInput;
    _DirectX *gDirectX;
    AudioDevice *gAudioDevice;

public:
    // int main() {
    Window* InitWindow(WindowConfig& cfg) {
        // Create global engine object
        gWindow = new Window();
        gWindow->Create(cfg);
        cfg = gWindow->GetCFG();
        return gWindow;
    }

    Input* InitInput() {
        gInput = gWindow->GetInputDevice();
        return gInput;
    }

    _DirectX* InitDirectX(DirectXConfig cfg) {
        gDirectX = new _DirectX();

        if( gDirectX->ShowError(gDirectX->Create(cfg)) ) { return nullptr; }

        // Debug report if we can
#ifdef _DEBUG
        if( gDirectX->gDebug ) gDirectX->gDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif

        // Set frame function
        bool(_DirectX::*gFrameFunction)(void);
        gFrameFunction = &_DirectX::FrameFunction;

        gWindow->SetFrameFunction(gFrameFunction); // Ref to function
        gWindow->SetDirectX(gDirectX);             // Ref to global object

        // Set directx object
        DirectXChild::SetDirectX(gDirectX);

        // Include ImGUI
#if _DEBUG_BUILD
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplWin32_Init(gWindow->GetHWND());
        ImGui_ImplDX11_Init(gDirectX->gDevice, gDirectX->gContext);
        ImGui::StyleColorsDark();
#endif

        // Return DirectX object
        return gDirectX;
    }

    AudioDevice* InitAudio(AudioDeviceConfig cfg) {
        gAudioDevice = new AudioDevice();

        // Create audio device
        gAudioDevice->Create(cfg);

        // Set audio object
        AudioDeviceChild::SetAudioDevice(gAudioDevice);

        return gAudioDevice;
    }

    void AppLoop() {
        // Load game data
        gDirectX->Load();

        // Start rendering loop
        gWindow->Loop();

        // Unload
        AppEnd();
    }

    void AppEnd() {
#if _DEBUG_BUILD
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
#endif

        gWindow->Destroy();
        gDirectX->Unload();
    }

    // }

    // 
    virtual WindowConfig DefaultResize() {
        WindowConfig cfg = gWindow->GetCFG();
        if( !cfg.Resized ) { return cfg; }                                     // Window isn't resized
        if( cfg.CurrentWidth <= 0 && cfg.CurrentHeight2 <= 0 ) { return cfg; } // Window was minimazed

        std::cout << "Window/DirectX resize event (w=" << cfg.CurrentWidth << ", h=" << cfg.CurrentHeight << ")" << std::endl;

        // Release targets
        gDirectX->gContext->OMSetRenderTargets(0, 0, 0);
        gDirectX->gRTV->Release();

        // Resize text port
        //gTextController->SetSize(cfg.CurrentWidth, cfg.CurrentHeight);

        // Resize swapchain
        DXGI_MODE_DESC pModeDesc = {};
        pModeDesc.Format           = gDirectX->scd.Format;
        pModeDesc.Width            = cfg.CurrentWidth;
        pModeDesc.Height           = cfg.CurrentHeight;
        pModeDesc.RefreshRate      = gDirectX->pSCFDesc.RefreshRate;
        pModeDesc.Scaling          = gDirectX->pSCFDesc.Scaling;
        pModeDesc.ScanlineOrdering = gDirectX->pSCFDesc.ScanlineOrdering;
        //pModeDesc.Stereo = scd.Stereo;

        gDirectX->gSwapchain->ResizeTarget(&pModeDesc);
        gDirectX->gSwapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

        // Create RTV
        ID3D11Texture2D *BackBufferColor;
        gDirectX->gSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBufferColor);

        gDirectX->gDevice->CreateRenderTargetView(BackBufferColor, NULL, &gDirectX->gRTV);
        BackBufferColor->Release(); // ?

        // Create depth texture
        gDirectX->pTex2DDesc.Width  = cfg.CurrentWidth;
        gDirectX->pTex2DDesc.Height = cfg.CurrentHeight;
        gDirectX->gDSVTex->Release();
        gDirectX->gDSV->Release();
        gDirectX->gDevice->CreateTexture2D(&gDirectX->pTex2DDesc, NULL, &gDirectX->gDSVTex);

        // Create depth stencil view
        gDirectX->gDevice->CreateDepthStencilView(gDirectX->gDSVTex, &gDirectX->pDesc2, &gDirectX->gDSV);

        // Bind default targets
        gDirectX->gContext->OMSetRenderTargets(1, &gDirectX->gRTV, gDirectX->gDSV);

        // 
        return cfg;
    }

    virtual void DefaultRenderBegin() {
        
    }

    virtual void DefaultRenderEnd() {
        
    }

    void SetTopology() {
        
    }

    void ClearRTV() {
        
    }

    void ClearDSV() {
        
    }

    // 
    friend _DirectX;
};
