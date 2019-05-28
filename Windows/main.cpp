// Extensions
#include "Engine/Extensions/Default.h"
#include "Engine/Input/Gamepad.h"

#include "EngineIncludes/MainInclude.h"

// Test instances
Camera *cPlayer, *c2DScreen, *cLight;
Shader *shTest, *shTerrain, *shSkeletalAnimations, *shGUI, *shVertexOnly, 
       *shSkybox, *shTexturedQuad, *shPostProcess, *shSSLR;
Model *mModel1, *mModel2, *mScreenPlane, *mModel3, *mSpaceShip;
ModelInstance *mLevel1, *mDunes, *mCornellBox, *mSkybox, *miSpaceShip;

Texture *tDefault, *tBlueNoiseRG, *tClearPixel;
DiffuseMap *mDefaultDiffuse;
Sampler *sPoint;

Material *mDefault;

RenderBufferDepth2D *bDepth;
RenderBufferColor3Depth *bGBuffer;
RenderBufferColor1 *bSSLR;

CubemapTexture *pCubemap;

Query *pQuery;

BlendState *pBlendState0; // TODO: Use BlendState class

SoundEffect *sfxShotSingle, *sfxGunReload, *sfxWalk1, *sfxWalk2;
Music* mscMainTheme;

// Not yet done
//#define LOWPOLY_EXAMPLE
#ifdef LOWPOLY_EXAMPLE
Mesh* mTerrainMesh;

// Terrain example
// Single square
struct Square {
    int row, col;
    int NL, NR;

    Vertex_PNT StoreData(int size) {
        Vertex_PNT v;
            v.Position = DirectX::XMFLOAT3(row, 0., col);
            v.Normal   = DirectX::XMFLOAT3(0., 1., 0.);
            v.Texcoord = DirectX::XMFLOAT2((float)row / (float)size, (float)col / (float)size);
        return v;
    }

    Vertex_PNT StoreBottomRow(int size) {
        Vertex_PNT v;
            v.Position = DirectX::XMFLOAT3(row, 0., col);
            v.Normal   = DirectX::XMFLOAT3(0., 1., 0.);
            v.Texcoord = DirectX::XMFLOAT2((float)row / (float)size, (float)col / (float)size);
        return v;
    }
};

void CreateLowpolyTerrain(Mesh* mesh, int size) {
    IndexBuffer *ib = new IndexBuffer;
    VertexBuffer *vb = new VertexBuffer;

    // 
    int mNumVertices = 0, mIndexNum = 0;
    std::vector<int> indices;
    std::vector<Vertex_PNT> vertices;

    std::vector<Square> vLastRow;

    // Calculate vertex count
    mNumVertices = size * 2 + (size - 2) * (size - 1) * 2;

    // Resize vector
    vertices.resize(mNumVertices);

    // Populate vertices
    for( int i = 0; i < size - 1; i++ ) {
        for( int j = 0; j < size - 1; j++ ) {
            // Store square
            Square s = {i, j};
            vertices.push_back(s.StoreData(size));

            // Save last row
            if( i == size - 2 ) {
                vLastRow.push_back(s);
            }
        }
    }

    std::cout << "Vertices populated" << std::endl;

    // Process last row
    for( int i = 0; i < vLastRow.size(); i++ ) {
        vertices.push_back(vLastRow[i].StoreBottomRow(size));
    }

    std::cout << "Last row processed" << std::endl;

    // Generate indices
    // ...
    // 
    int rLen = (mNumVertices - 1) * 2;
    mIndexNum = (mNumVertices - 1) * (mNumVertices - 1) * 6;

    // Store top section
    for( int i = 0; i < mNumVertices - 3; i++ ) {
        for( int j = 0; j < mNumVertices - 1; j++ ) {
            int TL = (i * rLen) + j * 2;
            int TR = (TL + 1);
            int BL = TL + rLen;
            int BR = BL + 1;

            // Store quad
            bool bRighthanded = (i % 2) != (j % 2);

            // Store Left Tri
            indices.push_back(TL);
            indices.push_back(BL);
            indices.push_back(bRighthanded ? BR : TR);

            // Store Right Tri
            indices.push_back(TR);
            indices.push_back(bRighthanded ? TL : BL);
            indices.push_back(BR);
        }
    }

    std::cout << "Top section is stored" << std::endl;

    // Store 2nd last line
    for( int i = 0, j = mNumVertices - 3; i < mNumVertices - 1; i++ ) {
        int TL = (j * rLen) + i * 2;
        int TR = (TL + 1);
        int BL = TL + rLen - i;
        int BR = BL + 1;

        // Store quad
        bool bRighthanded = (i % 2) != (j % 2);

        // Store Left Tri
        indices.push_back(TL);
        indices.push_back(BL);
        indices.push_back(bRighthanded ? BR : TR);

        // Store Right Tri
        indices.push_back(TR);
        indices.push_back(bRighthanded ? TL : BL);
        indices.push_back(BR);
    }

    std::cout << "2nd line is stored" << std::endl;

    // Store last line
    for( int i = 0, j = mNumVertices - 2; i < mNumVertices - 1; i++ ) {
        int TL = (j * rLen) + i;
        int TR = (TL + 1);
        int BL = TL + mNumVertices;
        int BR = BL + 1;

        // Store last row quad
        bool bRighthanded = (i % 2) != (j % 2);

        // Store Left Tri
        indices.push_back(TL);
        indices.push_back(BL);
        indices.push_back(bRighthanded ? BR : TR);

        // Store Right Tri
        indices.push_back(BR);
        indices.push_back(TR);
        indices.push_back(bRighthanded ? TL : BL);
    }

    std::cout << "Last line is stored" << std::endl;

    // Create buffers
    ib->CreateDefault(mIndexNum, &indices[0]);
    vb->CreateDefault(mNumVertices, sizeof(Vertex_PNT), &vertices[0]);

    // Debug
    ib->SetName("Lowpoly Terrain's Index Buffer");
    vb->SetName("Lowpoly Terrain's Vertex Buffer");

    // Create mesh
    mesh->SetBuffer(vb, ib);
}
#endif

// HBAO+
#if USE_HBAO_PLUS
GFSDK_SSAO_CustomHeap CustomHeap;
GFSDK_SSAO_Context_D3D11* pAOContext;

GFSDK_SSAO_InputData_D3D11 _Input;
GFSDK_SSAO_Parameters Params;
GFSDK_SSAO_Output_D3D11 Output;
#endif

float fAspect = 1024.f / 540.f;
bool bIsWireframe = false, bDebugGUI = false;
int SceneID = 0;
int sfxWalkIndex;

// Define Frame Function
bool _DirectX::FrameFunction() {
    // Resize event
    Resize();
    
    // Bind and clear RTV
    gContext->OMSetRenderTargets(1, &gRTV, gDSV);

    float Clear[4] = {.2f, .2f, .2f, 1.f}; // RGBA
    float Clear0[4] = {0.f, 0.f, 0.f, 1.f}; // RGBA black
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
    
    if( bIsWireframe ) {
        gContext->RSSetState(gRSDefaultWriteframe);
    } else {
        gContext->RSSetState(gRSDefault);
    }

    // Render to default buffers
    //gContext->OMSetRenderTargets(1, &gRTV, gDSV);
    bGBuffer->Bind();
    gContext->ClearRenderTargetView(bGBuffer->GetColor0()->pRTV, Clear);
    gContext->ClearRenderTargetView(bGBuffer->GetColor1()->pRTV, Clear0);
    gContext->ClearRenderTargetView(bGBuffer->GetColor2()->pRTV, Clear0);
    gContext->ClearDepthStencilView(bGBuffer->GetDepth()->pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

    // Render scene

    // Bind light buffer
    cLight->BindBuffer(Shader::Vertex, 1);
    
    // Bind material
    mDefault->BindTextures(Shader::Pixel);

    // Bind depth buffer
    bDepth->BindResources(Shader::Pixel, 1);
    sPoint->Bind(Shader::Pixel, 1);

    // Bind noise texture
    tBlueNoiseRG->Bind(Shader::Pixel, 2);
    sPoint->Bind(Shader::Pixel, 2);

    // Bind cubemap
    pCubemap->Bind(Shader::Pixel, 3);
    sPoint->Bind(Shader::Pixel, 3);

    // Render level
    //mLevel1->Bind(cPlayer);
    //mLevel1->Render();

    //mTerrainMesh->Bind();
    //mTerrainMesh->Render();
    
    miSpaceShip->Bind(cPlayer);
    miSpaceShip->Render();

#pragma region Occlusion query
    // Begin occlusion query
    //pQuery->Begin();

    // 
    //gContext->OMSetBlendState(pBlendState0, NULL, 1);

    // Render quad at sun's position
    //DirectX::XMVECTOR SunWorldPos = {90.f, 5.f, 0.f};
    //
    //auto mWorld = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(-90.f)) * 
    //              DirectX::XMMatrixTranslationFromVector(SunWorldPos);
    //cPlayer->SetWorldMatrix(mWorld);
    //cPlayer->BuildConstantBuffer();
    
    //shTexturedQuad->Bind();
    //cPlayer->BindBuffer(Shader::Vertex, 0);

    // We don't wan't to render quad to the screen
    // So unbind texture
    ////tClearPixel->Bind(Shader::Pixel);
    //sPoint->Bind(Shader::Pixel);

    //mScreenPlane->Render();

    // End occlusion query test
    //UINT64 QueryValue = (UINT64)pQuery->End();
    //float proc = fmin(1.f, (float)QueryValue / (8100 * fAspect / (1024. / 540.))); // Light flare brightness
    // TODO: Add distance to sun

    // Disable depth test
    //gContext->OMSetBlendState(NULL, NULL, 1);
    //gContext->OMSetDepthStencilState(pDSS_Default_NoDepthWrite, 1);
#pragma endregion

    // Render skybox
    mSkybox->Bind(cPlayer);

    pCubemap->Bind(Shader::Pixel);
    sPoint->Bind(Shader::Pixel);

    mSkybox->Render();

    // Enable depth test
    gContext->OMSetDepthStencilState(pDSS_Default, 1);
    
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

    // Set defaults
    gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gContext->RSSetState(gRSDefault);

    sRenderBuffer* _Depth = bGBuffer->GetDepth();
    sRenderBuffer* _Color0 = bGBuffer->GetColor0(); // Diffuse
    sRenderBuffer* _Color1 = bGBuffer->GetColor1(); // Normal
    sRenderBuffer* _Color2 = bGBuffer->GetColor2(); // Specular

    // Screen-space local reflections
    bSSLR->Bind(); // Render Target
    shSSLR->Bind(); // Shader
    gContext->ClearRenderTargetView(bSSLR->GetColor0()->pRTV, Clear0);

    c2DScreen->SetWorldMatrix(DirectX::XMMatrixIdentity());
    c2DScreen->BuildConstantBuffer(); // Constant buffer
    c2DScreen->BindBuffer(Shader::Vertex, 0);

    gContext->PSSetShaderResources(0, 1, &_Color0->pSRV);
    gContext->PSSetShaderResources(1, 1, &_Color1->pSRV);
    gContext->PSSetShaderResources(2, 1, &_Depth->pSRV);

    sPoint->Bind(Shader::Pixel, 0);
    sPoint->Bind(Shader::Pixel, 1);
    sPoint->Bind(Shader::Pixel, 2);

    mScreenPlane->Render();

    // Render to screen
    sRenderBuffer* _SSLRBf = bSSLR->GetColor0();

    gContext->OMSetRenderTargets(1, &gRTV, nullptr);
    
    shPostProcess->Bind();
    
    // Diffuse
    gContext->PSSetShaderResources(0, 1, &_Color0->pSRV);
    sPoint->Bind(Shader::Pixel);

    // SSLR
    //gContext->PSSetShaderResources(1, 1, &_SSLRBf->pSRV);
    //sPoint->Bind(Shader::Pixel);

    mScreenPlane->Render();

    // HBAO+
#if USE_HBAO_PLUS
    DirectX::XMFLOAT4X4 mProjDest;
    DirectX::XMStoreFloat4x4(&mProjDest, cPlayer->GetProjMatrix());
    _Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4(&mProjDest(0, 0));

    GFSDK_SSAO_Status status = GFSDK_SSAO_OK;
    status = pAOContext->RenderAO(gContext, _Input, Params, Output);
    assert(status == GFSDK_SSAO_OK);
#endif

    // Render debug GUI
    if( bDebugGUI ) {
        // 3 is best number here
        std::vector<ID3D11ShaderResourceView*> pDebugTextures = {
            _Color0->pSRV,
            _Color1->pSRV,
            _Color2->pSRV
        };

        shGUI->Bind();
        sPoint->Bind(Shader::Pixel);
        int size = pDebugTextures.size();
        float width = (cfg.Width / (float)size);
        float height = width * .5f;

        height /= cfg.Height; // Normalize
        width /= cfg.Width;

        //std::cout << width << " " << height << std::endl;

        for( int i = 0; i < size; i++ ) {
            // 
            DirectX::XMMATRIX mOffset = DirectX::XMMatrixTranslation(.67f - width * i * 2.f, .67f, 0.f);
            c2DScreen->SetWorldMatrix(DirectX::XMMatrixScaling(width, height, 1.f) * mOffset);
            c2DScreen->BuildConstantBuffer();
            c2DScreen->BindBuffer(Shader::Vertex, 0);

            // Bind texture
            gContext->PSSetShaderResources(0, 1, &pDebugTextures[(size - 1) - i]);

            // Render plane
            mScreenPlane->Render();
        }
    }

    // 2D Rendering
    ComposeUI();

    // End of frame
    gSwapchain->Present(1, 0);
    return false;
}

float fDir = 0.f, fPitch = 0.f;
void _DirectX::Tick(float fDeltaTime) {
    // Set light's view matrix to math main camera's one
    if( gKeyboard->IsPressed(VK_SPACE) ) {
        cLight->SetViewMatrix(cPlayer->GetViewMatrix());
        cLight->BuildConstantBuffer();
    }

    // Toggle debug
    bIsWireframe ^= gKeyboard->IsPressed(VK_F1); // Toggle wireframe mode
    bDebugGUI ^= gKeyboard->IsPressed(VK_F3);    // Toggle debug gui mode

    // Update camera
    const float fSpeed = 20.f, fRotSpeed = 100.f, fSensetivityX = 2.f, fSensetivityY = 3.f;
    DirectX::XMFLOAT3 f3Move(0.f, 0.f, 0.f); // Movement vector

    static DirectX::XMFLOAT2 pLastPos = {0, 0}; // Last mouse pos

    // Camera
    if( gKeyboard->IsDown(VK_W) ) f3Move.x = +fSpeed * fDeltaTime;  // Forward / Backward
    if( gKeyboard->IsDown(VK_S) ) f3Move.x = -fSpeed * fDeltaTime;
    if( gKeyboard->IsDown(VK_D) ) f3Move.z = +fSpeed * fDeltaTime;  // Strafe
    if( gKeyboard->IsDown(VK_A) ) f3Move.z = -fSpeed * fDeltaTime;

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

            fDir   += dx;
            fPitch -= dy;
        }
    } //else
#endif
    {
        // Use mouse
        bool b = false;
        if( abs(dx) <= .1 ) {
            fDir += (float(gMouse->GetX() - cfg.Width  * .5f) * fSensetivityX * fDeltaTime);
            b = true;
        }

        if( abs(dy) <= .1 ) {
            fPitch += (float(gMouse->GetY() - cfg.Height * .5f) * fSensetivityY * fDeltaTime);
            b = true;
        }

        if( b ) gMouse->SetAt(int(cfg.Width  * .5f), int(cfg.Height * .5f));
    }

    // Walk SFX
    /*if( abs(f3Move.x) > 0.f ) {
        sfxWalkIndex = (sfxWalkIndex + 1) % 100;

        if( sfxWalkIndex == 50 ) {
            sfxWalk1->PlayNQ();
        } else if( sfxWalkIndex == 0 ) {
            sfxWalk2->PlayNQ();
        }
    }*/

    if( gKeyboard->IsDown(VK_LEFT ) ) fDir -= fRotSpeed * fDeltaTime; // Right / Left 
    if( gKeyboard->IsDown(VK_RIGHT) ) fDir += fRotSpeed * fDeltaTime;

    // I got used to KSP and other avia/space sims
    // So i flipped them
    if( gKeyboard->IsDown(VK_UP  ) ) fPitch -= fRotSpeed * fDeltaTime; // Look Up / Down
    if( gKeyboard->IsDown(VK_DOWN) ) fPitch += fRotSpeed * fDeltaTime;

    // Sound check
    if( gMouse->IsPressed(MouseButton::Left) ) {
        sfxShotSingle->PlayNQ();
        std::cout << "Left\n";
    }

    if( gKeyboard->IsPressed(VK_R) ) {
        sfxGunReload->PlayNQ();
        std::cout << "Reload\n";
    }

    // Limit pitch
    fPitch = std::min(std::max(fPitch, -84.f), 84.f);

    // Look around
    cPlayer->TranslateLookAt(f3Move);
    cPlayer->RotateAbs(DirectX::XMFLOAT3(fPitch, fDir, 0.));
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

    // Save aspect
    fAspect = cfg2.fAspect;

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
    // Temp Bug fix
    gKeyboard->SetState(VK_W, false);
    gKeyboard->SetState(VK_S, false);
    gKeyboard->SetState(VK_UP, false);
    gKeyboard->SetState(VK_DOWN, false);
    gKeyboard->SetState(VK_LEFT, false);
    gKeyboard->SetState(VK_RIGHT, false);

    // Create instances
    shTest = new Shader();
    shTerrain = new Shader();
    shSkeletalAnimations = new Shader();
    shGUI = new Shader();
    shVertexOnly = new Shader();
    shSkybox = new Shader();
    shTexturedQuad = new Shader();
    shPostProcess = new Shader();
    shSSLR = new Shader();

    cPlayer = new Camera(DirectX::XMFLOAT3(50, 2, -2), DirectX::XMFLOAT3(0., 0.f, 0.));
    c2DScreen = new Camera();
    cLight = new Camera(DirectX::XMFLOAT3(-10.f, 10.f, -10.f), DirectX::XMFLOAT3(45.f, 0.f, 0.f));

    tDefault = new Texture();
    tBlueNoiseRG = new Texture();
    tClearPixel = new Texture();

    sPoint = new Sampler();

    mDefaultDiffuse = new DiffuseMap();

    mDefault = new Material();

    bDepth = new RenderBufferDepth2D();
    bGBuffer = new RenderBufferColor3Depth();
    bSSLR = new RenderBufferColor1();

    pCubemap = new CubemapTexture();

    pQuery = new Query();

    sfxShotSingle = new SoundEffect("../Sounds/SingleEnergyShot.wav");
    sfxShotSingle->Create();
    sfxShotSingle->SetVolume(.2f);

    sfxGunReload = new SoundEffect("../Sounds/EnergyReload.wav");
    sfxGunReload->Create();
    sfxGunReload->SetVolume(.2f);

    mscMainTheme = new Music("../Music/EnergyReload.wav");
    mscMainTheme->Create();
    mscMainTheme->SetVolume(.2f);

    sfxWalk1 = new SoundEffect("../Sounds/Paid/Walk1.wav");
    sfxWalk1->Create();
    sfxWalk1->SetVolume(.2f);

    sfxWalk2 = new SoundEffect("../Sounds/Paid/Walk2.wav");
    sfxWalk2->Create();
    sfxWalk2->SetVolume(.2f);

    // Ansel support
#if USE_ANSEL
    AnselEnable(cPlayer->GetViewMatrix());
#endif

    const WindowConfig& cfg = gWindow->GetCFG();

    // Screen-Space Local Reflections buffer
    bSSLR->SetSize(1024, 540);
    bSSLR->CreateColor0(DXGI_FORMAT_R8G8B8A8_UNORM);

    // Geometry Buffer
    bGBuffer->SetSize(cfg.CurrentWidth, cfg.CurrentHeight);
    bGBuffer->CreateDepth(32);
    bGBuffer->CreateColor0(DXGI_FORMAT_R16G16B16A16_FLOAT); // Diffuse
    bGBuffer->CreateColor1(DXGI_FORMAT_R16G16_FLOAT);       // Normal
    bGBuffer->CreateColor2(DXGI_FORMAT_R8G8B8A8_UNORM);     // Specular
    
    // Create blend state with no color write
    pBlendState0 = new BlendState();

    D3D11_BLEND_DESC pDesc_;
    pDesc_.RenderTarget[0].BlendEnable = true;
    pDesc_.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    pDesc_.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    pDesc_.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    pDesc_.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    pDesc_.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    pDesc_.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    pDesc_.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    pDesc_.AlphaToCoverageEnable = false;
    pDesc_.IndependentBlendEnable = false;

    pBlendState0->Create(pDesc_, {1.f, 1.f, 1.f, 1.f});

    // Create occlusion query
    pQuery->Create(D3D11_QUERY_OCCLUSION);

    // Create cubemap
    pCubemap->CreateFromFiles("../Textures/Cubemaps/Test/", false, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Create depth buffer
    bDepth->Create(2048, 2048, 32);
    bDepth->SetName("Depth buffer");

    // Create default texture
    tDefault->Load("../Textures/TileInverse.png", DXGI_FORMAT_R8G8B8A8_UNORM);
    tDefault->SetName("Default tile texture");
    //gContext->GenerateMips(tDefault->GetSRV());

    // Set default texture
    Model::SetDefaultTexture(tDefault);

    // Load more textures
    tBlueNoiseRG->Load("../Textures/Noise/Blue/LDR_RG01_0.png", DXGI_FORMAT_R16G16_UNORM);

    //tClearPixel->Load("../Textures/ClearPixel.png", DXGI_FORMAT_R8G8B8A8_UNORM);

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

    // Setup cameras
    CameraConfig cfg2;
    cfg2.fAspect = float(cfg.CurrentWidth) / float(cfg.CurrentHeight);
    cfg2.FOV = 100.f;
    cfg2.fNear = .1f;
    cfg2.fFar = 10000.f;
    cPlayer->SetParams(cfg2);
    cPlayer->BuildProj();
    cPlayer->BuildView();

    // Save aspect for further use
    fAspect = cfg2.fAspect;

    // 
    cfg2.FOV = 90.f;
    cfg2.fNear = .1f;
    cfg2.fFar = 1.f;
    c2DScreen->SetParams(cfg2);
    c2DScreen->BuildProj();
    c2DScreen->BuildView();

    // 
    cfg2.fNear = .1f;
    cfg2.fFar = 10000.f;
    cfg2.FOV = 90.f;
    cfg2.fAspect = 1.f;
    cLight->SetParams(cfg2);
    cLight->BuildProj();
    cLight->BuildView();

    // Load shader
    shTest->LoadFile("shTestVS.cso", Shader::Vertex);
    shTest->LoadFile("shTestPS.cso", Shader::Pixel);

    // Don't forget to bind MatrixBuffer to Domain shader instead of Vertex
    shTerrain->LoadFile("shTerrainVS.cso", Shader::Vertex);
    shTerrain->LoadFile("shTerrainHS.cso", Shader::Hull);
    shTerrain->LoadFile("shTerrainDS.cso", Shader::Domain);
    shTerrain->LoadFile("shTerrainPS.cso", Shader::Pixel);

    // Skeletal animations
    shSkeletalAnimations->LoadFile("shSkeletalAnimationsVS.cso", Shader::Vertex);
    shSkeletalAnimations->LoadFile("shSkeletalAnimationsPS.cso", Shader::Pixel);

    // Vertex only shader
    shVertexOnly->LoadFile("shSimpleVS.cso", Shader::Vertex);
    shVertexOnly->SetNullShader(Shader::Pixel);

    // Skybox
    shSkybox->LoadFile("shSkyboxVS.cso", Shader::Vertex);
    shSkybox->LoadFile("shSkyboxPS.cso", Shader::Pixel);

    // Simple textured quad
    shTexturedQuad->LoadFile("shTexturedQuadVS.cso", Shader::Vertex);
    shTexturedQuad->LoadFile("shTexturedQuadPS.cso", Shader::Pixel);

    // Post process
    shPostProcess->LoadFile("shPostProcessVS.cso", Shader::Vertex);
    shPostProcess->LoadFile("shPostProcessPS.cso", Shader::Pixel);

    // SSLR
    shSSLR->AttachShader(shPostProcess, Shader::Vertex);
    shSSLR->LoadFile("shSSLRPS.cso", Shader::Pixel);

    // GUI
    //shGUI->LoadFile("../CompiledShaders/shGUIVS.cso", Shader::Vertex);
    shGUI->AttachShader(shPostProcess, Shader::Vertex);
    shGUI->LoadFile("shGUIPS.cso", Shader::Pixel);

    // Clean shaders
    shTest->ReleaseBlobs();
    shTerrain->ReleaseBlobs();
    shSkeletalAnimations->ReleaseBlobs();
    shGUI->ReleaseBlobs();
    shVertexOnly->ReleaseBlobs();
    shSkybox->ReleaseBlobs();
    shTexturedQuad->ReleaseBlobs();
    shPostProcess->ReleaseBlobs();
    shSSLR->ReleaseBlobs();

    // Create model
    mModel1 = new Model("Test model #1");
    mModel1->LoadModel<Vertex_PNT>("../Models/Teapot.obj");
    mModel1->EnableDefaultTexture();

    mModel2 = new Model("Test model #2");
    //mModel2->LoadModel("../Models/Dunes1.obj");

    mModel3 = new Model("Unit sphere");
    mModel3->LoadModel<Vertex_P>("../Models/UVMappedUnitSphere.obj");
    mModel3->DisableDefaultTexture();

    mScreenPlane = new Model("Screen plane model");
    mScreenPlane->LoadModel<Vertex_PT>("../Models/ScreenPlane.obj");
    mScreenPlane->DisableDefaultTexture();

    mSpaceShip = new Model("Space ship model");
    mSpaceShip->LoadModel<Vertex_PNT>("../Models/Space Ship Guns1.obj");
    mSpaceShip->EnableDefaultTexture();

    // Create model instances
    // Test level
    mLevel1 = new ModelInstance();
    mLevel1->SetName("Level 1 Instance");
    mLevel1->SetWorldMatrix(DirectX::XMMatrixScaling(4, 4, 4));
    mLevel1->SetShader(shTest);
    mLevel1->SetModel(mModel1);

    // Dunes
    mDunes = new ModelInstance();
    /*mDunes->SetName("Dunes Instance");
    mDunes->SetWorldMatrix(DirectX::XMMatrixScaling(4, 4, 4));
    mDunes->SetShader(shTerrain);
    mDunes->SetModel(mModel2);
    mDunes->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    mDunes->SetBindBuffer(Shader::Domain);*/

    // Cornell box
    mCornellBox = new ModelInstance();
    /*mCornellBox->SetName("Cornell box Instance");
    mCornellBox->SetWorldMatrix(DirectX::XMMatrixScaling(1, 1, 1));
    mCornellBox->SetShader(shTest);
    mCornellBox->SetModel(mModel3);*/

    // Skybox
    mSkybox = new ModelInstance();
    mSkybox->SetModel(mModel3);
    mSkybox->SetShader(shSkybox);
    mSkybox->SetWorldMatrix(DirectX::XMMatrixScaling(1000, 1000, 1000));

    // Space ship
    miSpaceShip = new ModelInstance();
    miSpaceShip->SetModel(mSpaceShip);
    miSpaceShip->SetShader(shTest);
    miSpaceShip->SetWorldMatrix(DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(180.f)) *
                                DirectX::XMMatrixScaling(10.f, 10.f, 10.f) * 
                                DirectX::XMMatrixTranslation(50.f, 10.f, 0.f));
    
    // Speaks for it's self
#ifdef LOWPOLY_EXAMPLE
    mTerrainMesh = new Mesh();
    CreateLowpolyTerrain(mTerrainMesh, 10);
#endif

    // HBAO+
#if USE_HBAO_PLUS
    CustomHeap.new_ = ::operator new;
    CustomHeap.delete_ = ::operator delete;

    GFSDK_SSAO_Status status;
    status = GFSDK_SSAO_CreateContext_D3D11(gDevice, &pAOContext, &CustomHeap);
    assert(status == GFSDK_SSAO_OK); // HBAO+ requires feature level 11_0 or above

    _Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
    _Input.DepthData.pFullResDepthTextureSRV = bGBuffer->GetDepth()->pSRV;
    _Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
    _Input.DepthData.MetersToViewSpaceUnits = 1.;

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
#ifdef _DEBUG
    if( gDirectX->gDebug ) gDirectX->gDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
}

void _DirectX::Unload() {
    // HBAO+
#if USE_HBAO_PLUS
    pAOContext->Release();
#endif

    // Release SFX
    sfxShotSingle->Release();
    sfxGunReload->Release();
    mscMainTheme->Release();

    // Release queries
    pQuery->Release();

    // Release cubemaps
    pCubemap->Release();

    // Release materials
    mDefault->Release();

    // Release textures
    tBlueNoiseRG->Release();
    tClearPixel->Release();
    //tDefault->Release(); // Material will delete it

    // Release shaders
    shTest->DeleteShaders();
    shTerrain->DeleteShaders();
    shGUI->DeleteShaders();
    shSkeletalAnimations->DeleteShaders();
    shVertexOnly->DeleteShaders();
    shSkybox->DeleteShaders();
    shTexturedQuad->DeleteShaders();
    shPostProcess->DeleteShaders();
    shSSLR->DeleteShaders();

    // Release models
    mModel1->Release();
    mModel2->Release();
    mModel3->Release();
    mScreenPlane->Release();

    // Release meshes
#ifdef LOWPOLY_EXAMPLE
    mTerrainMesh->Release();
#endif

    // Release buffers
    bDepth->Release();
    bGBuffer->Release();
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
    gAudioDevice = new AudioDevice();
    
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
#if USE_GAMEPADS
    for( int i = 0; i < NUM_GAMEPAD; i++ ) gGamepad[i] = gInput->GetGamepad(i);
#endif

    // Audio device config
    AudioDeviceConfig adCFG;
    adCFG.Flags = 0;

    // Create audio device
    gAudioDevice->Create(&adCFG);

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

    // <strike>TODO: MSAA Support</strike>
    // TODO: Remove MSAA from here
    // TODO: Add TAA support
    dxCFG.MSAA = false;
    dxCFG.MSAA_Samples = 1;
    dxCFG.MSAA_Quality = 0;

    // Create device and swap chain
    if( gDirectX->ShowError(gDirectX->Create(dxCFG)) ) { return 1; }

    //std::cout << "Ansel avaliable: " << ansel::isAnselAvailable() << std::endl;

    // Set frame function
    bool(_DirectX::*gFrameFunction)(void);
    gFrameFunction = &_DirectX::FrameFunction;

    gWindow->SetFrameFunction(gFrameFunction); // Ref to function
    gWindow->SetDirectX(gDirectX);             // Ref to global object
    //gWindow->SetAudioDevice(0, gAudioDevice);  // Ref to global audio device[0]

    // Set directx object
    DirectXChild::SetDirectX(gDirectX);

    // Set audio object
    AudioDeviceChild::SetAudioDevice(gAudioDevice);

    // Load game data
    gDirectX->Load();

    // Start rendering loop
    gWindow->Loop();

    // Unload game
    gWindow->Destroy();
    gDirectX->Unload();
    gAudioDevice->Release();
}
