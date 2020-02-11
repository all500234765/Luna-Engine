#include "pc.h"
#include "Other/CPUID.h"

// Extensions
#include "Engine/Extensions/Default.h"
#include "Engine/Input/Gamepad.h"

#include "Engine Includes/MainInclude.h"
#include "HighLevel/DirectX/HighLevel.h"

#include "Renderer/RendererDeferred.h"

#include "Engine/Window/SplashScreen.h"

#include "Engine Includes/UI.h"

HighLevel gHighLevel;
RendererBase *gRenderer;
Scene *gMainScene;

Shader *shSkybox{};

bool g_bMouseHUD{};
float g_fAvgMS{};

EntityHandle gTestLight;

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
    dxCFG.UseHDR = false;
    dxCFG.DeferredContext = false;
    dxCFG.Windowed = winCFG.Windowed;
    dxCFG.Ansel = USE_ANSEL;

    // Init DirectX
    gDirectX = gHighLevel.InitDirectX(dxCFG, true);

    // Main Loop
    gHighLevel.AppLoop();

    // 
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

    // GUI Render
    UIManager::Clear();

    if( true )
    {
        float2 Pos = { 261.f, 146.5f };
        float2 Size = { 844.f, 475.f };

        float TopBarHeight = 64.f;

        //UIPrimitive::SetColor({ 1.f, 0.f, 0.f, 1.f });
        UIPrimitive::SetColor({ .8f, .8f, .8f, .75f });
        UIRoundrect RrR(64.f, 64.f, 64.f * 2, 64.f * 2);

        UIPrimitive::SetColor({ 1.f, 0.f, 0.f, 1.f });
        UIRoundrect RrR2(2.f * 64.f, 64.f, 64.f * 3.f, 64.f * 2.f, { 16.f, 0.f, 16.f, 0.f });

        UIPrimitive::SetColor({ 1.f, 0.f, 0.f, .75f });
        UIRoundrect RrR3(3.f * 64.f, 64.f, 64.f * 4.f, 64.f * 2.f, { 16.f, 0.f, 16.f, 0.f });

        {
            UIContainer cont(Pos, Size);

            UIPrimitive::SetColor({ 1.f, 0.f, 0.f, 1.f });
            UIRectangle b({ 0.f, 0.f }, Size);

            // Top bar
            {
                UIContainer cont1(0.f, 0.f, Size.x, TopBarHeight);

                UIPrimitive::SetColor({ 0.f, 0.f, 0.f, 1.f });
                UIRectangle c(0.f, 0.f, 56.f, 64.f);
                UIRectangle d(732.f + 56.f, 0.f, 732.f + 56.f + 56.f, 64.f);

                UIPrimitive::SetColor(LunaEngine::Math::normrgba({ 45.f, 45.f, 45.f, 255.f }));
                UIRectangle e(56.f, 0.f, 56.f + 732.f, 64.f);
            }

            // Content
            {
                UIContainer cont3(0.f, 64.f, 844.f, 384.f);
                {
                    UIScrollbar vsb(UIScrollbarType::Vertical);

                    float4 Colors[2] = {
                        LunaEngine::Math::normrgba({ 29.f, 29.f, 29.f, 255.f }),
                        LunaEngine::Math::normrgba({ 45.f, 45.f, 45.f, 255.f })
                    };

                    // 
                    const int num = 9;
                    for( uint i = 0; i < num; i++ ) {
                        UIPrimitive::SetColor(Colors[i % 2]);
                        UIRectangle rect(0.f, i * 48.f, 844.f, (i + 1.f) * 48.f);
                    }

                    UIPrimitive::SetColor(LunaEngine::Math::normrgba({ 190.f, 240.f, 0.f, 255.f }));
                    UIRectangle rect(0.f, num * 48.f, 844.f, (num + 1.f) * 48.f);
                }
            }

            // Bottom bar
            {
                UIContainer cont2(gKeyboard->IsDown(VK_G) * 10.f + 0.f, 448.f, 844.f, 27.f);
                //UIPrimitive::SetColor(LunaEngine::Math::normrgba({ 29.f, 29.f, 29.f, 255.f }));
                UIPrimitive::SetColor(LunaEngine::Math::normrgba({ 97.f, 97.f, 97.f, 255.f }));
                UIRectangle f(0.f, 0.f, 844.f, 64.f);
            }
        }
    }

    static bool fLaG = false;
    fLaG ^= gKeyboard->IsPressed(VK_H);
    UIManager::Submit();
    UIManager::Render(fLaG);

    // Render to screen
    gContext->OMSetRenderTargets(1, &gRTV, nullptr);
    UIManager::Screen();

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
    //if( gKeyboard->IsPressed(VK_F10) ) { gHighLevel.RenderDocLaunchUI(); }
    //if( gHighLevel.RenderDocGetUI() ) return;
    
    static uint64_t g_iTickFrame = 0;
    if( (g_iTickFrame % 240) == 0 ) {
        g_fAvgMS /= 240.f;
    } else {
        g_fAvgMS += fDeltaTime;
    }

    {
        PointLightComponent* p = gMainScene->GetComponent<PointLightComponent>(gTestLight);
        p->_LightPosition.x += cosf(fDeltaTime);
        p->_LightPosition.z += sinf(fDeltaTime);
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

        TransformComponent *ts = gMainScene->GetCamera(0)->cTransf;
        gMainScene->WorldLight(ts->vPosition, ts->vRotation, { .7f, .6f, .3f });
    }

    if( !g_bMouseHUD ) {
        // Clamp camera pitch
        TransformComponent *Transform = gMainScene->GetCamera(0)->cTransf;
        Transform->vRotation.x = LunaEngine::Math::clamp(Transform->vRotation.x, -84.f, 84.f);

        // Set mouse at center of the screen
        RECT rect = gWindow->GetRect();
        WindowConfig wcfg = gWindow->GetCFG();
        float ww = (float)(wcfg.CurrentWidth);
        float wh = (float)(wcfg.CurrentHeight);

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
    //UIManager::Resize();

    // Resize camera
    CameraComponent *cam = gMainScene->GetCamera(0)->cCam;
    cam->fWidth = (float)(wcfg.CurrentWidth);
    cam->fHeight = (float)(wcfg.CurrentHeight);
    cam->fAspect = (float)(wcfg.CurrentWidth / wcfg.CurrentHeight);

    // Set mouse at center of the screen
    RECT rect = gWindow->GetRect();
    float ww = (float)(wcfg.CurrentWidth);
    float wh = (float)(wcfg.CurrentHeight);

    gMouse->SetAt(rect.left + ww * .5f, rect.top + wh * .5f, true);
}

void _DirectX::PostCreateResources(bool Recreated) {};

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
    
    // Add lights
    {
        // Static
        PointLightBuff light{};
        light._LightColor    = float3(.7f, .9f, 0.f);
        light._LightPosition = float3(0.f, 100.f, 0.f);
        light._LightPower    = 1.f;
        light._LightRadius   = 128.f;
        gMainScene->InsertStaticPointLight(light);
        
        // Dynamic
        light._LightColor    = float3(.4f, .3f, .6f);
        light._LightPosition = float3(0.f, 100.f, 0.f);
        light._LightPower    = 1.f;
        light._LightRadius   = 25.f;
        gTestLight = gMainScene->InsertDynamicPointLight(light);
    }

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

    /*gMainScene->LoadModelStaticOpaque("../Models/SponzaRed/SponzaRed.obj", 
                                        //"../Models/TestSceneCurve.obj", 
                                      0u, [](EntityHandle e, uint32_t index) {
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

        mesh->mInstanceCount = 4;* /
    });*/

    /*gMainScene->LoadModelStaticOpaque("../Models/Sponza/sponza.obj", 
                                      //"../Models/cube.obj",
                                      0u, [](EntityHandle e, uint32_t index) {
        TransformComponent *transf = gMainScene->GetComponent<TransformComponent>(e);
        MaterialComponent *mat     = gMainScene->GetComponent<MaterialComponent>(e);
        MeshStaticComponent *mesh  = gMainScene->GetComponent<MeshStaticComponent>(e);

        transf->vPosition = { 0.f,  0.f, 0.f };
        transf->vRotation = { 0.f,  0.f, 0.f };
        transf->vScale    = float3(.0625f, .0625f, .0625f);
        //transf->vScale    = float3(500.f, 500.f, 500.f);
        transf->Build();

        //mat->_Alb       = true;
        //mat->_AlbedoTex = new Texture("../Textures/DarkPixel.png", 0u, "Black plane");
    });*/

    EntityHandleList elist = gMainScene->LoadModelStaticOpaque(//"../Models/Sketchfab/ch basement a/scene.gltf", 
                                                               "../Models/VolumetricTests/Volumetric Test.obj", 
                                                               //"../Models/Sponza/SponzaPBR.gltf",
                                                               //"../Models/SDKMesh/TankScene.gltf",
                                                               aiProcess_FlipUVs, [](EntityHandle e, uint32_t index) {
        TransformComponent *transf = gMainScene->GetComponent<TransformComponent>(e);
        MaterialComponent *mat = gMainScene->GetComponent<MaterialComponent>(e);
        MeshStaticComponent *mesh = gMainScene->GetComponent<MeshStaticComponent>(e);

        //transf->vRotation = float3(270.f, 0.f, 0.f);
        transf->vRotation = float3(90.f, 0.f, 0.f); // Sponza, TankScene
        //transf->vScale = float3(100.f, 100.f, 100.f);
        transf->vScale = float3(10.f, 10.f, 10.f); // TankScene
        //transf->vScale    = float3(.125, .125, .125);
        //transf->vPosition = float3(-50.f, 0.f, 50.f);

        //transf->vPosition = { 0.f, 0.f, 0.f };
        //transf->vRotation = { 0.f, 0.f, 0.f };
        //transf->vScale = float3(16.f, 16.f, 16.f);
        transf->Build();
    });

    auto Move = [](EntityHandleList list) {
        for( EntityHandle e : list ) {
            TransformComponent *transf = gMainScene->GetComponent<TransformComponent>(e);

            transf->vPosition.z += 50.f * 16.f / 20.f;
            transf->Build();
        }

        return list;
    };

    // Instantiate single model several times
    /*elist = Move(gMainScene->Instantiate(elist));
    elist = Move(gMainScene->Instantiate(elist));
    elist = Move(gMainScene->Instantiate(elist));
    elist = Move(gMainScene->Instantiate(elist));*/

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
        //mat->_AlbedoTex = new Texture("../Textures/DarkPixel.png", 0u, "Env Cubemap");

    });

    // TODO: Try DefaultTexture.png
}

void _DirectX::InitGameData() {
    gRenderer = new RendererDeferred();
    
    gMainScene = new Scene();
    gMainScene->SetAsActive(); // Bind current scene as active

    // Create input controller for player camera
    float fSpeed = 4.f*50.f;
    MovementControlComponent lMovementControlComp{};
    lMovementControlComp.mAssignedControls = {
        InputControl(VK_A, GamepadButtonState::_StickL).SetValue(0.f, 0.f, -fSpeed).OrientationDependent(),
        InputControl(VK_D, GamepadButtonState::_StickL).SetValue(0.f, 0.f, +fSpeed).OrientationDependent(),
        InputControl(VK_W, GamepadButtonState::_StickL).SetValue(+fSpeed).OrientationDependent(),
        InputControl(VK_S, GamepadButtonState::_StickL).SetValue(-fSpeed).OrientationDependent(),
        InputControl(MouseButton::AxisXY).SetValue(1.f, 1.f).OrientationUpdate()
    };

    // Create cameras
    gMainScene->MakeCameraFOVH(0, .2f, 10000.f, (float)(gRenderer->Width()), (float)(gRenderer->Height()), 70.f); // Player
    gMainScene->MakeCameraFOVH(1, .2f, 10000.f, 2048.f, 2048.f, 70.f); // Light
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
    SAFE_RELEASE_RENDERER(RendererDeferred, gRenderer);
    SAFE_DELETE(gMainScene);
}

