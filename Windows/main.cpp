// Extensions
#include "Engine/Extensions/Default.h"
#include "Engine/Input/Gamepad.h"

#include "Engine Includes/MainInclude.h"

// Test instances
Camera *cPlayer, *c2DScreen, *cLight;
Shader *shSurface, *shTerrain, *shSkeletalAnimations, *shGUI, *shVertexOnly, 
       *shSkybox, *shTexturedQuad, *shPostProcess, *shSSLR, *shDeferred, 
       *shDeferredFinal, *shDeferredPointLight, *shScreenSpaceShadows, 
       *shUnitSphere, *shUnitSphereDepthOnly;
Model *mModel1, *mModel2, *mScreenPlane, *mModel3, *mSpaceShip, *mShadowTest1, *mUnitSphereUV;
ModelInstance *mLevel1, *mDunes, *mCornellBox, *mSkybox, *miSpaceShip;

Texture *tDefault, *tBlueNoiseRG, *tClearPixel, *tOpacityDefault, *tSpecularDefault;
DiffuseMap *mDefaultDiffuse;
Sampler *sPoint, *sMipLinear, *sPointClamp, *sMipLinearOpacity, *sMipLinearRougness;

Material *mDefault;

RenderBufferDepth2D *bDepth;
RenderBufferColor3Depth *bGBuffer;
RenderBufferColor1 *bSSLR, *bDeferred, *bShadows;

CubemapTexture *pCubemap;

Query *pQuery;

BlendState *pBlendState0; // TODO: Use BlendState class

SoundEffect *sfxShotSingle, *sfxGunReload, *sfxWalk1, *sfxWalk2;
Music* mscMainTheme;

ID3D11RasterizerState *rsFrontCull;

D3D11_VIEWPORT vpDepth, vpMain;

// Physical objects
PhysicsObjectSphere *sphere1, *sphere2;

// Deferred light system needs those
ConstantBuffer *cbDeferredGlobalInst, *cbDeferredLightInst;

struct cbDeferredLight {
    DirectX::XMFLOAT3 _LightDiffuse; // Light diffuse color
    float             PADDING1;      // Unused
    DirectX::XMFLOAT2 _LightData;    // Range, intensity
    DirectX::XMFLOAT2 PADDING2;      // Unused
    DirectX::XMFLOAT4 vPosition;     // Camera pos, w - unused
};

struct cbDeferredGlobal {
    DirectX::XMFLOAT2 _TanAspect;  // dtan(fov * .5) * aspect, - dtan(fov / 2)
    DirectX::XMFLOAT2 _Texel;      // 1 / target width, 1 / target height
    float _Far;                    // Far
    float PADDING0;                // Unused
    DirectX::XMFLOAT4 _ProjValues; // 1 / m[0][0], 1 / m[1][1], m[3][2], -m[2][2]
    DirectX::XMMATRIX _mInvView;   // Inverse matrix of view matrix
    DirectX::XMFLOAT4 vCameraPos;  // Camera pos, w - unused
};

// SSLR
ConstantBuffer *cbSSLRMatrixInst;
struct cbSSLRMatrix {
    DirectX::XMMATRIX _mInvView; // Inverse matrix of view matrix
    DirectX::XMMATRIX _mInvProj; // Inverse matrix of projection matrix
    DirectX::XMMATRIX _mProj;
    DirectX::XMMATRIX _mView;
};

// Not yet done
// And prob. won't be
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
bool bIsWireframe = false, bDebugGUI = false, bLookMouse = true;
int SceneID = 0;
int sfxWalkIndex;

// Define Frame Function
bool _DirectX::FrameFunction() {
    // Resize event
    Resize();

    // Bind and clear RTV
    gContext->OMSetRenderTargets(1, &gRTV, gDSV);

    float Clear[4] = { .2f, .2f, .2f, 1.f }; // RGBA
    float Clear0[4] = { 0.f, 0.f, 0.f, 1.f }; // RGBA black
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


#pragma region Render depth buffer for directional light
    bDepth->Bind();

    gContext->RSSetState(rsFrontCull);
    gContext->RSSetViewports(1, &vpDepth);
    gContext->ClearDepthStencilView(bDepth->GetTarget(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    gContext->OMSetDepthStencilState(pDSS_Default, 1);

    miSpaceShip->Bind(cLight);
    shVertexOnly->Bind();
    miSpaceShip->Render();

    // Render physics engine test unit spheres
    shUnitSphereDepthOnly->Bind();

    // 
    gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

    auto DrawBall = [&](Camera* cam, const pFloat3& pos, float radius) {
        cam->SetWorldMatrix(DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));
        cam->BuildConstantBuffer({ 0., 0., 0., radius });
        cam->BindBuffer(Shader::Domain, 0);

        gContext->Draw(2, 0);
    };

    // Balls
    for( int i = 0; i < gPhysicsEngine->GetNumObjects(); i++ ) {
        DrawBall(cLight, gPhysicsEngine->GetObjectP(i)->GetPosition(), 
                 ((PhysicsObjectSphere*)gPhysicsEngine->GetObjectP(i))->GetRadius());
    }

    //mShadowTest1->Render();
#pragma endregion

    // Reset to defaults
    if( bIsWireframe ) {
        gContext->RSSetState(gRSDefaultWriteframe);
    } else {
        gContext->RSSetState(gRSDefault);
    }

#pragma region Render to gbuffer
    gContext->RSSetViewports(1, &vpMain);

    //gContext->OMSetRenderTargets(1, &gRTV, gDSV);
    ID3D11RenderTargetView *pEmptyRTV = nullptr;
    gContext->OMSetRenderTargets(1, &pEmptyRTV, nullptr);

    bGBuffer->Bind();
    gContext->ClearRenderTargetView(bGBuffer->GetColor0()->pRTV, Clear);
    gContext->ClearRenderTargetView(bGBuffer->GetColor1()->pRTV, Clear0);
    gContext->ClearRenderTargetView(bGBuffer->GetColor2()->pRTV, Clear0);
    gContext->ClearDepthStencilView(bGBuffer->GetDepth()->pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

    // Render scene

    // Bind light buffer
    cLight->BindBuffer(Shader::Vertex, 1);

    // Bind default material
    mDefault->BindTextures(Shader::Pixel, 0);
    sMipLinear->Bind(Shader::Pixel, 0);

    mDefault->BindTextures(Shader::Pixel, 1);
    sMipLinear->Bind(Shader::Pixel, 1);

    mDefault->BindTextures(Shader::Pixel, 2);
    sMipLinearOpacity->Bind(Shader::Pixel, 2);

    mDefault->BindTextures(Shader::Pixel, 3);
    sMipLinearRougness->Bind(Shader::Pixel, 3);

    // Bind depth buffer
    bDepth->BindResources(Shader::Pixel, 4);
    sPointClamp->Bind(Shader::Pixel, 4);

    // Bind noise texture
    tBlueNoiseRG->Bind(Shader::Pixel, 5);
    sPoint->Bind(Shader::Pixel, 5);

    // Bind cubemap
    pCubemap->Bind(Shader::Pixel, 6);
    sPoint->Bind(Shader::Pixel, 6);

    // Render level
    //mLevel1->Bind(cPlayer);
    //mLevel1->Render();

    //mTerrainMesh->Bind();
    //mTerrainMesh->Render();

    miSpaceShip->Bind(cPlayer);
    miSpaceShip->Render();

    //mShadowTest1->Render();

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

    // Render physics engine test unit spheres
    shUnitSphere->Bind();

    // 
    gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

    // Physics

    // Balls
    for( int i = 0; i < gPhysicsEngine->GetNumObjects(); i++ ) {
        const PhysicsObject *obj = gPhysicsEngine->GetObjectP(i);

        if( obj->GetShapeType() == PhysicsShapeType::Sphere )
            DrawBall(cPlayer, obj->GetPosition(), ((PhysicsObjectSphere*)obj)->GetRadius());

    }

    // Render skybox
    mSkybox->Bind(cPlayer);

    pCubemap->Bind(Shader::Pixel);
    sMipLinear->Bind(Shader::Pixel);

    mSkybox->Render();
#pragma endregion

    // Enable depth test
    //gContext->OMSetDepthStencilState(pDSS_Default, 1);

    // Set defaults
    gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gContext->RSSetState(gRSDefault);

    sRenderBuffer* _Depth2 = bDepth->GetDepth();     // Shadow map
    sRenderBuffer* _Depth = bGBuffer->GetDepth();    // Depth
    sRenderBuffer* _Color0 = bGBuffer->GetColor0();  // Diffuse
    sRenderBuffer* _Color1 = bGBuffer->GetColor1();  // Normal
    sRenderBuffer* _Color2 = bGBuffer->GetColor2();  // Specular
    sRenderBuffer* _ColorD = bDeferred->GetColor0(); // Diffuse deferred
    sRenderBuffer* _SSLRBf = bSSLR->GetColor0();     // SSLR
    sRenderBuffer* _Shadow = bShadows->GetColor0();  // Shadow buffer

#pragma region Deferred rendering pass 1
    bDeferred->Bind();                                      // Set Render Target
    shDeferredPointLight->Bind();                           // Set Shader
    gContext->ClearRenderTargetView(_ColorD->pRTV, Clear0); // Clear Render Target

    gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

    // Build & Bind Constant buffer
    DirectX::XMFLOAT4 LightPos = { 1.62895, 5.54253, 2.93616/*60.4084, 22.6733, 2.3704*/, 100 };
    cPlayer->SetWorldMatrix(
        //DirectX::XMMatrixScaling(LightPos.w, LightPos.w, LightPos.w) *
        DirectX::XMMatrixTranslation(LightPos.x, LightPos.y, LightPos.z));
    cPlayer->BuildConstantBuffer({ LightPos.x, LightPos.y, LightPos.z, LightPos.w });
    cPlayer->BindBuffer(Shader::Domain, 0);

    // Update matrix
    float FOV = cPlayer->GetParams().FOV;
    float fFar = cPlayer->GetParams().fFar;
    DirectX::XMMATRIX mProjTmp = cPlayer->GetProjMatrix();
    float fHalfTanFov = tanf(DirectX::XMConvertToRadians(FOV * .5)); // dtan(fov * .5)
    cbDeferredGlobal* data = (cbDeferredGlobal*)cbDeferredGlobalInst->Map();
    data->_mInvView   = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(cPlayer->GetViewMatrix()), cPlayer->GetViewMatrix());
    data->_TanAspect = { fHalfTanFov * fAspect, -fHalfTanFov };
    data->_Texel = { 1.f / cfg.Width, 1.f / cfg.Height };
    data->_Far = fFar;
    data->PADDING0 = 0;
    data->_ProjValues = { 1.f / mProjTmp.r[0].m128_f32[0], 1.f / mProjTmp.r[1].m128_f32[1], mProjTmp.r[3].m128_f32[2], -mProjTmp.r[2].m128_f32[2] };
    cbDeferredGlobalInst->Unmap();

    // Bind buffers
    cbDeferredGlobalInst->Bind(Shader::Pixel, 0);
    cbDeferredLightInst->Bind(Shader::Pixel, 1);

    gContext->PSSetShaderResources(0, 1, &_Color1->pSRV);
    gContext->PSSetShaderResources(1, 1, &_Depth->pSRV);

    sPoint->Bind(Shader::Pixel, 0);
    sPoint->Bind(Shader::Pixel, 1);

    gContext->Draw(2, 0);

    //mUnitSphereUV->Render();

#pragma endregion

    // Defaults
    gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gContext->OMSetDepthStencilState(pDSS_Default, 1);

#pragma region Screen-space local reflections
    bSSLR->Bind(); // Render Target
    shSSLR->Bind(); // Shader
    gContext->ClearRenderTargetView(_SSLRBf->pRTV, Clear0);

    DirectX::XMVECTOR wparam;
    wparam.m128_f32[0] = cPlayer->GetPosition().x;
    wparam.m128_f32[1] = cPlayer->GetPosition().y;
    wparam.m128_f32[2] = cPlayer->GetPosition().z;
    wparam.m128_f32[3] = 1.;

    c2DScreen->SetWorldMatrix(DirectX::XMMatrixIdentity());
    c2DScreen->BuildConstantBuffer(wparam); // Constant buffer
    c2DScreen->BindBuffer(Shader::Vertex, 0);

    // Update constant buffer for SSLR
    cbSSLRMatrix *SSLRMdata = (cbSSLRMatrix*)cbSSLRMatrixInst->Map();
    //DirectX::XMMATRIX __m = cPlayer->GetProjMatrix() * cPlayer->GetViewMatrix();
    SSLRMdata->_mInvView = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(cPlayer->GetViewMatrix()), cPlayer->GetViewMatrix());
    SSLRMdata->_mInvProj = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(cPlayer->GetProjMatrix()), cPlayer->GetProjMatrix());
    SSLRMdata->_mView    = cPlayer->GetViewMatrix();
    SSLRMdata->_mProj    = cPlayer->GetProjMatrix();
    cbSSLRMatrixInst->Unmap();

    // Bind constant buffer
    cbSSLRMatrixInst->Bind(Shader::Pixel, 0);

    gContext->PSSetShaderResources(0, 1, &_Color0->pSRV); // Diffuse
    gContext->PSSetShaderResources(1, 1, &_Color1->pSRV); // Normal
    gContext->PSSetShaderResources(2, 1, &_Depth->pSRV);  // Depth

    sPoint->Bind(Shader::Pixel, 0);
    sPoint->Bind(Shader::Pixel, 1);
    sPoint->Bind(Shader::Pixel, 2);

    gContext->Draw(6, 0);
#pragma endregion

#pragma region Screen-Space Shadow mapping
    // Set up states
    /*bShadows->Bind();
    shScreenSpaceShadows->Bind();
    gContext->ClearRenderTargetView(_Shadow->pRTV, Clear0);

    // Constant buffers
    c2DScreen->SetWorldMatrix(DirectX::XMMatrixIdentity());
    c2DScreen->BuildConstantBuffer(); // Constant buffer
    c2DScreen->BindBuffer(Shader::Vertex, 0);

    cbSSLRMatrixInst->Bind(Shader::Pixel, 0);
    cLight->BindBuffer(Shader::Pixel, 1);

    //gContext->PSSetShaderResources(2, 1, &_Depth->pSRV);  // Depth

    // Bind scene depth buffer
    bGBuffer->BindResources(Shader::Pixel, 0);
    sPoint->Bind(Shader::Pixel, 0);

    // Bind sun depth buffer
    bDepth->BindResources(Shader::Pixel, 1);
    sPointClamp->Bind(Shader::Pixel, 1);

    // Bind noise texture
    tBlueNoiseRG->Bind(Shader::Pixel, 2);
    sPoint->Bind(Shader::Pixel, 2);

    gContext->Draw(6, 0);*/
#pragma endregion

#pragma region Deferred final pass

#pragma endregion

#pragma region Render to screen, Final Post Process Pass
    gContext->OMSetRenderTargets(1, &gRTV, nullptr);
    
    shPostProcess->Bind();

    c2DScreen->SetWorldMatrix(DirectX::XMMatrixIdentity());
    c2DScreen->BuildConstantBuffer(); // Constant buffer
    c2DScreen->BindBuffer(Shader::Vertex, 0);
    
    // Diffuse
    gContext->PSSetShaderResources(0, 1, &_Color0->pSRV);
    sPoint->Bind(Shader::Pixel, 0);

    // SSLR
    gContext->PSSetShaderResources(1, 1, &_SSLRBf->pSRV);
    sPoint->Bind(Shader::Pixel, 1);

    // Shadows
    gContext->PSSetShaderResources(2, 1, &_Shadow->pSRV);
    sPoint->Bind(Shader::Pixel, 2);

    // Deferred
    gContext->PSSetShaderResources(3, 1, &_ColorD->pSRV);
    sPoint->Bind(Shader::Pixel, 3);

    gContext->Draw(6, 0);

#pragma endregion

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
            _Shadow->pSRV,
            _SSLRBf->pSRV,
            _ColorD->pSRV
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
            gContext->Draw(6, 0);
        }
    }

    // 2D Rendering
    ComposeUI();

    // TODO: MT support
    if( cfg.DeferredContext ) {
        gContext->FinishCommandList(false, &pCommandList);
        gContext->ExecuteCommandList(pCommandList, true);
    }

    // End of frame
    gSwapchain->Present(1, 0);
    return false;
}

float fSpeed = 20.f, fRotSpeed = 100.f, fSensetivityX = 2.f, fSensetivityY = 3.f;
float fDir = 0.f, fPitch = 0.f;
void _DirectX::Tick(float fDeltaTime) {
    const WindowConfig& winCFG = gWindow->GetCFG();

    // Update physics
    gPhysicsEngine->Dispatch(fDeltaTime);

    // Set light's view matrix to math main camera's one
    if( gKeyboard->IsPressed(VK_SPACE) ) {
        cLight->SetViewMatrix(cPlayer->GetViewMatrix());
        cLight->BuildConstantBuffer();
    }

    // Toggle debug
    bIsWireframe ^= gKeyboard->IsPressed(VK_F1); // Toggle wireframe mode
    bDebugGUI ^= gKeyboard->IsPressed(VK_F3);    // Toggle debug gui mode
    bLookMouse ^= gKeyboard->IsPressed(VK_F2);   // Toggle release mouse

    // Update camera
    DirectX::XMFLOAT3 f3Move(0.f, 0.f, 0.f); // Movement vector

    static DirectX::XMFLOAT2 pLastPos = {0, 0}; // Last mouse pos

    // Camera
    if( gKeyboard->IsDown(VK_W) ) f3Move.x = +fSpeed * fDeltaTime;  // Forward / Backward
    if( gKeyboard->IsDown(VK_S) ) f3Move.x = -fSpeed * fDeltaTime;
    if( gKeyboard->IsDown(VK_D) ) f3Move.z = +fSpeed * fDeltaTime;  // Strafe
    if( gKeyboard->IsDown(VK_A) ) f3Move.z = -fSpeed * fDeltaTime;
    if( gKeyboard->IsPressed(VK_SPACE) ) {
        auto p = cPlayer->GetPosition();
        std::cout << p.x << ", " << p.y << ", " << p.z << std::endl;
    }

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

            fDir   += dx;
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
            fPitch += (float(gMouse->GetY() - winCFG.CurrentHeight2 * .5f) * fSensetivityY * fDeltaTime);
            b = true;
            //std::cout << winCFG.CurrentHeight2 * .5f << " " << gMouse->GetY() << std::endl;
        }

        if( b ) gMouse->SetAt(int(winCFG.CurrentWidth * .5f), int(winCFG.CurrentHeight2 * .5f));
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
#if _DEBUG_BUILD
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    // Create debug window
    ImGui::Begin("Debug");
    
    ImGui::Button("Test button");

    // Console
    static std::string buf;
    if( buf.size() == 0 ) {
        buf.resize(50, ' ');
    }

    ImGui::InputText("", (char*)buf.data(), buf.size());
    ImGui::SameLine();
    if( ImGui::Button("Submit") ) {
        std::cout << buf << std::endl;
        // gDebugObject->ConsoleParse(buf);
    }

    bIsWireframe ^= ImGui::Button("Wireframe");
    bDebugGUI    ^= ImGui::Button("Debug buffers");



    ImGui::End();
    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

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
    if( cfg.CurrentWidth <= 0 && cfg.CurrentHeight2 <= 0 ) { return;  } // Window was minimazed

    std::cout << "Window/DirectX resize event (w=" << cfg.CurrentWidth << ", h=" << cfg.CurrentHeight << ")" << std::endl;

    // Release targets
    gContext->OMSetRenderTargets(0, 0, 0);
    gRTV->Release();

    // Resize buffer
    bGBuffer->Resize(cfg.CurrentWidth, cfg.CurrentHeight);
    bSSLR->Resize(cfg.CurrentWidth, cfg.CurrentHeight);
    bDeferred->Resize(cfg.CurrentWidth, cfg.CurrentHeight);
    bShadows->Resize(cfg.CurrentWidth, cfg.CurrentHeight);

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

    vpMain.MinDepth = 0.f;
    vpMain.MaxDepth = 1.f;
    vpMain.TopLeftX = 0;
    vpMain.TopLeftY = 0;
    vpMain.Width    = vp.Width;
    vpMain.Height   = vp.Height;
}

void _DirectX::Load() {
    // Physics
    sphere1 = new PhysicsObjectSphere({ 0., 0.,   0. }, 1., { 0., 0., +1. });
    sphere2 = new PhysicsObjectSphere({ 0., 0., +10. }, 2., { 0., 0., -1. });

    gPhysicsEngine->PushObject(sphere1);
    gPhysicsEngine->PushObject(sphere2);

    // Temp Bug fix
    gKeyboard->SetState(VK_W, false);
    gKeyboard->SetState(VK_S, false);
    gKeyboard->SetState(VK_UP, false);
    gKeyboard->SetState(VK_DOWN, false);
    gKeyboard->SetState(VK_LEFT, false);
    gKeyboard->SetState(VK_RIGHT, false);

    // Create instances
    shSurface = new Shader();
    shTerrain = new Shader();
    shSkeletalAnimations = new Shader();
    shGUI = new Shader();
    shVertexOnly = new Shader();
    shSkybox = new Shader();
    shTexturedQuad = new Shader();
    shPostProcess = new Shader();
    shSSLR = new Shader();
    shDeferred = new Shader();
    shDeferredFinal = new Shader();
    shDeferredPointLight = new Shader();
    shScreenSpaceShadows = new Shader();
    shUnitSphere = new Shader();
    shUnitSphereDepthOnly = new Shader();

    cPlayer = new Camera();
    c2DScreen = new Camera();
    cLight = new Camera(DirectX::XMFLOAT3(-10.f, 10.f, -10.f), DirectX::XMFLOAT3(45.f, 0.f, 0.f));

    tDefault = new Texture();
    tBlueNoiseRG = new Texture();
    tClearPixel = new Texture();
    tOpacityDefault = new Texture();
    tSpecularDefault = new Texture();

    sPoint = new Sampler();
    sMipLinear = new Sampler();
    sPointClamp = new Sampler();
    sMipLinearOpacity = new Sampler();
    sMipLinearRougness = new Sampler();

    mDefaultDiffuse = new DiffuseMap();

    mDefault = new Material();

    bDepth = new RenderBufferDepth2D();
    bGBuffer = new RenderBufferColor3Depth();
    bSSLR = new RenderBufferColor1();
    bDeferred = new RenderBufferColor1();
    bShadows = new RenderBufferColor1();

    pCubemap = new CubemapTexture();

    pQuery = new Query();

    cbDeferredGlobalInst = new ConstantBuffer();
    cbDeferredLightInst  = new ConstantBuffer();
    cbSSLRMatrixInst = new ConstantBuffer();
    
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

    // Create constant buffers
    cbSSLRMatrixInst->CreateDefault(sizeof(cbSSLRMatrix));

    // 
    bShadows->SetSize(1024, 540);
    bShadows->CreateColor0(DXGI_FORMAT_R8G8B8A8_UNORM);
    
    // Create depth buffer
    bDepth->Create(2048, 2048, 32);
    bDepth->SetName("Shadow map depth buffer");

    // Deferred buffer
    bDeferred->SetSize(1024, 540);
    bDeferred->CreateColor0(DXGI_FORMAT_R16G16B16A16_FLOAT);

    // Screen-Space Local Reflections buffer
    bSSLR->SetSize(1024, 540);
    bSSLR->CreateColor0(DXGI_FORMAT_R8G8B8A8_UNORM);

    // Geometry Buffer
    bGBuffer->SetSize(cfg.CurrentWidth, cfg.CurrentHeight);
    bGBuffer->CreateDepth(32);
    bGBuffer->CreateColor0(DXGI_FORMAT_R16G16B16A16_FLOAT); // Diffuse
    bGBuffer->CreateColor1(DXGI_FORMAT_R16G16B16A16_FLOAT); // Normal
    bGBuffer->CreateColor2(DXGI_FORMAT_R8G8B8A8_UNORM);     // Specular
    
    // Create 
    D3D11_RASTERIZER_DESC rDesc2;
    ZeroMemory(&rDesc2, sizeof(D3D11_RASTERIZER_DESC));
    rDesc2.AntialiasedLineEnable = false;
    rDesc2.CullMode = D3D11_CULL_NONE;
    rDesc2.DepthBias = 0;
    rDesc2.DepthBiasClamp = 0.0f;
    rDesc2.DepthClipEnable = true;
    rDesc2.FillMode = D3D11_FILL_SOLID;
    rDesc2.FrontCounterClockwise = false;
    rDesc2.MultisampleEnable = false;
    rDesc2.ScissorEnable = false;
    rDesc2.SlopeScaledDepthBias = 0.0f;
    
    gDevice->CreateRasterizerState(&rDesc2, &rsFrontCull);
    
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

    // Create default texture
    tDefault->Load("../Textures/TileInverse.png", DXGI_FORMAT_R8G8B8A8_UNORM);
    tDefault->SetName("Default tile texture");
    //gContext->GenerateMips(tDefault->GetSRV());

    // Load more textures
    tBlueNoiseRG->Load("../Textures/Noise/Blue/LDR_RG01_0.png", DXGI_FORMAT_R16G16_UNORM);

    //tClearPixel->Load("../Textures/ClearPixel.png", DXGI_FORMAT_R8G8B8A8_UNORM);
    tOpacityDefault->Load("../Textures/ClearPixel.png", DXGI_FORMAT_R8G8B8A8_UNORM);
    tSpecularDefault->Load("../Texture/DarkPixel.png", DXGI_FORMAT_B8G8R8A8_UNORM);

    // Set default textures
    Model::SetDefaultTexture(tDefault);
    Model::SetDefaultTextureOpacity(tOpacityDefault);
    Model::SetDefaultTextureSpecular(tSpecularDefault);

    // Create samplers
    D3D11_SAMPLER_DESC pDesc;
    ZeroMemory(&pDesc, sizeof(D3D11_SAMPLER_DESC));
    pDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    pDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    pDesc.MaxLOD = D3D11_FLOAT32_MAX;
    pDesc.MinLOD = 0;
    pDesc.MipLODBias = 0;
    pDesc.MaxAnisotropy = 16;

    // Point sampler
    sPoint->Create(pDesc);
    sPoint->SetName("Point sampler");

    // Linear mip opacity sampler
    pDesc.BorderColor[0] = pDesc.BorderColor[1] = pDesc.BorderColor[2] = pDesc.BorderColor[3] = 1.;
    sMipLinearOpacity->Create(pDesc);
    sMipLinearOpacity->SetName("Linear mip opacity sampler");

    // Linear mip rougness sampler
    pDesc.BorderColor[0] = pDesc.BorderColor[1] = pDesc.BorderColor[2] = 0.;
    sMipLinearRougness->Create(pDesc);
    sMipLinearRougness->SetName("Linear mip rougness sampler");

    // Anisotropic mip sampler
    pDesc.Filter = D3D11_FILTER_MAXIMUM_ANISOTROPIC;
    sMipLinear->Create(pDesc);
    sMipLinear->SetName("Anisotropic mip sampler");

    // Clamped point sampler
    pDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    pDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    pDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    pDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    pDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    pDesc.BorderColor[0] = pDesc.BorderColor[1] = pDesc.BorderColor[2] = pDesc.BorderColor[3] = 1.;
    sPointClamp->Create(pDesc);
    sPointClamp->SetName("Clamp point sampler");

    // Create maps
    mDefaultDiffuse->mTexture = tDefault;

    // Create materials
    mDefault->SetDiffuse(mDefaultDiffuse);
    mDefault->SetSampler(sMipLinear);
    mDefault->SetName("Default material");

    // Setup cameras
    CameraConfig cfg2;
    cfg2.fAspect = float(cfg.CurrentWidth) / float(cfg.CurrentHeight2);
    cfg2.FOV = 100.f;
    cfg2.fNear = .1f;
    cfg2.fFar = 10000.f;
    cPlayer->SetParams(cfg2);
    cPlayer->BuildProj();
    cPlayer->BuildView();

    /*vpMain.MinDepth = cfg2.fNear;
    vpMain.MaxDepth = cfg2.fFar;*/
    vpMain.MinDepth = 0.f;
    vpMain.MaxDepth = 1.f;
    vpMain.TopLeftX = 0;
    vpMain.TopLeftY = 0;
    vpMain.Width    = cfg.CurrentWidth;
    vpMain.Height   = cfg.CurrentHeight2;

    // Save aspect for further use
    fAspect = cfg2.fAspect;

    // Deferred global constant buffer
    cbDeferredGlobalInst->CreateDefault(sizeof(cbDeferredGlobal));
    cbDeferredGlobal* dgData = (cbDeferredGlobal*)cbDeferredGlobalInst->Map();
    
    // Don't forget to update buffer
    DirectX::XMMATRIX mProjTmp = cPlayer->GetProjMatrix();
    float fHalfTanFov = tanf(DirectX::XMConvertToRadians(cfg2.FOV * .5)); // dtan(fov * .5)
    dgData->_TanAspect  = { fHalfTanFov * fAspect, -fHalfTanFov };
    dgData->_Texel      = { 1.f / cfg.CurrentWidth, 1.f / cfg.CurrentHeight2};
    dgData->_Far        = cfg2.fFar;
    dgData->PADDING0    = 0;
    dgData->_ProjValues = { 1.f / mProjTmp.r[0].m128_f32[0], 1.f / mProjTmp.r[1].m128_f32[1], mProjTmp.r[3].m128_f32[2], -mProjTmp.r[2].m128_f32[2] };
    dgData->_mInvView   = DirectX::XMMatrixIdentity();

    cbDeferredGlobalInst->Unmap();

    // Deferred light constant buffer
    cbDeferredLightInst->CreateDefault(sizeof(cbDeferredLight));
    cbDeferredLight* dlData = (cbDeferredLight*)cbDeferredLightInst->Map();

    dlData->_LightDiffuse = { .7f, .9f, 0.f };
    dlData->_LightData    = { 8.f, 1.5f };
    dlData->PADDING1      = 0.f;
    dlData->PADDING2      = { 0.f, 0.f };
    dlData->vPosition     = { 1.62895, 5.54253, 2.93616, 0 };

    cbDeferredLightInst->Unmap();

    // 
    cfg2.FOV = 90.f;
    cfg2.fNear = .1f;
    cfg2.fFar = 1.f;
    c2DScreen->SetParams(cfg2);
    c2DScreen->BuildProj();
    c2DScreen->BuildView();

    // 
    cfg2.fNear = .1f;
    cfg2.fFar  = 10000.f;
    cfg2.FOV = 90.f;
    cfg2.fAspect = 1.f;
    cLight->SetParams(cfg2);
    cLight->BuildProj();
    cLight->BuildView();

    /*vpDepth.MinDepth = cfg2.fNear;
    vpDepth.MaxDepth = cfg2.fFar;*/
    vpDepth.MinDepth = 0.f;
    vpDepth.MaxDepth = 1.f;
    vpDepth.TopLeftX = 0;
    vpDepth.TopLeftY = 0;
    vpDepth.Width    = bDepth->GetWidth();
    vpDepth.Height   = bDepth->GetHeight();

    // Load shader
    shSurface->LoadFile("shTestVS.cso", Shader::Vertex);
    shSurface->LoadFile("shTestPS.cso", Shader::Pixel);

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
    shGUI->AttachShader(shPostProcess, Shader::Vertex);
    shGUI->LoadFile("shGUIPS.cso", Shader::Pixel);

    // Deferred
    shDeferred->LoadFile("shDeferredVS.cso", Shader::Vertex);
    shDeferred->LoadFile("shDeferredPS.cso", Shader::Pixel);

    // Deferred final pass
    shDeferredFinal->AttachShader(shPostProcess, Shader::Vertex);
    shDeferredFinal->LoadFile("shDeferredFinalPS.cso", Shader::Pixel);

    // Deferred point light
    shDeferredPointLight->LoadFile("shNullVertexVS.cso"        , Shader::Vertex);
    shDeferredPointLight->LoadFile("shHemisphereHS.cso"        , Shader::Hull);
    shDeferredPointLight->LoadFile("shHemisphereDS.cso"        , Shader::Domain);
    shDeferredPointLight->LoadFile("shDeferredPointLightPS.cso", Shader::Pixel);

    // Screen-Space shadows
    shScreenSpaceShadows->AttachShader(shPostProcess, Shader::Vertex);
    shScreenSpaceShadows->LoadFile("shScreenSpaceShadowsPS.cso", Shader::Pixel);
    
    // Unit sphere
    shUnitSphere->AttachShader(shDeferredPointLight, Shader::Vertex);
    shUnitSphere->AttachShader(shDeferredPointLight, Shader::Hull);
    shUnitSphere->LoadFile("shUnitSphereDS.cso", Shader::Domain);
    shUnitSphere->LoadFile("shUnitSpherePS.cso", Shader::Pixel);

    // Unit sphere depth only
    shUnitSphereDepthOnly->AttachShader(shDeferredPointLight, Shader::Vertex);
    shUnitSphereDepthOnly->AttachShader(shDeferredPointLight, Shader::Hull);
    shUnitSphereDepthOnly->AttachShader(shUnitSphere, Shader::Domain);
    shUnitSphereDepthOnly->SetNullShader(Shader::Pixel);

    // Clean shaders
    shSurface->ReleaseBlobs();
    shTerrain->ReleaseBlobs();
    shSkeletalAnimations->ReleaseBlobs();
    shGUI->ReleaseBlobs();
    shVertexOnly->ReleaseBlobs();
    shSkybox->ReleaseBlobs();
    shTexturedQuad->ReleaseBlobs();
    shPostProcess->ReleaseBlobs();
    shSSLR->ReleaseBlobs();
    shDeferred->ReleaseBlobs();
    shDeferredFinal->ReleaseBlobs();
    shDeferredPointLight->ReleaseBlobs();
    shScreenSpaceShadows->ReleaseBlobs();
    shUnitSphere->ReleaseBlobs();
    shUnitSphereDepthOnly->ReleaseBlobs();

    // Create model
    mModel1 = new Model("Test model #1");
    //mModel1->LoadModel<Vertex_PNT_TgBn>("../Models/Sponza/sponza.obj");
    mModel1->LoadModel<Vertex_PNT_TgBn>("../Models/ScreenPlane.obj");
    //mModel1->LoadModel<Vertex_PNT_TgBn>("../Models/TransparencyTest1/TrasparancyTestJoined.obj");
    //mModel1->LoadModel<Vertex_PNT_TgBn>("../Models/PBRTest.obj");
    mModel1->EnableDefaultTexture();

    mModel2 = new Model("Test model #2");
    //mModel2->LoadModel("../Models/Dunes1.obj");

    mModel3 = new Model("Unit sphere");
    mModel3->LoadModel<Vertex_P>("../Models/UVMappedUnitSphere.obj");
    mModel3->DisableDefaultTexture();

    mUnitSphereUV = new Model("Unit sphere");
    mUnitSphereUV->LoadModel<Vertex_PT>("../Models/UVMappedUnitSphere.obj");
    mUnitSphereUV->DisableDefaultTexture();

    mScreenPlane = new Model("Screen plane model");
    mScreenPlane->LoadModel<Vertex_PT>("../Models/ScreenPlane.obj");
    mScreenPlane->DisableDefaultTexture();

    mSpaceShip = new Model("Space ship model");
    mSpaceShip->LoadModel<Vertex_PNT_TgBn>("../Models/Space Ship Guns1.obj");
    mSpaceShip->EnableDefaultTexture();

    mShadowTest1 = new Model("Shadow test model");
    mShadowTest1->LoadModel<Vertex_PNT_TgBn>("../Models/TestLevel2.obj");
    mShadowTest1->EnableDefaultTexture();

    // Create model instances
    // Test level
    mLevel1 = new ModelInstance();
    mLevel1->SetName("Level 1 Instance");
    mLevel1->SetWorldMatrix(DirectX::XMMatrixScaling(4, 4, 4));
    mLevel1->SetShader(shSurface);
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
    miSpaceShip->SetModel(mModel1); //mSpaceShip);
    miSpaceShip->SetShader(shSurface);
    miSpaceShip->SetWorldMatrix(DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(90.f)) *
                                DirectX::XMMatrixTranslation(-1, -.25, 1) *
                                //DirectX::XMMatrixScaling(.0625, .0625, .0625)
                                DirectX::XMMatrixScaling(40, 10, 40)
                                //DirectX::XMMatrixTranslation(-.5.f, -.5.f, 0.f)
    );
    //miSpaceShip->SetWorldMatrix(DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(180.f)) *
    //                            //DirectX::XMMatrixScaling(.0625, .0625, .0625)
    //                            DirectX::XMMatrixScaling(4, 4, 4)
    //                            //DirectX::XMMatrixTranslation(50.f, 10.f, 0.f)
    //);
    
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
}

void _DirectX::Unload() {
    // HBAO+
#if USE_HBAO_PLUS
    pAOContext->Release();
#endif

    // Release constant buffers
    cbDeferredGlobalInst->Release();
    cbDeferredLightInst->Release();

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
    shSurface->DeleteShaders();
    shTerrain->DeleteShaders();
    shGUI->DeleteShaders();
    shSkeletalAnimations->DeleteShaders();
    shVertexOnly->DeleteShaders();
    shSkybox->DeleteShaders();
    shTexturedQuad->DeleteShaders();
    shPostProcess->DeleteShaders();
    shSSLR->DeleteShaders();
    shDeferred->DeleteShaders();
    shDeferredFinal->DeleteShaders();
    shDeferredPointLight->DeleteShaders();
    shScreenSpaceShadows->DeleteShaders();
    shUnitSphere->DeleteShaders();
    shUnitSphereDepthOnly->DeleteShaders();

    // Release models
    mModel1->Release();
    mModel2->Release();
    mModel3->Release();
    mScreenPlane->Release();
    mShadowTest1->Release();
    mUnitSphereUV->Release();

    // Release meshes
#ifdef LOWPOLY_EXAMPLE
    mTerrainMesh->Release();
#endif

    // Release buffers
    bDepth->Release();
    bGBuffer->Release();
    bDeferred->Release();
    bShadows->Release();
    bSSLR->Release();
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
    gPhysicsEngine = new PhysicsEngine();
    
    // Window config
    WindowConfig winCFG;
    winCFG.Borderless  = false;
    winCFG.Windowed    = true;
    winCFG.ShowConsole = true;
    winCFG.Width = 1024;
    winCFG.Height = 540;
    winCFG.Title = L"Luna Engine";
    winCFG.Icon = L"Engine/Assets/Engine.ico";

    // Create window
    gWindow->Create(winCFG);
    winCFG = gWindow->GetCFG();

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
    gAudioDevice->Create(adCFG);

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

    // <strike>TODO: MSAA Support</strike>
    // TODO: Remove MSAA from here
    // TODO: Add TAA support
    dxCFG.MSAA = false;
    dxCFG.MSAA_Samples = 1;
    dxCFG.MSAA_Quality = 0;

    // Create device and swap chain
    if( gDirectX->ShowError(gDirectX->Create(dxCFG)) ) { return 1; }

    //std::cout << "Ansel avaliable: " << ansel::isAnselAvailable() << std::endl;

    // Debug report if we can
#ifdef _DEBUG
    if( gDirectX->gDebug ) gDirectX->gDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif

    // Include ImGUI
#if _DEBUG_BUILD
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init(gWindow->GetHWND());
    ImGui_ImplDX11_Init(gDirectX->gDevice, gDirectX->gContext);
    ImGui::StyleColorsDark();
#endif

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

    // Set physics object
    //PhysicsEngineChild::SetPhysicsEngine(gPhysicsEngine);

    // Load game data
    gDirectX->Load();

    // Start rendering loop
    gWindow->Loop();

    // Unload game
#if _DEBUG_BUILD
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
#endif

    gWindow->Destroy();
    gDirectX->Unload();
    gAudioDevice->Release();
}
