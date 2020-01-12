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

Shader *shSkybox{};

bool g_bMouseHUD{};
float g_fAvgMS{};

int WINAPI WINMAIN(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPCMDLINE lpCmdLine, int       snShowCmd) {
    // Create, redirect IO to, and hide console
    if( !GetConsoleWindow() ) {
        AllocConsole();
        freopen("CONOUT$", "wt", stdout);
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    // Show splashscreen
    //SplashScreen::Launch(L"Engine/SplashEditor2.bmp", 1000);

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

bool _DirectX::Render() {
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

    // Debug
    gRenderer->DebugHUD();
    gRenderer->ImGui();

    // Debug frame statistics
    if( (gRenderFrame % 240) == 0 ) {
        printf_s("Frame=%u(%.4fms; %ffps); Drawcalls=%u; Instances=%u; Dispatches=%u;\n"
                 "Render stats: \n"
                 "\t- Transparent %u\n"
                 "\t- Opaque %u\n",
                 gRenderFrame, g_fAvgMS * 1000.f, 1.f / g_fAvgMS, 
                 gDrawCallCount, gDrawCallInstanceCount, gDispatchCallCount, 
                 gRenderer->GetTransparencyAmount(), gRenderer->GetOpaqueAmount());

        g_fAvgMS = 0.f;
    }

    // Handle present event
    Present(1, 0);

    // End of frame
    gRenderFrame++;
    return false;
}

void _DirectX::Tick(float fDeltaTime) {
    static uint64_t g_iTickFrame = 0;
    if( (g_iTickFrame % 240) == 0 ) {
        g_fAvgMS /= 240.f;
    } else {
        g_fAvgMS += fDeltaTime;
    }

    gMainScene->Update(fDeltaTime);

    if( gKeyboard->IsPressed(VK_F2) ) {
        g_bMouseHUD ^= true;
        MovementControlComponent *comp = gMainScene->GetComponent<MovementControlComponent>(gMainScene->GetActiveCameraHandle());
        for( InputControl& c : comp->mAssignedControls ) {
            c.bDisabled ^= true;
        }
    }

    // Set world light position & direction
    if( gKeyboard->IsPressed(VK_SPACE) ) {
        gMainScene->UpdateCameraData(1);

        TransformComponent *td = gMainScene->GetCamera(1)->cTransf;
        TransformComponent *ts = gMainScene->GetCamera(0)->cTransf;
        td->vPosition = ts->vPosition;
        td->vRotation = ts->vRotation;

        gMainScene->GetCamera(1)->BuildView();
    }

    if( !g_bMouseHUD ) {
        // Clamp camera pitch
        TransformComponent *Transform = gMainScene->GetCamera(0)->cTransf;
        Transform->vRotation.x = LunaEngine::Math::clamp(Transform->vRotation.x, -84.f, 84.f);

        // Set mouse at center of the screen
        RECT rect = gWindow->GetRect();
        WindowConfig wcfg = gWindow->GetCFG();
        float ww = wcfg.CurrentWidth;
        float wh = wcfg.CurrentHeight;

        gMouse->SetAt(rect.left + ww * .5f, rect.top + wh * .5f, true);
    } else {

    }

    g_iTickFrame++;
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

void _DirectX::PostCreateResources(bool Recreated) {};

struct cbSDFDims {
    uint _VertexCount;
    uint _IndexCount;
    uint _Width;
    uint _Height;
    
    uint _Depth;
    uint _Spread;
    uint2 _Align;
};

void _DirectX::CreateResources() {
    // Create renderer's resources
    gRenderer->Init();

    // Load Terrain shaders
    shSkybox = new Shader();
    shSkybox->SetLayoutGenerator(LgMesh);
    shSkybox->LoadFile("shSkyboxVS.cso", Shader::Vertex);
    shSkybox->LoadFile("shSkyboxPS.cso", Shader::Pixel);

    //shSDFGen.ReleaseBlobs();
    shSkybox->ReleaseBlobs();

    // Add models
    gMainScene->SetSkybox("../Textures/Cubemap default.dds");

    /*gMainScene->LoadModelStaticOpaque("../Models/OpacityTest.obj",
                                      [](EntityHandle e, uint32_t index) {
        TransformComponent *transf = gMainScene->GetComponent<TransformComponent>(e);
        MaterialComponent *mat     = gMainScene->GetComponent<MaterialComponent>(e);

        //transf->vScale = float3(5.f, 5.f, 5.f);
        transf->vScale = float3(3.f, 3.f, 3.f);

        //transf->vRotation = float3(DirectX::XMConvertToRadians(270.f), 0.f, 0.f);
        //transf->vScale    = float3(.125, .125, .125);
        //transf->vPosition = float3(-50.f, 0.f, 50.f);
        transf->fAcceleration = 0.f;
        transf->fVelocity = 0.f;
        transf->vDirection = float3(0.f, 0.f, 0.f);

        transf->Build();

        mat->_Norm = 1.f;
    });*/


    /*gMainScene->LoadModelStaticOpaque("../Models/teapot.obj", 0u,
                                      [](EntityHandle e, uint32_t index) {
        TransformComponent *transf = gMainScene->GetComponent<TransformComponent>(e);
        MaterialComponent *mat = gMainScene->GetComponent<MaterialComponent>(e);
        MeshStaticComponent *mesh = gMainScene->GetComponent<MeshStaticComponent>(e);


        transf->vScale = float3(1.f, 1.f, 1.f);
        //transf->vScale = float3(5.f, 5.f, 5.f);
        transf->Build();
    });*/

    gMainScene->LoadModelStaticOpaque("../Models/SponzaRed/SponzaRed.obj", 0u, 
                                                   [](EntityHandle e, uint32_t index) {
        TransformComponent *transf = gMainScene->GetComponent<TransformComponent>(e);
        MaterialComponent *mat     = gMainScene->GetComponent<MaterialComponent>(e);
        MeshStaticComponent *mesh  = gMainScene->GetComponent<MeshStaticComponent>(e);

        transf->vPosition = { 0.f, 0.f, 0.f };
        transf->vRotation = { 0.f, 0.f, 0.f };
        //transf->vScale = float3(.0625f, .0625f, .0625f);
        transf->vScale = float3(5.f, 5.f, 5.f);
        transf->Build();

        //mat->_Alpha = .5f;
        //mat->_IsTransparent = 1.f;



        mat->_AlbedoMul = 1.f;
        //mat->_Alb = 1.f;
        //mat->_AlbedoTex = gRenderer->GetTexture(RendererDeferred::TextureList::Checkboard);

        /*mat->_Heightmap    = 1.f;
        mat->_HeightmapMul = 1.f;
        mat->_HeightmapTex = new Texture("../Textures/Heightmap.dds", 0u, "Heightmap");

        mat->_ShaderDepth      = shTerrainDepth;
        mat->_Shader           = shTerrain;
        mat->_MatDrawCallType  = DXDRAWINDEXED;//DXDRAWINDEXEDINSTANCED;
        mat->_MatBindingShader = Shader::Domain << (32 - Shader::Count);
        mat->_MatTopology      = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

        mesh->mInstanceCount = 4;*/
    });

    gMainScene->LoadModelStaticOpaque("../Models/UVMappedUnitSphere.obj", 0u, 
                                      [](EntityHandle e, uint32_t index) {
        TransformComponent *transf = gMainScene->GetComponent<TransformComponent>(e);
        MaterialComponent *mat = gMainScene->GetComponent<MaterialComponent>(e);

        transf->vPosition = { 0.f, 0.f, 0.f };
        transf->vRotation = { -90.f, 0.f, 0.f };
        transf->vScale = float3(10000.f, 10000.f, 10000.f);
        transf->Build();

        mat->_ShadowCaster = 0.f;
        mat->_ShadowReceiver = 0.f;

        mat->_Shader = shSkybox;

        //mat->_Alb = true;
        //mat->_AlbedoTex = new Texture("../Textures/Cubemap default.dds", 0u, "Env Cubemap");

    });

    // TODO: Try DefaultTexture.png
}

void _DirectX::InitGameData() {
    gRenderer = new RendererDeferred();
    
    gMainScene = new Scene();
    gMainScene->SetAsActive(); // Bind current scene as active

    // Create input controller for player camera
    float fSpeed = 50.f;
    MovementControlComponent lMovementControlComp{};
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
    //gMainScene->AmbientLight({ .4f, .5f, .8f }, .1f);

    // Add controls to main camera
    gMainScene->AddComponent(gMainScene->GetActiveCameraHandle(), &lMovementControlComp);

}

void _DirectX::FreeResources() {
    // Cleanup
    SAFE_RELEASE(shSkybox);

    gRenderer->Release();

    // Release component resources

    // TODO: Must!
    //gMainScene->ReleaseResources();
}

void _DirectX::Unload() {
    SAFE_RELEASE((RendererDeferred*)gRenderer);
    SAFE_DELETE(gMainScene);
}

