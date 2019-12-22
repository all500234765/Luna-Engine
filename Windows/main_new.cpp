#include "Other/CPUID.h"

#undef _____________TYPE_FLOAT_OPERATORS22____3333
#include "Other/FloatTypeMath.h"

#include <array>
//#include <algorithm>

// Extensions
#include "Engine/Extensions/Default.h"
#include "Engine/Input/Gamepad.h"

#include "Engine Includes/MainInclude.h"
#include "HighLevel/DirectX/HighLevel.h"

#include "Renderer/RendererDeferred.h"

HighLevel gHighLevel;

RendererDeferred *gRenderer;
Scene *gMainScene;

int main() {
    // Window config
    WindowConfig winCFG;
    winCFG.Borderless = false;
    winCFG.Windowed = true;
    winCFG.ShowConsole = true;
    winCFG.Width = 1024;
    winCFG.Height = 540;
    winCFG.Title = L"Scene example - Luna Engine";
    winCFG.Icon = L"Engine/Assets/Engine.ico";

    // Create window
    gWindow = gHighLevel.InitWindow(winCFG);

    // Get input devices
    gInput = gHighLevel.InitInput();
    gKeyboard = gInput->GetKeyboard();
    gMouse = gInput->GetMouse();

#if USE_GAMEPADS
    for( int i = 0; i < NUM_GAMEPAD; i++ ) gGamepad[i] = gInput->GetGamepad(i);
#endif

    // Audio device config
    AudioDeviceConfig adCFG = { 0 };

    // Create audio device
    gAudioDevice = gHighLevel.InitAudio(adCFG);

    // DirectX config
    DirectXConfig dxCFG;
    dxCFG.BufferCount = 2;
    dxCFG.Width = winCFG.CurrentWidth;
    dxCFG.Height = winCFG.CurrentHeight2;
    dxCFG.m_hwnd = gWindow->GetHWND();
    dxCFG.RefreshRate = 60;
    dxCFG.UseHDR = true;
    dxCFG.DeferredContext = false;
    dxCFG.Windowed = winCFG.Windowed;
    dxCFG.Ansel = USE_ANSEL;

    // Init DirectX
    gDirectX = gHighLevel.InitDirectX(dxCFG);

    // Main Loop
    gHighLevel.AppLoop();

    // 
    SAFE_DELETE(gWindow);
    SAFE_DELETE(gDirectX);
    SAFE_DELETE_N(gGamepad, NUM_GAMEPAD);
    SAFE_DELETE(gInput);
    SAFE_RELEASE(gAudioDevice);
}

bool _DirectX::FrameFunction() {
    // Resize event
    Resize();

    // 


    // Bind and clear RTV
    gContext->OMSetRenderTargets(1, &gRTV, gDSV);
    gRenderer->ClearMainRT();

    gRenderer->Render();


    // End of frame
    gSwapchain->Present(1, 0);
    return false;
}


void _DirectX::Tick(float fDeltaTime) {

}

void _DirectX::Resize() {

}

void _DirectX::Load() {
    gRenderer = new RendererDeferred();
    gRenderer->Init();
    
    gMainScene = new Scene();
    gMainScene->SetAsActive();

    gMainScene->MakeCameraFOVH(0, .2f, 10000.f, gRenderer->Width(), gRenderer->Height(), 70.f);
    gMainScene->LoadModelStaticOpaque("../Models/LevelModelOBJ.obj", [](TransformComponent *transf) {
        transf->vRotation = float3(DirectX::XMConvertToRadians(270.f), 0.f, 0.f);
        transf->vScale    = float3(.125, .125, .125);
        transf->vPosition = float3(-50.f, 0.f, 50.f);
    });



}

void _DirectX::Unload() {
    SAFE_RELEASE(gRenderer);
    SAFE_DELETE(gMainScene);

}

