// Extensions
#include "Engine/Extensions/Default.h"

#include "EngineIncludes/MainInclude.h"

// Test instances
Camera *cPlayer, *c2DScreen, *cLight;
Shader *shTest, *shTerrain, *shSkeletalAnimations, *shGUI, *shVertexOnly;
Model *mModel1, *mModel2, *mScreenPlane, *mModel3;
ModelInstance *mLevel1, *mDunes, *mCornellBox;

Texture *tDefault;
Texture *tBlueNoiseRG;
DiffuseMap *mDefaultDiffuse;
Sampler *sPoint;

Material *mDefault;

RenderBufferDepth2D *bDepth;

CubemapTexture *pCubemap;

// HBAO+
#if USE_HBAO_PLUS
GFSDK_SSAO_CustomHeap CustomHeap;
GFSDK_SSAO_Context_D3D11* pAOContext;

GFSDK_SSAO_InputData_D3D11 Input;
GFSDK_SSAO_Parameters Params;
GFSDK_SSAO_Output_D3D11 Output;
#endif

int SceneID = 0;

// Define Frame Function
bool _DirectX::FrameFunction() {
    // Resize event
    Resize();
    
    // Bind and clear RTV
    gContext->OMSetRenderTargets(1, &gRTV, gDSV);

    float Clear[4] = {.2f, .2f, .2f, 1.f}; // RGBA
    gContext->ClearRenderTargetView(gRTV, Clear);
    gContext->ClearDepthStencilView(gDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

    // Ansel session
#if USE_ANSEL
    AnselSession();
#endif

    // Render scene and every thing else here
#if USE_ANSEL
    if( !gAnselSessionIsActive )
#endif
    {
        cPlayer->BuildView();
    }

    // Render depth buffer
    bDepth->Bind();
        
        gContext->ClearDepthStencilView(bDepth->GetTarget(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

        mLevel1->Bind(cLight);
        shVertexOnly->Bind();
        mLevel1->Render();
    
    // Render default
    gContext->OMSetRenderTargets(1, &gRTV, gDSV);

    mLevel1->Bind(cPlayer);
        
        // Bind material
        mDefault->BindTextures(Shader::Pixel);

        // Bind depth buffer
        bDepth->BindResources(Shader::Pixel, 1);
        sPoint->Bind(Shader::Pixel, 1);

        // Bind noise texture
        tBlueNoiseRG->Bind(Shader::Pixel, 2);
        sPoint->Bind(Shader::Pixel, 2);

        // Bind light buffer
        cLight->BindBuffer(Shader::Vertex, 1);

        // Render level
        mLevel1->Render();

    /*switch( SceneID ) {
        case 0: // Test level
            mLevel1->Bind(cPlayer);
            mDefault->BindTextures(Shader::Pixel);
            mLevel1->Render();
            break;

        case 1: // Dunes
            mDunes->Bind(cPlayer);
            mDunes->Render();
            break;

        case 2: // Cornell box
            mCornellBox->Bind(cPlayer);
            mCornellBox->Render();
            break;
    }*/

    // Set default topology
    gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // HBAO+
#if USE_HBAO_PLUS
    DirectX::XMFLOAT4X4 mProjDest;
    DirectX::XMStoreFloat4x4(&mProjDest, cPlayer->GetProjMatrix());
    Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4(&mProjDest(0, 0));

    GFSDK_SSAO_Status status = GFSDK_SSAO_OK;
    status = pAOContext->RenderAO(gContext, Input, Params, Output);
    assert(status == GFSDK_SSAO_OK);
#endif

    // 2D Rendering
    ComposeUI();

    // End of frame
    gSwapchain->Present(1, 0);
    return false;
}

void _DirectX::Tick(float fDeltaTime) {
    // Select scenes
    //if( gKeyboard->IsPressed(VK_0) ) { SceneID = 0; }
    //if( gKeyboard->IsPressed(VK_1) ) { SceneID = 1; }
    //if( gKeyboard->IsPressed(VK_2) ) { SceneID = 2; }

    if( gKeyboard->IsPressed(VK_SPACE) ) {
        // Set light's view matrix to math main camera's one
        cLight->SetViewMatrix(cPlayer->GetViewMatrix());
        cLight->BuildConstantBuffer();
    }

    // Update camera
    const float fSpeed = 20.f, fRotSpeed = 5.f, fSensetivityX = 2.f, fSensetivityY = 3.f;
    DirectX::XMFLOAT3 f3Move(0.f, 0.f, 0.f); // Movement vector

    float fDir = 0.f, fPitch = 0.f;             // 
    static DirectX::XMFLOAT2 pLastPos = {0, 0}; // Last mouse pos

    // Camera
    if( gKeyboard->IsDown(VK_W) ) f3Move.x = +fSpeed * fDeltaTime;  // Forward / Backward
    if( gKeyboard->IsDown(VK_S) ) f3Move.x = -fSpeed * fDeltaTime;
    if( gKeyboard->IsDown(VK_D) ) f3Move.z = +fSpeed * fDeltaTime;  // Strafe
    if( gKeyboard->IsDown(VK_A) ) f3Move.z = -fSpeed * fDeltaTime;

    if( gKeyboard->IsDown(VK_LEFT ) ) fDir = -fRotSpeed * fDeltaTime; // Right / Left 
    if( gKeyboard->IsDown(VK_RIGHT) ) fDir = +fRotSpeed * fDeltaTime;

    // I got used to KSP and other avia/space sims
    // So i flipped them
    if( gKeyboard->IsDown(VK_UP  ) ) fPitch = -fRotSpeed * fDeltaTime; // Look Up / Down
    if( gKeyboard->IsDown(VK_DOWN) ) fPitch = +fRotSpeed * fDeltaTime;

    // Move view around
    //if( gMouse->IsPressed(MouseButton::Left) ) {
    //    std::cout << gMouse->GetX() << " " << gMouse->GetY() << std::endl;
    //
    //    pLastPos = DirectX::XMFLOAT2(gMouse->GetX(), gMouse->GetY());
    //}
    //
    //if( gMouse->IsDown(MouseButton::Left) ) {
    //    fDir   = +(float(gMouse->GetX() - pLastPos.x) / 5.f);
    //    fPitch = +(float(gMouse->GetY() - pLastPos.y) / 10.f);
    //
    //    pLastPos = DirectX::XMFLOAT2(gMouse->GetX(), gMouse->GetY());
    //}

    fDir   = +(float(gMouse->GetX() - cfg.Width  * .5f) * fSensetivityX * fDeltaTime);
    fPitch = +(float(gMouse->GetY() - cfg.Height * .5f) * fSensetivityY * fDeltaTime);
    gMouse->SetAt(int(cfg.Width  * .5f), int(cfg.Height * .5f));

    cPlayer->TranslateLookAt(f3Move);
    cPlayer->Rotate(DirectX::XMFLOAT3(fPitch, fDir, 0.));
}

void _DirectX::ComposeUI() {
    //shGUI->Bind();



    //CreateDeviceResources();

    // Begin rendering
    /*g2DRT->BeginDraw();

    // Rendering code goes here
    // ...
    g2DRT->SetTransform(D2D1::IdentityMatrix());          // Reset transformation
    g2DRT->Clear(D2D1::ColorF(D2D1::ColorF::Black, 1.f)); // Clear text output

    // Draw text
    g2DRT->DrawTextLayout(D2D1::Point2F(5.f, 5.f), gTextLayout, gSolidLimeBrush);

    // End rendering
    g2DRT->EndDraw();*/

    // 
    //DiscardDeviceResources();
}

void _DirectX::Resize() {
    const WindowConfig& cfg = gWindow->GetCFG();
    if( !cfg.Resized ) { return; }                                     // Window isn't resized
    if( cfg.CurrentWidth <= 0 && cfg.CurrentHeight <= 0 ) { return;  } // Window was minimazed

    std::cout << "Window/DirectX resize event (w=" << cfg.CurrentWidth << ", h=" << cfg.CurrentHeight << ")" << std::endl;

    // Release targets
    gContext->OMSetRenderTargets(0, 0, 0);
    gRTV->Release();

    // Resize swapchain
    scd.BufferDesc.Width  = (UINT)cfg.CurrentWidth;
    scd.BufferDesc.Height = (UINT)cfg.CurrentHeight;

    gSwapchain->ResizeTarget(&scd.BufferDesc);

    std::cout << (gSwapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0) == S_OK) << std::endl;

    // Create RTV
    ID3D11Texture2D *BackBufferColor;
    gSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBufferColor);

    std::cout << (gDevice->CreateRenderTargetView(BackBufferColor, NULL, &gRTV) == S_OK) << std::endl;
    BackBufferColor->Release(); // ?

    // Create depth texture
    gDSVTex->Release();
    gDSV->Release();
    std::cout << (gDevice->CreateTexture2D(&pTex2DDesc, NULL, &gDSVTex) == S_OK) << std::endl;

    // Create depth stencil view
    std::cout << (gDevice->CreateDepthStencilView(gDSVTex, &pDesc2, &gDSV) == S_OK) << std::endl;

    // Bind default targets
    gContext->OMSetRenderTargets(1, &gRTV, gDSV);

    // Recalculate camer's aspect ratio and projection matrix
    CameraConfig cfg2 = cPlayer->GetParams();
    cfg2.fAspect = float(cfg.CurrentWidth) / float(cfg.CurrentHeight);
    cPlayer->BuildProj();

    cfg2 = c2DScreen->GetParams();
    cfg2.fAspect = float(cfg.CurrentWidth) / float(cfg.CurrentHeight);
    c2DScreen->BuildProj();

    // Set up the viewport
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(cfg.CurrentWidth);
    vp.Height = static_cast<float>(cfg.CurrentHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gContext->RSSetViewports(1, &vp);
}

void _DirectX::Load() {
    // Create instances
    shTest = new Shader();
    shTerrain = new Shader();
    shSkeletalAnimations = new Shader();
    shGUI = new Shader();
    shVertexOnly = new Shader();

    cPlayer = new Camera(DirectX::XMFLOAT3(0, 2, -2), DirectX::XMFLOAT3(0., 130., 0.));
    c2DScreen = new Camera();
    cLight = new Camera(DirectX::XMFLOAT3(-10.f, 10.f, -10.f), DirectX::XMFLOAT3(45.f, 130.f, 0.f));

    tDefault = new Texture();
    tBlueNoiseRG = new Texture();
    sPoint = new Sampler();
    mDefaultDiffuse = new DiffuseMap();
    mDefault = new Material();

    bDepth = new RenderBufferDepth2D();

    pCubemap = new CubemapTexture();

    // Ansel support
#if USE_ANSEL
    AnselEnable(cPlayer->GetViewMatrix());
#endif

    // Create cubemap
    pCubemap->CreateFromFiles("../Textures/Cubemaps/Test/", false, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Create depth buffer
    bDepth->Create(1024, 1024, 32);
    bDepth->SetName("Depth buffer");

    // Create default texture
    tDefault->Load("../Textures/TileInverse.png", DXGI_FORMAT_R8G8B8A8_UNORM);
    tDefault->SetName("Default tile texture");
    //gContext->GenerateMips(tDefault->GetSRV());

    // Create point sampler
    D3D11_SAMPLER_DESC pDesc;
    ZeroMemory(&pDesc, sizeof(D3D11_SAMPLER_DESC));
    pDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    pDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sPoint->Create(pDesc);
    sPoint->SetName("Point sampler");

    // Create maps
    mDefaultDiffuse->mTexture = tDefault;

    // Create materials
    mDefault->SetDiffuse(mDefaultDiffuse);
    mDefault->SetSampler(sPoint);
    mDefault->SetName("Default material");

    // Load blue noise texture
    tBlueNoiseRG->Load("../Textures/Noise/Blue/LDR_RG01_0.png", DXGI_FORMAT_R16G16_UNORM);
    
    // Setup camera
    const WindowConfig& cfg = gWindow->GetCFG();
    CameraConfig cfg2;
    cfg2.fAspect = float(cfg.CurrentWidth) / float(cfg.CurrentHeight);
    cfg2.FOV = 100.f;
    cfg2.fNear = .1f;
    cfg2.fFar = 300.f;
    cPlayer->SetParams(cfg2);
    cPlayer->BuildProj();
    cPlayer->BuildView();

    cfg2.FOV = 90.f;
    cfg2.fNear = .1f;
    cfg2.fFar = 1.f;
    c2DScreen->SetParams(cfg2);
    c2DScreen->BuildProj();
    c2DScreen->BuildView();

    cfg2.fNear = .1f;
    cfg2.fFar = 300.f;
    cfg2.FOV = 90.f;
    cfg2.fAspect = 1.f;
    cLight->SetParams(cfg2);
    cLight->BuildProj();
    cLight->BuildView();

    // Load shader
    shTest->LoadFile("../CompiledShaders/shTestVS.cso", Shader::Vertex);
    shTest->LoadFile("../CompiledShaders/shTestPS.cso", Shader::Pixel);

    // Don't forget to bind MatrixBuffer to Domain shader instead of Vertex
    shTerrain->LoadFile("../CompiledShaders/shTerrainVS.cso", Shader::Vertex);
    shTerrain->LoadFile("../CompiledShaders/shTerrainHS.cso", Shader::Hull);
    shTerrain->LoadFile("../CompiledShaders/shTerrainDS.cso", Shader::Domain);
    shTerrain->LoadFile("../CompiledShaders/shTerrainPS.cso", Shader::Pixel);

    // Skeletal animations
    shSkeletalAnimations->LoadFile("../CompiledShaders/shSkeletalAnimationsVS.cso", Shader::Vertex);
    shSkeletalAnimations->LoadFile("../CompiledShaders/shSkeletalAnimationsPS.cso", Shader::Pixel);

    // GUI
    shGUI->LoadFile("../CompiledShaders/shGUIVS.cso", Shader::Vertex);
    shGUI->LoadFile("../CompiledShaders/shGUIPS.cso", Shader::Pixel);
    
    // Vertex only shader
    shVertexOnly->LoadFile("../CompiledShaders/shSimpleVS.cso", Shader::Vertex);
    shVertexOnly->SetNullShader(Shader::Pixel);

    // Clean shaders
    shTest->ReleaseBlobs();
    shTerrain->ReleaseBlobs();
    shSkeletalAnimations->ReleaseBlobs();
    shGUI->ReleaseBlobs();
    shVertexOnly->ReleaseBlobs();

    // Create model
    mModel1 = new Model("Test model #1");
    mModel1->LoadModel("../Models/TestLevel1.obj");

    mModel2 = new Model("Test model #2");
    mModel2->LoadModel("../Models/Dunes1.obj");

    mModel3 = new Model("Test model #3");
    mModel3->LoadModel("../Models/cornellbox.obj");

    mScreenPlane = new Model("Screen plane model");
    //mScreenPlane->LoadModel("../Models/ScreenPlane.obj");

    // Create model instances
    // Test level
    mLevel1 = new ModelInstance();
    mLevel1->SetName("Level 1 Instance");
    mLevel1->SetWorldMatrix(DirectX::XMMatrixScaling(4, 4, 4));
    mLevel1->SetShader(shTest);
    mLevel1->SetModel(mModel1);

    // Dunes
    mDunes = new ModelInstance();
    mDunes->SetName("Dunes Instance");
    mDunes->SetWorldMatrix(DirectX::XMMatrixScaling(4, 4, 4));
    mDunes->SetShader(shTerrain);
    mDunes->SetModel(mModel2);
    mDunes->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    mDunes->SetBindBuffer(Shader::Domain);

    // Cornell box
    mCornellBox = new ModelInstance();
    mCornellBox->SetName("Dunes Instance");
    mCornellBox->SetWorldMatrix(DirectX::XMMatrixScaling(1, 1, 1));
    mCornellBox->SetShader(shTest);
    mCornellBox->SetModel(mModel3);

    // HBAO+
#if USE_HBAO_PLUS
    CustomHeap.new_ = ::operator new;
    CustomHeap.delete_ = ::operator delete;

    GFSDK_SSAO_Status status;
    status = GFSDK_SSAO_CreateContext_D3D11(gDevice, &pAOContext, &CustomHeap);
    assert(status == GFSDK_SSAO_OK); // HBAO+ requires feature level 11_0 or above

    Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
    Input.DepthData.pFullResDepthTextureSRV = gDSV_SRV;
    Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
    Input.DepthData.MetersToViewSpaceUnits = 1.;

    Params.Radius = 2.f;
    Params.Bias = 0.1f;
    Params.PowerExponent = 2.f;
    Params.Blur.Enable = true;
    Params.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
    Params.Blur.Sharpness = 16.f;

    Output.pRenderTargetView = gRTV;
    Output.Blend.Mode = GFSDK_SSAO_BlendMode::GFSDK_SSAO_MULTIPLY_RGB;
#endif

    // Debug report if we can
    if( gDirectX->gDebug ) gDirectX->gDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
}

void _DirectX::Unload() {
    // HBAO+
#if USE_HBAO_PLUS
    pAOContext->Release();
#endif

    // Release cubemaps
    pCubemap->Release();

    // Release materials
    mDefault->Release();

    // Release textures
    tBlueNoiseRG->Release();

    // Release shaders
    shTest->DeleteShaders();
    shTerrain->DeleteShaders();
    shGUI->DeleteShaders();
    shSkeletalAnimations->DeleteShaders();
    shVertexOnly->DeleteShaders();

    // Release model
    mModel1->Release();
    mModel2->Release();
    mModel3->Release();
    mScreenPlane->Release();

    // Release buffers
    bDepth->Release();
}

#if USE_ANSEL
void _DirectX::AnselEnable(DirectX::XMMATRIX view) {
    // Init AnselSDK
    if( !cfg.Ansel || !ansel::isAnselAvailable() ) { return; }
    ansel::Configuration cfg2;

    cfg2.translationalSpeedInWorldUnitsPerSecond = 10.f;
    cfg2.right = {1., 0., 0.};
    cfg2.forward = {0., 0., 1.};
    cfg2.up = {0., 1., 0.};
    cfg2.fovType = ansel::kVerticalFov;

    cfg2.isCameraFovSupported = true;
    cfg2.isCameraOffcenteredProjectionSupported = false;
    cfg2.isCameraRotationSupported = true;
    cfg2.isCameraTranslationSupported = true;

    cfg2.gameWindowHandle = cfg.m_hwnd;
    cfg2.titleNameUtf8 = u8"Best Gaem Ever Made~ ~heart~";

    // 
    gView = view;

    // Callbacks
    cfg2.startSessionCallback = [](ansel::SessionConfiguration& conf, void* userPointer) {
        UNREFERENCED_PARAMETER(userPointer);
        UNREFERENCED_PARAMETER(conf);
        gDirectX->gPause = true;
        gDirectX->gAnselSessionIsActive = true;
        gDirectX->gViewBackup = gDirectX->gView;
        return ansel::kAllowed;
    };

    cfg2.stopSessionCallback = [](void* userPointer) {
        UNREFERENCED_PARAMETER(userPointer);
        gDirectX->gPause = false;
        gDirectX->gAnselSessionIsActive = false;
        gDirectX->gView = gDirectX->gViewBackup;
        ShowCursor(true);
    };;

    cfg2.startCaptureCallback = [](const ansel::CaptureConfiguration&, void*) {
        // turn non-uniform full screen effects like vignette off here
    };
        
    cfg2.stopCaptureCallback = [](void*) {
        // turn disabled effects back on here
    };;

    cfg2.changeQualityCallback = [](bool isHighQuality, void*) {
        // boost or drop quality here depending on requested level
        //gHighQuality = isHighQuality;
    };

    ansel::SetConfigurationStatus status = ansel::setConfiguration(cfg2);
    if( status != ansel::kSetConfigurationSuccess ) {
        std::cout << "Ansel configuration has failed. Ansel isn't avaliable and disabled for further sessions." << std::endl;

        return; // Something wen't wrong, disable ansel
    }

    std::cout << "Ansel has been successfully configurated!" << std::endl;
}

void _DirectX::AnselSession() {
    if( !gAnselSessionIsActive || !cfg.Ansel ) { return; }
    CameraConfig camCFG = cPlayer->GetParams();
    DirectX::XMFLOAT3 p = cPlayer->GetPosition();
    DirectX::XMMATRIX v = cPlayer->GetViewMatrix();

    ansel::Camera cam;

    cam.fov = camCFG.FOV * 180.f / DirectX::XM_PI; // *180.f / DirectX::XM_PI;
    cam.aspectRatio = camCFG.fAspect;
    cam.farPlane = camCFG.fFar;
    cam.nearPlane = camCFG.fNear;
    
    // 
    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(nullptr, v);
    const nv::Vec3 right = {invView.r[0].m128_f32[0], invView.r[0].m128_f32[1], invView.r[0].m128_f32[2]};
    const nv::Vec3 up = {invView.r[1].m128_f32[0], invView.r[1].m128_f32[1], invView.r[1].m128_f32[2]};
    const nv::Vec3 forward = {invView.r[2].m128_f32[0], invView.r[2].m128_f32[1], invView.r[2].m128_f32[2]};
    ansel::rotationMatrixVectorsToQuaternion(right, up, forward, cam.rotation);
    cam.position = {invView.r[3].m128_f32[0], invView.r[3].m128_f32[1], invView.r[3].m128_f32[2]};

    // 
    ansel::updateCamera(cam);

    // Convert position and quaternion returned in ansel::updateCamera call and update camera parameters
    invView = DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f),
                                                    DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                                                    DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&cam.rotation)),
                                                    DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&cam.position)));
    camCFG.FOV = cam.fov;

    // Update in-game camera
    cPlayer->SetViewMatrix(XMMatrixInverse(nullptr, invView));
    cPlayer->SetParams(camCFG);
    cPlayer->BuildProj();
}
#endif

int main() {
    // Create global engine objects
    gWindow = new Window();
    gDirectX = new _DirectX();
    
    // Window config
    WindowConfig winCFG;
    winCFG.Borderless = false;
    winCFG.Windowed = true;
    winCFG.Width = 1024;
    winCFG.Height = 540;
    winCFG.Title = L"Luna Engine";
    winCFG.Icon = L"Engine/Assets/Engine.ico";
    
    // Create window
    gWindow->Create(&winCFG);

    // Get input devices
    gInput = gWindow->GetInputDevice();
    gKeyboard = gInput->GetKeyboard();
    gMouse = gInput->GetMouse();
#if USE_GAMEPAD
    for( int i = 0; i < NUM_GAMEPAD; i++ ) gGamepad[i] = gInput->GetGamepad(i);
#endif

    // DirectX config
    DirectXConfig dxCFG;
    dxCFG.BufferCount = 2;
    dxCFG.Width = winCFG.CurrentWidth;
    dxCFG.Height = winCFG.CurrentHeight;
    dxCFG.m_hwnd = gWindow->GetHWND();
    dxCFG.RefreshRate = 60;
    dxCFG.UseHDR = true;
    dxCFG.Windowed = winCFG.Windowed;
    dxCFG.Ansel = USE_ANSEL;

    // TODO: MSAA Support
    dxCFG.MSAA = false;
    dxCFG.MSAA_Samples = 1;
    dxCFG.MSAA_Quality = 0;

    // Create device and swap chain
    gDirectX->ShowError(gDirectX->Create(dxCFG));

    //std::cout << "Ansel avaliable: " << ansel::isAnselAvailable() << std::endl;

    // Set frame function
    bool(_DirectX::*gFrameFunction)(void);
    gFrameFunction = &_DirectX::FrameFunction;

    gWindow->SetFrameFunction(gFrameFunction); // Ref to function
    gWindow->SetDirectX(gDirectX);             // Ref to global object

    // Set directx object
    DirectXChild::SetDirectX(gDirectX);

    // Load game data
    gDirectX->Load();

    // Start rendering loop
    gWindow->Loop();

    // Unload game
    gWindow->Destroy();
    gDirectX->Unload();
}
