#include "pc.h"
#include "Other/CPUID.h"

// Extensions
#include "Engine/Extensions/Default.h"
#include "Engine/Input/Gamepad.h"

#include "Engine Includes/MainInclude.h"
#include "HighLevel/DirectX/HighLevel.h"

#include "Renderer/RendererDeferred.h"

#include "Engine/Window/SplashScreen.h"

HighLevel gHighLevel;

RendererBase *gRenderer;
Scene *gMainScene;

int WINAPI WINMAIN(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPCMDLINE lpCmdLine, int       snShowCmd) {
    // Create, redirect IO to, and hide console
    if( !GetConsoleWindow() ) {
        AllocConsole();
        freopen("CONOUT$", "wt", stdout);
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    // Show splashscreen
    SplashScreen::Launch(L"Engine/SplashEditor2.bmp", 1500);

    // Print CPU info
    CPUID cpu;
    cpu.PrintBasicInfo();

    // Window config
    WindowConfig winCFG;
    winCFG.Borderless = false;
    winCFG.Windowed = true;
    winCFG.ShowConsole = true;
    winCFG.Width = 1366;
    winCFG.Height = 768;
    winCFG.Title = L"Scene example - Luna Engine";
    winCFG.Icon = L"Engine/Luna150.ico";

    // Create window
    gWindow = gHighLevel.InitWindow(winCFG);

    // Get input devices
    gInput = gHighLevel.InitInput();
    gKeyboard = gInput->GetKeyboard();
    gMouse = gInput->GetMouse();

    for( int i = 0; i < NUM_GAMEPAD; i++ ) gGamepad[i] = gInput->GetGamepad(i);

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
    static uint gRenderFrame = 0;

    // Reset counters
    gDrawCallInstanceCount = 0;
    gDispatchCallCount = 0;
    gDrawCallCount = 0;

    // Resize event
    Resize();

    // Bind and clear RTV
    gRenderer->ClearMainRT();

    // Render world
    gRenderer->Render();

    // Render to screen
    gContext->OMSetRenderTargets(1, &gRTV, gDSV);
    gRenderer->FinalScreen();

    // Debug frame statistics
    if( (gRenderFrame % 120) == 0 ) {
        printf_s("Frame=%u; Drawcalls=%u; Instances=%u; Dispatches=%u\n", 
                 gRenderFrame, gDrawCallCount, gDrawCallInstanceCount, gDispatchCallCount);
    }

    // End of frame
    gSwapchain->Present(1, 0);
    gRenderFrame++;
    return false;
}

void _DirectX::Tick(float fDeltaTime) {
    gMainScene->Update(fDeltaTime);

    // Set world light position & direction
    if( gKeyboard->IsPressed(VK_SPACE) ) {
        gMainScene->UpdateCameraData(1);

        TransformComponent *td = gMainScene->GetCamera(1)->cTransf;
        TransformComponent *ts = gMainScene->GetCamera(0)->cTransf;
        td->vPosition = ts->vPosition;
        td->vRotation = ts->vRotation;

        gMainScene->GetCamera(1)->BuildView();
    }

    // Clamp camera pitch
    TransformComponent *Transform = gMainScene->GetCamera(0)->cTransf;
    Transform->vRotation.x = LunaEngine::Math::clamp(Transform->vRotation.x, -84.f, 84.f);

    // Set mouse at center of the screen
    RECT rect = gWindow->GetRect();
    WindowConfig wcfg = gWindow->GetCFG();
    float ww = wcfg.CurrentWidth;
    float wh = wcfg.CurrentHeight;

    gMouse->SetAt(rect.left + ww * .5f, rect.top + wh * .5f, true);
}

void _DirectX::Resize() {
    WindowConfig wcfg = gHighLevel.DefaultResize();
    if( !wcfg.Resized ) return;

    // Resize renderer
    gRenderer->Resize();

    // Resize camera
    CameraComponent *cam = gMainScene->GetCamera(0)->cCam;
    cam->fWidth = wcfg.CurrentWidth;
    cam->fHeight = wcfg.CurrentHeight;
    cam->fAspect = wcfg.CurrentWidth / wcfg.CurrentHeight;

    // Set mouse at center of the screen
    RECT rect = gWindow->GetRect();
    float ww = wcfg.CurrentWidth;
    float wh = wcfg.CurrentHeight;

    gMouse->SetAt(rect.left + ww * .5f, rect.top + wh * .5f, true);
}

void _DirectX::Load() {
    gRenderer = new RendererDeferred();
    gRenderer->Init();
    
    gMainScene = new Scene();
    gMainScene->SetAsActive(); // Bind current scene as active

    // Create input controller for player camera
    float fSpeed = 50.f;
    MovementControlComponent lMovementControlComp;
    lMovementControlComp.mAssignedControls = {
        InputControl(VK_A, GamepadButtonState::_StickL).SetValue(0.f, 0.f, -fSpeed).OrientationDependent(),
        InputControl(VK_D, GamepadButtonState::_StickL).SetValue(0.f, 0.f, +fSpeed).OrientationDependent(),
        InputControl(VK_W, GamepadButtonState::_StickL).SetValue(+fSpeed).OrientationDependent(),
        InputControl(VK_S, GamepadButtonState::_StickL).SetValue(-fSpeed).OrientationDependent(),
        InputControl(MouseButton::AxisXY).SetValue(1.f, 1.f).OrientationUpdate()
    };

    // Create cameras
    gMainScene->MakeCameraFOVH(0, .2f, 10000.f, gRenderer->Width(), gRenderer->Height(), 70.f); // Player
    gMainScene->MakeCameraFOVH(1, .2f, 10000.f, gRenderer->Width(), gRenderer->Height(), 70.f); // Light
    gMainScene->SetActiveCamera(0);
    gMainScene->UpdateMadeCameras();
    gMainScene->GetCamera(0)->cTransf->vPosition = float3(0.f, 10.f, 0.f);

    // Add controls to main camera
    gMainScene->AddComponent(gMainScene->GetActiveCameraHandle(), &lMovementControlComp);

    // Add model
    gMainScene->LoadModelStaticOpaque("../Models/LevelModelOBJ.obj", [](TransformComponent *transf) {
        transf->vRotation = float3(DirectX::XMConvertToRadians(270.f), 0.f, 0.f);
        transf->vScale    = float3(.125, .125, .125);
        transf->vPosition = float3(-50.f, 0.f, 50.f);
        transf->fAcceleration = 0.f;
        transf->fVelocity     = 0.f;
        transf->vDirection    = float3(0.f, 0.f, 0.f);

        transf->Build();
    });

    // TODO: Try DefaultTexture.png

}

void _DirectX::Unload() {
    SAFE_RELEASE(gRenderer);
    SAFE_DELETE(gMainScene);

}

