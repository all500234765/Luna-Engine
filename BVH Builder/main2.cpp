#include "Defines.h"

#include "Engine Includes/MainInclude.h"

#include "HighLevel/DirectX/HighLevel.h"
#include "HighLevel/DirectX/Utlities.h"

#include "Engine/Model/Mesh.h"

HighLevel gHighLevel;
Model mModel;
Texture DefaultTexture;
Camera *cPlayer;

NewMesh<3> mNewMesh;

bool _DirectX::FrameFunction() {
    //ScopedRangeProfiler s0(__FUNCTION__);

    // Resize event
    Resize();
    {
        ScopedRangeProfiler s1(L"Clear");

        // Bind and clear RTV
        gContext->OMSetRenderTargets(1, &gRTV, gDSV);

        float Clear[4] = { .2f, .2f, .2f, 1.f }; // RGBA
        float Clear0[4] = { 0.f, 0.f, 0.f, 1.f }; // RGBA black
        gContext->ClearRenderTargetView(gRTV, Clear0);
        gContext->ClearDepthStencilView(gDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.f, 0);
        //                                                                Default is 1 ^^^
    }



    // 
    //shRenderAVC.Bind();

    cPlayer->BuildProj();
    cPlayer->BuildView();
    cPlayer->BuildConstantBuffer();
    cPlayer->BindBuffer(Shader::Geometry, 0);

    // Render 
    //gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    //vb.BindVertex();

    //gContext->Draw(vb.GetNumber(), 0);

    // End of frame
    gSwapchain->Present(1, 0);
    return false;
}

bool bLookMouse = true;

float fSpeed = 37.f, fRotSpeed = 100.f, fSensetivityX = 2.f, fSensetivityY = 3.f;
float fDir = 0.f, fPitch = 0.f;
void _DirectX::Tick(float fDeltaTime) {
    const WindowConfig& winCFG = gWindow->GetCFG();

    // Update camera
    float3 f3Move(0.f, 0.f, 0.f); // Movement vector

    static float2 pLastPos = { 0, 0 }; // Last mouse pos

    // Camera
    if( gKeyboard->IsDown(VK_W) ) f3Move.x = +fSpeed * fDeltaTime;  // Forward / Backward
    if( gKeyboard->IsDown(VK_S) ) f3Move.x = -fSpeed * fDeltaTime;
    if( gKeyboard->IsDown(VK_D) ) f3Move.z = +fSpeed * fDeltaTime;  // Strafe
    if( gKeyboard->IsDown(VK_A) ) f3Move.z = -fSpeed * fDeltaTime;

    bLookMouse ^= gKeyboard->IsPressed(VK_F2); // Toggle release mouse

    if( !bLookMouse ) { return; }
    float dx = 0.f, dy = 0.f;

#if USE_GAMEPADS
    // Use gamepad if we can and it's connected
    if( gGamepad[0]->IsConnected() ) {
        // Move around
        if( !gGamepad[0]->IsDeadZoneL() ) {
            f3Move.x = gGamepad[0]->LeftY() * fSpeed * fDeltaTime;
            f3Move.z = gGamepad[0]->LeftX() * fSpeed * fDeltaTime;
        }

        // Look around
        if( !gGamepad[0]->IsDeadZoneR() ) {
            dx = gGamepad[0]->RightX() * 100.f * fSensetivityX * fDeltaTime;
            dy = gGamepad[0]->RightY() *  50.f * fSensetivityY * fDeltaTime;

            fDir += dx;
            fPitch -= dy;
        }
    } //else
#endif

    {
        // Use mouse
        bool b = false;
        if( abs(dx) <= .1 ) {
            fDir += (float(gMouse->GetX() - winCFG.CurrentWidth * .5f) * fSensetivityX * fDeltaTime);
            b = true;
        }

        if( abs(dy) <= .1 ) {
            fPitch += (float(gMouse->GetY() - winCFG.CurrentHeight * .5f) * fSensetivityY * fDeltaTime);
            b = true;
            //std::cout << winCFG.CurrentHeight2 * .5f << " " << gMouse->GetY() << std::endl;
        }

        if( b ) gMouse->SetAt(int(winCFG.CurrentWidth * .5f), int(winCFG.CurrentHeight * .5f));
    }

    if( gKeyboard->IsDown(VK_LEFT) ) fDir -= fRotSpeed * fDeltaTime; // Right / Left 
    if( gKeyboard->IsDown(VK_RIGHT) ) fDir += fRotSpeed * fDeltaTime;

    // I got used to KSP and other avia/space sims
    // So i flipped them
    if( gKeyboard->IsDown(VK_UP) ) fPitch -= fRotSpeed * fDeltaTime; // Look Up / Down
    if( gKeyboard->IsDown(VK_DOWN) ) fPitch += fRotSpeed * fDeltaTime;

    // Limit pitch
    fPitch = std::min(std::max(fPitch, -84.f), 84.f);

    // Look around
    cPlayer->TranslateLookAt(f3Move);
    cPlayer->RotateAbs(DirectX::XMFLOAT3(fPitch, fDir, 0.));
}

void _DirectX::Resize() {
    gHighLevel.DefaultResize();


}

void _DirectX::Load() {
    DefaultTexture.Load("../Textures/TileInverse.png", false, false);
    Model::SetDefaultTexture(&DefaultTexture);

    mModel.EnableDefaultTexture();
    mModel.LoadModel<Vertex_PNT>("../Models/Cube.obj");


    // 
    VertexBuffer vbs[3];
    mNewMesh.SetVertexBuffers(&vbs[0]);

    //mNewMesh.SetIndexBuffer();

    // Load shaders


    cPlayer = new Camera();
    CameraConfig C_config;
    C_config.fAspect = cfg.Width / cfg.Height;
    C_config.fNear = .1f;
    C_config.fFar = 1000.f;
    C_config.FOV = 90.f;
    C_config.Ortho = false;
    cPlayer->SetParams(C_config);

}

void _DirectX::Unload() {
    mModel.Release();
    DefaultTexture.Release();



    SAFE_DELETE(cPlayer);

}

int main() {
    // Window config
    WindowConfig winCFG;
    winCFG.Borderless = false;
    winCFG.Windowed = true;
    winCFG.ShowConsole = true;
    winCFG.Width = 1024;
    winCFG.Height = 540;
    winCFG.Title = L"Luna Engine";
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
    //SAFE_DELETE(gHighLevel);
    SAFE_DELETE(gWindow);
    SAFE_DELETE(gDirectX);
    SAFE_DELETE_N(gGamepad, NUM_GAMEPAD);
    SAFE_DELETE(gInput);
    SAFE_RELEASE(gAudioDevice);
}

