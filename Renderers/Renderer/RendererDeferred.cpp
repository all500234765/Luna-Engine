#include "RendererDeferred.h"

void RendererDeferred::Init() {
#pragma region Shaders
    // Load shaders
    shSurface = new Shader();
    shSurface->SetLayoutGenerator(LgMesh);
    shSurface->LoadFile("shSurfaceVS.cso", Shader::Vertex);
    shSurface->LoadFile("shSurfacePS.cso", Shader::Pixel);

    shVertexOnly = new Shader();
    shVertexOnly->SetLayoutGenerator(LayoutGenerator::LgMesh);
    shVertexOnly->LoadFile("shSimpleVS.cso", Shader::Vertex); // Change to shSurfaceVS
    shVertexOnly->SetNullShader(Shader::Pixel);

    shPostProcess = new Shader();
    shPostProcess->LoadFile("shPostProcessVS.cso", Shader::Vertex);
    shPostProcess->LoadFile("shPostProcessPS.cso", Shader::Pixel);

    shGUI = new Shader();
    shGUI->AttachShader(shPostProcess, Shader::Vertex);
    shGUI->LoadFile("shGUIPS.cso", Shader::Pixel);

    shCombinationPass = new Shader();
    shCombinationPass->AttachShader(shPostProcess, Shader::Vertex);
    shCombinationPass->LoadFile("shCombinationPassPS.cso", Shader::Pixel);


    // Release blobs
    shSurface->ReleaseBlobs();
    shVertexOnly->ReleaseBlobs();
    shPostProcess->ReleaseBlobs();
    shCombinationPass->ReleaseBlobs();
    shGUI->ReleaseBlobs();
#pragma endregion

#pragma region Render Targets
    rtGBuffer = new RenderTarget2DColor4DepthMSAA(Width(), Height(), 1, "GBuffer");
    rtGBuffer->Create(32);                                     // Depth
    rtGBuffer->CreateList(0, DXGI_FORMAT_R16G16B16A16_FLOAT,   // Direct
                             DXGI_FORMAT_R16G16B16A16_FLOAT,   // Normals
                             DXGI_FORMAT_R16G16B16A16_FLOAT,   // Ambient
                             DXGI_FORMAT_R16G16B16A16_FLOAT);  // Emission
    
    rtCombinedGBuffer = new RenderTarget2DColor4DepthMSAA(Width(), Height(), 1, "GBuffer combined");
    rtCombinedGBuffer->Create(32);                                     // Depth
    rtCombinedGBuffer->CreateList(0, DXGI_FORMAT_R16G16B16A16_FLOAT,   // Color
                                     DXGI_FORMAT_R16G16B16A16_FLOAT,   // Normals
                                     DXGI_FORMAT_R16G16B16A16_FLOAT,   // 
                                     DXGI_FORMAT_R16G16B16A16_FLOAT);  // 

    rtDepth = new RenderTarget2DDepthMSAA(Width(), Height(), 1, "World light shadowmap");
    rtDepth->Create(32);

    rtFinalPass = new RenderTarget2DColor1(Width(), Height(), 1, "Final Pass");
    rtFinalPass->CreateList(0, DXGI_FORMAT_R8G8B8A8_UNORM);

    rtTransparency = new RenderTarget2DColor2DepthMSAA(Width(), Height(), 1, "Transparency");
    rtTransparency->Create(32);                                     // Depth
    rtTransparency->CreateList(0, DXGI_FORMAT_R8G8B8A8_UNORM,       // Color
                                  DXGI_FORMAT_R16G16B16A16_FLOAT);  // Normals
#pragma endregion

#pragma region Default Textures
    s_material.mCubemap = new CubemapTexture();
    s_material.mCubemap->CreateFromDDS("../Textures/Cubemaps/environment.dds", false);
    
    s_material.tex.checkboard = new Texture();
    s_material.tex.checkboard->Load("../Textures/TileInverse.png", DXGI_FORMAT_R8G8B8A8_UNORM);
    s_material.tex.checkboard->SetName("Default texture");
    
    s_material.tex.bluenoise_rg_512 = new Texture();
    s_material.tex.bluenoise_rg_512->Load("../Textures/Noise/Blue/LDR_RG01_0.png", DXGI_FORMAT_R16G16_UNORM);
    s_material.tex.bluenoise_rg_512->SetName("Bluenoise RG");
#pragma endregion

#pragma region Samplers
    {
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
        s_material.sampl.point = new Sampler();
        s_material.sampl.point->Create(pDesc);

        // Compare point sampler
        pDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
        pDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        s_material.sampl.point_comp = new Sampler();
        s_material.sampl.point_comp->Create(pDesc);

        // Linear sampler
        pDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        pDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        s_material.sampl.linear = new Sampler();
        s_material.sampl.linear->Create(pDesc);

        // Compare linear sampler
        pDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        pDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
        s_material.sampl.linear_comp = new Sampler();
        s_material.sampl.linear_comp->Create(pDesc);
    }
#pragma endregion

#pragma region Effects
    gHDRPostProcess             = new HDRPostProcess;
    gSSAOPostProcess            = new SSAOPostProcess;
    gSSLRPostProcess            = new SSLRPostProcess;
    gCascadeShadowMapping       = new CascadeShadowMapping;
    //gCoverageBuffer             = new CoverageBuffer;
    gOrderIndendentTransparency = new OrderIndendentTransparency;
#pragma endregion

#pragma region Blend states
    s_states.blend.normal  = new BlendState;
    s_states.blend.add     = new BlendState;

    {
        D3D11_BLEND_DESC pDesc;
        pDesc.RenderTarget[0].BlendEnable = true;
        pDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        pDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        pDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        pDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        pDesc.AlphaToCoverageEnable = false;
        pDesc.IndependentBlendEnable = false;

        s_states.blend.normal->Create(pDesc, { 1.f, 1.f, 1.f, 1.f });

        // 

        //s_states.blend.add->Create(pDesc, { 1.f, 1.f, 1.f, 1.f });
    }
#pragma endregion

#pragma region Depth stencil states
    s_states.depth.normal = new DepthStencilState;
    s_states.depth.norw = new DepthStencilState;

    {
        D3D11_DEPTH_STENCIL_DESC pDesc;
        pDesc.DepthEnable = true;
        pDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        pDesc.DepthFunc = D3D11_COMPARISON_GREATER; //D3D11_COMPARISON_LESS
        
        pDesc.StencilEnable = !true;
        pDesc.StencilReadMask = 0xFF;
        pDesc.StencilWriteMask = 0xFF;

        // Stencil operations if pixel is front-facing.
        pDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        pDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        pDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        pDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        // Stencil operations if pixel is back-facing.
        pDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        pDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        pDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        pDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        
        s_states.depth.normal->Create(pDesc, 0);

        pDesc.DepthEnable = false;
        pDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        s_states.depth.norw->Create(pDesc, 0);
    }
#pragma endregion

#pragma region Raster states
    s_states.raster.normal          = new RasterState;
    s_states.raster.wire            = new RasterState;
    s_states.raster.normal_scissors = new RasterState;
    s_states.raster.wire_scissors   = new RasterState;

    {
        D3D11_RASTERIZER_DESC pDesc;
        ZeroMemory(&pDesc, sizeof(D3D11_RASTERIZER_DESC));
        pDesc.AntialiasedLineEnable = true;
        pDesc.CullMode              = D3D11_CULL_NONE;
        pDesc.DepthBias             = 0;
        pDesc.DepthBiasClamp        = 0.0f;
        pDesc.DepthClipEnable       = true;
        pDesc.FillMode              = D3D11_FILL_SOLID;
        pDesc.FrontCounterClockwise = false;
        pDesc.MultisampleEnable     = true;
        pDesc.ScissorEnable         = false;
        pDesc.SlopeScaledDepthBias  = 0.0f;

        // Normal
        s_states.raster.normal->Create(pDesc);

        // Normal + Scissors
        pDesc.ScissorEnable = true;
        s_states.raster.normal_scissors->Create(pDesc);

        // Wireframe + Scissors
        pDesc.FillMode = D3D11_FILL_WIREFRAME;
        s_states.raster.wire->Create(pDesc);

        // Wireframe
        pDesc.ScissorEnable = false;
        s_states.raster.wire->Create(pDesc);
    }
#pragma endregion

    cbTransform = new ConstantBuffer();
    cbTransform->CreateDefault(sizeof(TransformBuff));

    IdentityTransf = new TransformComponent;
    IdentityTransf->fAcceleration = 0.f;
    IdentityTransf->fVelocity     = 0.f;
    IdentityTransf->vDirection    = { 0.f, 0.f, 0.f };
    IdentityTransf->vPosition     = { 0.f, 0.f, 0.f };
    IdentityTransf->vRotation     = { 0.f, 0.f, 0.f };
    IdentityTransf->vScale        = { 1.f, 1.f, 1.f };
    IdentityTransf->mWorld        = DirectX::XMMatrixIdentity();
}

void RendererDeferred::Render() {
    ScopedRangeProfiler q0(L"Deferred Renderer");
    mScene = Scene::Current();
    if( !mScene ) {
        printf_s("[%s]: No scene is bound!\n", __FUNCTION__);
        return;
    }

    // Update player camera
    mScene->UpdateCameraData(0);
    mScene->GetCamera(0)->BuildView();
    //mScene->UpdateCameraData(0);
    //mScene->UpdateCameraData(1);

    {
        ScopedRangeProfiler q(L"Geometry rendering");
        s_states.blend.normal->Bind();
        s_states.depth.normal->Bind();

        Shadows();
        GBuffer();
        OIT();
    }

    {
        ScopedRangeProfiler q(L"Screen-Space");

        // Light calculation
        SSAO();
        SSLR();
        SSLF();
        FSSSSS();
        Deferred();

        // Combination pass
        s_states.depth.norw->Bind();
        Combine();
    }

    {
        ScopedRangeProfiler q(L"Final post processing");

        // Final post-processing
        HDR();
        Final();
    }

    {
        ScopedRangeProfiler q(L"Unbind resources");

        // HDR Post Processing
        // Swap buffer data
        gHDRPostProcess->End();

        // SSAO
        // Unbind for further use
        gSSAOPostProcess->End();
    }
}

void RendererDeferred::FinalScreen() {
    shGUI->Bind();

    // Bind resources
    s_material.sampl.point->Bind(Shader::Pixel);
    rtFinalPass->Bind(0u, Shader::Pixel, 0, false);

    BindOrtho();

    // 
    DXDraw(6, 0);
}

void RendererDeferred::Resize(float W, float H) {
    // TODO: 
}

void RendererDeferred::Release() {
    // Shaders
    SAFE_RELEASE(shSurface);
    SAFE_RELEASE(shVertexOnly);
    SAFE_RELEASE(shPostProcess);
    SAFE_RELEASE(shCombinationPass);
    SAFE_RELEASE(shGUI);

    // Render Targets
    SAFE_RELEASE(rtTransparency);
    SAFE_RELEASE(rtCombinedGBuffer);
    SAFE_RELEASE(rtGBuffer);
    SAFE_RELEASE(rtDepth);
    SAFE_RELEASE(rtFinalPass);

    // Textures
    SAFE_RELEASE(s_material.tex.bluenoise_rg_512);
    SAFE_RELEASE(s_material.tex.checkboard);
    SAFE_RELEASE(s_material.mCubemap);

    // Samplers
    SAFE_RELEASE(s_material.sampl.point      );
    SAFE_RELEASE(s_material.sampl.point_comp );
    SAFE_RELEASE(s_material.sampl.linear     );
    SAFE_RELEASE(s_material.sampl.linear_comp);

    // Effects
    SAFE_DELETE(gHDRPostProcess            );
    SAFE_DELETE(gSSAOPostProcess           );
    SAFE_DELETE(gSSLRPostProcess           );
    SAFE_DELETE(gCascadeShadowMapping      );
    //SAFE_DELETE(gCoverageBuffer            );
    SAFE_DELETE(gOrderIndendentTransparency);
    
    // States
    SAFE_RELEASE(s_states.blend.normal );
    SAFE_RELEASE(s_states.blend.add    );
    SAFE_RELEASE(s_states.raster.normal);
    SAFE_RELEASE(s_states.raster.wire  );
    SAFE_RELEASE(s_states.raster.normal_scissors);
    SAFE_RELEASE(s_states.raster.wire_scissors  );
    SAFE_RELEASE(s_states.depth.normal );
    SAFE_RELEASE(s_states.depth.norw   );

    // Buffers
    SAFE_RELEASE(cbTransform);

    // Other
    SAFE_DELETE(IdentityTransf);
}

void RendererDeferred::ImGui() {

}

void RendererDeferred::ClearMainRT() {
    gDirectX->gContext->ClearRenderTargetView(gDirectX->gRTV, s_clear.black_void2);
    gDirectX->gContext->ClearDepthStencilView(gDirectX->gDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.f, 0);
}

void RendererDeferred::Shadows() {
    ScopedRangeProfiler s1(L"World light");

    // Bind render target
    rtDepth->Bind();
    rtDepth->Clear(0.f, 0, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);

    //gDirectX->gContext->OMSetDepthStencilState(pDSS_Default, 1);
    gDirectX->gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    shVertexOnly->Bind();
    
    // 
    uint32_t old = mScene->GetActiveCamera();
    mScene->SetActiveCamera(1);
    
    {
        mScene->BindCamera(1, Shader::Vertex, 0); // Light camera
        mScene->RenderOpaque(RendererFlags::ShadowPass, Shader::Vertex);
    }

    mScene->SetActiveCamera(old);
}

void RendererDeferred::GBuffer() {
    ScopedRangeProfiler s1(L"Render GBuffer");

    // Bind render target
    rtGBuffer->Bind();
    rtGBuffer->Clear(0.f, 0, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
    rtGBuffer->Clear(s_clear.black_void2);

    // Bind shader
    shSurface->Bind();

    // Bind default material textures
    for( uint32_t i = 0; i < 6; i++ ) {
        s_material.tex.checkboard->Bind(Shader::Pixel, i);
        s_material.sampl.linear->Bind(Shader::Pixel, i);
    }

    // Bind depth buffer
    rtDepth->Bind(0u, Shader::Pixel, 6);
    s_material.sampl.linear_comp->Bind(Shader::Pixel, 6);

    // Bind noise texture
    s_material.tex.bluenoise_rg_512->Bind(Shader::Pixel, 7);
    s_material.sampl.point->Bind(Shader::Pixel, 7);

    // Bind cubemap
    s_material.mCubemap->Bind(Shader::Pixel, 8);
    s_material.sampl.linear->Bind(Shader::Pixel, 8);

    // Bind CBs
    mScene->BindCamera(0, Shader::Vertex, 1); // Main camera
    mScene->BindCamera(1, Shader::Vertex, 2); // Shadow camera

    // Bind material layers
    uint32_t old_ml = mScene->GetEnabledMaterialLayers();
    mScene->SetLayersState(Scene::Default);

    uint32_t old = mScene->GetActiveCamera();
    mScene->SetActiveCamera(0);
    
    {
        mScene->Render(RendererFlags::OpaquePass, Shader::Vertex);
    }

    // Restore states
    mScene->SetActiveCamera(old);
    mScene->SetLayersState(old_ml);

    // Unbind textures
    LunaEngine::PSDiscardSRV<8>();
}

void RendererDeferred::OIT() {
    // 
    gOrderIndendentTransparency->Begin(rtGBuffer);

    // Resolve MSAA
    rtGBuffer->MSAAResolve();

    // 
    UINT dc = gDrawCallCount + gDrawCallInstanceCount + gDispatchCallCount;

    // 
    mScene->BindCamera(0, Shader::Vertex, 1); // Main camera
    mScene->Render(RendererFlags::OpacityPass, Shader::Vertex);

    // 
    UINT ddc = (gDrawCallCount + gDrawCallInstanceCount + gDispatchCallCount) - dc;

    // Done
    if( ddc > 0 ) {
        gOrderIndendentTransparency->End(rtTransparency, gOITSettings);
    } else {
        // Unbind
        ID3D11UnorderedAccessView *pEmptyUAV[2] = { nullptr, nullptr };
        ID3D11RenderTargetView *pEmptyRTV[2] = { nullptr, nullptr };
        gDirectX->gContext->OMSetRenderTargetsAndUnorderedAccessViews(2, pEmptyRTV, nullptr, 2, 2, pEmptyUAV, 0);

        //TopologyState::Pop();
        DepthStencilState::Pop();
        RasterState::Pop();
        BlendState::Pop();

        // L"Order Independent Transparency"
        RangeProfiler::End();
    }
}

void RendererDeferred::CoverageBuffer() {

}

void RendererDeferred::Deferred() {

}

void RendererDeferred::SSAO() {

    gSSAOArgs._Blur = true;
    gSSAOArgs._CameraFar = mScene->GetCamera(0)->cCam->fFar;
    gSSAOArgs._CameraNear = mScene->GetCamera(0)->cCam->fNear;
    gSSAOArgs._mProj = mScene->GetCamera(0)->cCam->mProj;
    gSSAOArgs._mView = mScene->GetCamera(0)->cCam->mView;
    gSSAOArgs._OffsetRad = 1.f;
    gSSAOArgs._Power = 2.f;
    gSSAOArgs._Radius = 1.5f;

    // Opaque
    gSSAOPostProcess->Begin(rtGBuffer, gSSAOArgs);

    // TODO: Add separate RT for OIT
    // Transparent
    //gSSAOPostProcess->Begin(rtTransparency, gSSAOArgs);
}

void RendererDeferred::SSLR() {

}

void RendererDeferred::SSLF() {

}

void RendererDeferred::FSSSSS() {

}

void RendererDeferred::Combine() {
    ScopedRangeProfiler q("Combine Pass");

    shCombinationPass->Bind();

    BlendState::Push();

    s_states.blend.normal->Bind();
    rtCombinedGBuffer->Bind();
    rtCombinedGBuffer->Clear(s_clear.black_void2);

    // 
    BindOrtho();

    // Bind resources
    s_material.sampl.linear->Bind(Shader::Pixel, 0);

    rtGBuffer->Bind(1, Shader::Pixel, 0);
    rtGBuffer->Bind(3, Shader::Pixel, 1);
    gSSAOPostProcess->BindAO(Shader::Pixel, 2);

    // Draw call
    DXDraw(6, 0);

    // 
    BlendState::Pop();
    LunaEngine::PSDiscardSRV<3>();

    // Copy result
    gDirectX->gContext->CopyResource(rtGBuffer->GetBufferTexture<0>(), rtCombinedGBuffer->GetBufferTexture<0>());

}

void RendererDeferred::HDR() {
    CameraComponent* cam = mScene->GetCamera(0)->cCam;
    float fNear = cam->fNear;
    float fFar  = cam->fFar;
    float fQ    = fFar / (fNear - fFar);
    
    float White             = 21.53f;
    float MidGray           = 20.863f;
    float gAdaptation       = 5.f;
    float gBloomScale       = 4.f;
    float gBloomThres       = 10.f;
    float gFarStart         = 0.f;
    float gFarRange         = 60.f;
    float gBokehThreshold   = 0.f;
    float gBokehColorScale  = 0.f;
    float gBokehRadiusScale = 0.f;

    const float gFarScale = 100.f;
    
    uint1 _RenderFlags = 0;
    bool gRenderSSLR          = true;
    bool gRenderSSAO          = true;
    bool gRenderEyeAdaptation = true;
    bool gRenderDepthOfField  = true;
    bool gRenderBloom         = true;
    bool gRenderBokeh         = true;
    bool gRenderDiffuse       = true;
    bool gRenderLight         = true;

    // Update flags
    //                        0            1            2
    std::array<bool, 8> _val{ gRenderSSLR, gRenderSSAO, gRenderEyeAdaptation,
    //  3                    4             5             6               7
        gRenderDepthOfField, gRenderBloom, gRenderBokeh, gRenderDiffuse, gRenderLight };

    for( size_t i = 0; i < _val.size(); i++ ) if( _val[i] ) _RenderFlags |= 1 << i;

    // Update constant buffers
    FinalPassInst *__q = gHDRPostProcess->MapFinalPass();
        __q->_LumWhiteSqr     = White * White * MidGray * MidGray;
        __q->_MiddleGrey      = MidGray;
        __q->_BloomScale      = gBloomScale;
        __q->_ProjectedValues = { fNear * fQ, fQ };
        __q->_DoFFarValues    = { gFarStart * gFarScale, 1.f / (gFarRange * gFarScale) };
        __q->_BokehThreshold  = gBokehThreshold;
        __q->_ColorScale      = gBokehColorScale;
        __q->_RadiusScale     = gBokehRadiusScale;
        __q->_RenderFlags     = _RenderFlags;
    gHDRPostProcess->UnmapFinalPass();

    const WindowConfig& cfg = Window::Current()->GetCFG();
    DownScaleInst* __c = gHDRPostProcess->MapDownScale();
        __c->_Res            = { static_cast<uint32_t>(cfg.CurrentWidth / 4), static_cast<uint32_t>(cfg.CurrentHeight / 4) };
        __c->_Domain         = __c->_Res.x * __c->_Res.y;
        __c->_GroupSize      = __c->_Domain / 1024;
        __c->_Adaptation     = gAdaptation / (float)gDirectX->GetConfig().RefreshRate;
        __c->_BloomThreshold = gBloomThres;
    gHDRPostProcess->UnmapDownScale();

    gHDRPostProcess->Begin(rtGBuffer);
}

void RendererDeferred::Final() {
    rtFinalPass->Bind();

    BindOrtho();

    shPostProcess->Bind();

    // HDR Post Processing; Eye Adaptation; Bloom; Depth of Field
    gHDRPostProcess->BindFinalPass(Shader::Pixel, 0);
    gHDRPostProcess->BindLuminance(Shader::Pixel, 4);
    gHDRPostProcess->BindBloom(Shader::Pixel, 5);
    gHDRPostProcess->BindBlur(Shader::Pixel, 6);

    gSSAOPostProcess->BindAO(Shader::Pixel, 8);

    rtGBuffer->Bind(0u, Shader::Pixel, 7, false);

    s_material.sampl.linear->Bind(Shader::Pixel, 5);

    // 
    BindOrtho();

    // Diffuse
    rtCombinedGBuffer->Bind(1u, Shader::Pixel, 0, false);
    s_material.sampl.point->Bind(Shader::Pixel, 0);

    // SSLR
    //gContext->PSSetShaderResources(1, 1, &_SSLRBf->pSRV);
    s_material.sampl.point->Bind(Shader::Pixel, 1);

    // Shadows
    //gContext->PSSetShaderResources(2, 1, &_Shadow->pSRV);
    //sPoint->Bind(Shader::Pixel, 2);

    // Deferred
    //gContext->PSSetShaderResources(3, 1, &_ColorD->pSRV);
    //sPoint->Bind(Shader::Pixel, 3);

    DXDraw(6, 0);

    LunaEngine::PSDiscardSRV<10>();
}

void RendererDeferred::BindOrtho() {
    // Bind matrices
    mScene->DefineCameraOrtho(2, .1f, 10.f, Width(), Height());
    mScene->BindCamera(2, Shader::Vertex, 1);

    IdentityTransf->Bind(cbTransform, Shader::Vertex, 0);
}
