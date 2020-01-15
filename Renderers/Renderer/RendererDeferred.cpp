#include "pc.h"
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

    shSimpleGUI = new Shader();
    shSimpleGUI->LoadFile("shTexturedQuadAutoVS.cso", Shader::Vertex);
    shSimpleGUI->AttachShader(shGUI, Shader::Pixel);

    shVolumetricLight = new Shader();
    shVolumetricLight->LoadFile("shVolumetricLightCS.cso", Shader::Compute);

    // Release blobs
    shSurface->ReleaseBlobs();
    shVertexOnly->ReleaseBlobs();
    shPostProcess->ReleaseBlobs();
    shCombinationPass->ReleaseBlobs();
    shSimpleGUI->ReleaseBlobs();
    shGUI->ReleaseBlobs();
    shVolumetricLight->ReleaseBlobs();
#pragma endregion

#pragma region Render Targets
    rtGBuffer = new RenderTarget2DColor4DepthMSAA(Width(), Height(), 1, "[RT]: GBuffer");
    rtGBuffer->Create(32);                                     // Depth
    rtGBuffer->CreateList(0, DXGI_FORMAT_R16G16B16A16_FLOAT,   // Direct
                             DXGI_FORMAT_R16G16B16A16_FLOAT,   // Normals
                             DXGI_FORMAT_R16G16B16A16_FLOAT,   // Ambient
                             DXGI_FORMAT_R16G16B16A16_FLOAT);  // Emission
    
    rtCombinedGBuffer = new RenderTarget2DColor4DepthMSAA(Width(), Height(), 1, "[RT]: GBuffer combined");
    rtCombinedGBuffer->Create(32);                                     // Depth
    rtCombinedGBuffer->CreateList(0, DXGI_FORMAT_R16G16B16A16_FLOAT,   // Direct light
                                     DXGI_FORMAT_R16G16B16A16_FLOAT,   // Normals
                                     DXGI_FORMAT_R16G16B16A16_FLOAT,   // Ambient light
                                     DXGI_FORMAT_R16G16B16A16_FLOAT);  // Emission

    rtDepth = new RenderTarget2DDepthMSAA(2048, 2048, 1, "[RT]: World light shadowmap");
    rtDepth->Create(32);

    rtFinalPass = new RenderTarget2DColor1(Width(), Height(), 1, "[RT]: Final Pass");
    rtFinalPass->CreateList(0, DXGI_FORMAT_R8G8B8A8_UNORM);

    rtDeferred = new RenderTarget2DColor2MSAA(Width(), Height(), 1, "[RT]: Deferred");
    rtDeferred->CreateList(0, DXGI_FORMAT_R11G11B10_FLOAT, DXGI_FORMAT_R11G11B10_FLOAT);

    rtTransparency = new RenderTarget2DColor4DepthMSAA(Width(), Height(), 1, "[RT]: Transparency");
    rtTransparency->Create(32);                                     // Depth
    rtTransparency->CreateList(0, DXGI_FORMAT_R8G8B8A8_UNORM,       // Direct light
                                  DXGI_FORMAT_R16G16B16A16_FLOAT,   // Normals
                                  DXGI_FORMAT_R16G16B16A16_FLOAT,   // Ambient light
                                  DXGI_FORMAT_R16G16B16A16_FLOAT);  // Emission
#pragma endregion

#pragma region Default Textures
    s_material.mCubemap = new Texture("../Textures/Cubemap default.dds");
    
    s_material.ti.tex.checkboard = new Texture("../Textures/DefaultTileBig.png");
    s_material.ti.tex.checkboard->SetName("Default texture");
    
    s_material.ti.tex.bluenoise_rg_512 = new Texture("../Textures/Noise/Blue/LDR_RG01_0.png", 0u, "Blue noise RG", 1u, DXGI_FORMAT_R16G16_UNORM);

    s_material.ti.tex.tile_normal = new Texture("../Textures/Normal.png");
    s_material.ti.tex.tile_normal->SetName("Tile normalmap");

    s_material.ti.tex.black = new Texture("../Textures/Black.png");
    s_material.ti.tex.black->SetName("Black");

    s_material.ti.tex.white = new Texture("../Textures/White.png");
    s_material.ti.tex.white->SetName("White");
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
        pDesc.RenderTarget[0].BlendEnable           = true;
        pDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        pDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;

        pDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        
        pDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
        pDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_SRC_ALPHA;
        
        pDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        pDesc.AlphaToCoverageEnable  = false;
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
        s_states.raster.wire_scissors->Create(pDesc);
    }
#pragma endregion
    
    // Volumetric Light
    mVolumetricLightAccum = new Texture(tf_dim_2 | tf_UAV, DXGI_FORMAT_R16G16B16A16_FLOAT, 
                                        ceil(Width() / 2.f), ceil(Height() / 2.f), 1u, 1u, "Volumetric Light Accumulation");

    cbVolumetricSettings = new ConstantBuffer();
    cbVolumetricSettings->CreateDefault(sizeof(VolumetricSettings));
    
    // Default CSM Settings
    gCSMArgs._Antiflicker = true;
    gCSMArgs._CascadeNum = 3;
    gCSMArgs._Resolution = 2048;
    gCSMArgs._MSAA = false;
    gCSMArgs._MSAALevel = 8;
    gCSMArgs._CascadeRange[0] = .1f;
    gCSMArgs._CascadeRange[1] = 10.f;
    gCSMArgs._CascadeRange[2] = 25.f;
    gCSMArgs._CascadeRange[3] = 100.f;

    // Deferred
    cbDeferredGlobal = new ConstantBuffer();
    cbDeferredGlobal->CreateDefault(sizeof(DeferredGlobal));
    cbDeferredGlobal->SetName("[Deferred::CB]: Deferred Global");

    cbTransform = new ConstantBuffer();
    cbTransform->CreateDefault(sizeof(TransformBuff));
    cbTransform->SetName("[Deferred::CB]: Transform");
    
    // Default Identity Transformation Component for internal use
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

        (bIsWireframe ? s_states.raster.wire : s_states.raster.normal)->Bind();
        if( bIsWireframe ) rtGBuffer->Clear(s_clear.black_void2);
        
        GBuffer();

        OIT();
        s_states.raster.normal->Bind();
    }

    {
        ScopedRangeProfiler q(L"Screen-Space");

        // Light calculation
        SSAO();
        SSLR();
        SSLF();
        FSSSSS();
        Deferred();
        VolumetricLight();

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

void RendererDeferred::Resize() {
    // Resize RTs
    rtCombinedGBuffer->Resize(Width(), Height(), 1u);
    rtTransparency->Resize(Width(), Height(), 1u);
    rtFinalPass->Resize(Width(), Height(), 1u);
    rtDeferred->Resize(Width(), Height(), 1u);
    rtGBuffer->Resize(Width(), Height(), 1u);
    mVolumetricLightAccum->Resize(ceil(Width() / gVolumetricSettings._Scaling.x), ceil(Height() / gVolumetricSettings._Scaling.y), 1u);

    EffectBase::ResizeGlobal(Width(), Height()); // TODO: EffectBase::ResizeGlobal
    gHDRPostProcess->Resize(Width(), Height());
    gSSAOPostProcess->Resize(Width(), Height());
    //gSSLRPostProcess->Resize(Width(), Height());
    gOrderIndendentTransparency->Resize(Width(), Height());
}

void RendererDeferred::Release() {
    // Shaders
    SAFE_RELEASE(shSurface);
    SAFE_RELEASE(shVertexOnly);
    SAFE_RELEASE(shPostProcess);
    SAFE_RELEASE(shCombinationPass);
    SAFE_RELEASE(shGUI);
    SAFE_RELEASE(shSimpleGUI);
    SAFE_RELEASE(shVolumetricLight);

    // Render Targets
    SAFE_RELEASE(rtTransparency);
    SAFE_RELEASE(rtCombinedGBuffer);
    SAFE_RELEASE(rtGBuffer);
    SAFE_RELEASE(rtDepth);
    SAFE_RELEASE(rtFinalPass);
    SAFE_RELEASE(rtDeferred);

    // Textures
    SAFE_RELEASE(s_material.ti.tex.bluenoise_rg_512);
    SAFE_RELEASE(s_material.ti.tex.tile_normal);
    SAFE_RELEASE(s_material.ti.tex.checkboard);
    SAFE_RELEASE(s_material.ti.tex.black);
    SAFE_RELEASE(s_material.ti.tex.white);
    SAFE_RELEASE(s_material.mCubemap);
    SAFE_RELEASE(mVolumetricLightAccum);

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
    SAFE_RELEASE(cbDeferredGlobal);

    // Other
    SAFE_DELETE(IdentityTransf);
}

void RendererDeferred::ImGui() {
    ScopedRangeProfiler s0(L"ImGui");

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    // Create debug window
    ImGui::Begin("Debug");

    // GPU Memory usage
    DXGI_QUERY_VIDEO_MEMORY_INFO MemoryUsage = gDirectX->GPUUsage();

    ImGui::Text("Memory usage:\n\tGPU:\n\t\tCurrent: %u\n\t\tAvaliable: %u\n\t\tBudget: %u\n\t\tReservation: %u",
                MemoryUsage.CurrentUsage, MemoryUsage.AvailableForReservation, MemoryUsage.Budget, MemoryUsage.CurrentReservation);

    // CPU Memory usage
    MemoryUsage = gDirectX->CPUUsage();
    ImGui::Text("\tCPU:\n\t\tCurrent: %u\n\t\tAvaliable: %u\n\t\tBudget: %u\n\t\tReservation: %u",
                MemoryUsage.CurrentUsage, MemoryUsage.AvailableForReservation, MemoryUsage.Budget, MemoryUsage.CurrentReservation);

    // 
    if( ImGui::Button("Wireframe") ) {
        bIsWireframe ^= true;
        //(bIsWireframe ? s_states.raster.wire : s_states.raster.normal)->Bind();
    }

    bDebugHUD ^= ImGui::Button("Debug buffers");

    // Test
    //ImGui::SliderFloat("Camera speed", &fSpeed, 0.f, 2000.f);
    //ImGui::SliderFloat("Light radius", &LightPos.w, 0.f, 200.f);

    // Rendering flags
    uint1 _RenderFlags = 0;
    static bool gRenderSSLR = true;
    static bool gRenderSSAO = true;
    static bool gRenderEyeAdaptation = true;
    static bool gRenderDepthOfField = true;
    static bool gRenderBloom = true;
    static bool gRenderBokeh = true;
    static bool gRenderDiffuse = true;
    static bool gRenderLight = true;

    // HDR; Eye Adaptation; Bloom; Bokeh; DoF
    static float White = 21.53f;
    static float MidGray = 20.863f;
    static float gAdaptation = 5.f;
    static float gBloomScale = 4.f;
    static float gBloomThres = 10.f;
    static float gFarStart = 0.f;
    static float gFarRange = 60.f;
    static float gBokehThreshold = 0.f;
    static float gBokehColorScale = 0.f;
    static float gBokehRadiusScale = 0.f;

    // SSAO
    static float gSSAOOffsetRad = 10.f;
    static float gSSAORadius = 13.f;
    static float gSSAOPower = 5.f;
    static bool  gSSAOBlur = true;

    // SSLR
    static float gViewAngleThreshold = .2f;
    static float gEdgeDistThreshold = .45f;
    static float gReflScale = 1.f;
    static float gDepthBias = .5f;

    // MSAA
    static int gMSAAMaxLevel = 3;
    static bool gMSAACheckbox = false;
    if( gMSAACheckbox != rtGBuffer->IsMSAAEnabled() )
        gMSAACheckbox = rtGBuffer->IsMSAAEnabled();

    if( ImGui::Checkbox("MSAA GBuffer", &gMSAACheckbox) ) {
        size_t w = rtGBuffer->GetWidth();
        size_t h = rtGBuffer->GetHeight();

        if( gMSAACheckbox ) rtGBuffer->EnableMSAA();
        else                rtGBuffer->DisableMSAA();

        rtGBuffer->Resize((UINT)w, (UINT)h);
    }

    // Choose number of samples
    if( gMSAACheckbox ) {
        if( ImGui::SliderInt("MSAA Max Samples", &gMSAAMaxLevel, 1, 5) ) {
            // Re-create RT
            size_t w = rtGBuffer->GetWidth();
            size_t h = rtGBuffer->GetHeight();

            rtGBuffer->SetMSAAMaxLevel(1u << (UINT)gMSAAMaxLevel);
            rtGBuffer->Resize((UINT)w, (UINT)h);
        }
    }

    // Volumetric Light
    static float gVLGScattering = .1f;
    static float gVLScaling     = 2.f;

    ImGui::Text("Volumetric Light Settings");
    ImGui::SliderFloat("G Scattering", &gVLGScattering, -1.f, 1.f);

    // OIT
    static float gMinFadeDist = 30.f;
    static float gMaxFadeDist = 30.f;

    ImGui::Text("Order Independent Transparency Settings");
    //ImGui::PlotLines("ms", &PValGetter, gOITPlot->mPlot.data(), (int)gOITPlot->mPlot.size());
    ImGui::SliderFloat("Min fade distance", &gMinFadeDist, 0.f, 50.f);
    ImGui::SliderFloat("Max fade distance", &gMaxFadeDist, 0.f, 50.f);

    // C-Buffer
    static int gCBuffScale = 2;

    ImGui::Text("Coverage Buffer Settings");
    ImGui::SliderInt("rcp Scaling", &gCBuffScale, 0, 3);

    // CSM
    gCSMArgs._Antiflicker = true;
    gCSMArgs._CascadeNum = 3;
    gCSMArgs._Resolution = 2048;
    gCSMArgs._MSAA = false;
    gCSMArgs._MSAALevel = 8;

    ImGui::Text("CSM Settings");
    ImGui::SliderFloat3("Range", gCSMArgs._CascadeRange, 0.f, 100.f);

    ImGui::Text("Render Flags");
    ImGui::Checkbox("Render Diffuse", &gRenderDiffuse);
    ImGui::Checkbox("Render Light", &gRenderLight);
    ImGui::Checkbox("Render Eye Adaptation", &gRenderEyeAdaptation);
    ImGui::Checkbox("Render Bloom", &gRenderBloom);
    ImGui::Checkbox("Render Depth Of Field", &gRenderDepthOfField);
    ImGui::Checkbox("Render Bokeh", &gRenderBokeh);
    ImGui::Checkbox("Render SSLR", &gRenderSSLR);
    ImGui::Checkbox("Blur", &gSSAOBlur);
    ImGui::Checkbox("Render SSAO", &gRenderSSAO);

    ImGui::Text("SSLR Settings");
    ImGui::SliderFloat("View angle Threshold", &gViewAngleThreshold, -.25f, 1.f);
    ImGui::SliderFloat("Edge distance Threshold", &gEdgeDistThreshold, 0.f, .999f);
    ImGui::SliderFloat("Reflect scale", &gReflScale, 0.f, 1.f);
    ImGui::SliderFloat("Depth bias", &gDepthBias, 0.f, 1.5f);

    ImGui::Text("SSAO Settings");
    //ImGui::PlotLines("ms", &PValGetter, gSSAOPlot->mPlot.data(), (int)gSSAOPlot->mPlot.size());
    //ImGui::Text("Min: %f\nMax: %f", gSSAOPlot->mMinMax.x, gSSAOPlot->mMinMax.y);

    ImGui::SliderFloat("Offset radius", &gSSAOOffsetRad, 0.f, 20.f);
    ImGui::SliderFloat("Radius", &gSSAORadius, 0.f, 50.f);
    ImGui::SliderFloat("Power", &gSSAOPower, .1f, 5.f);

    ImGui::Text("HDR Settings");
    //ImGui::PlotLines("ms", &PValGetter, gHDRPlot->mPlot.data(), (int)gHDRPlot->mPlot.size());
    //ImGui::Text("Min: %f\nMax: %f", gHDRPlot->mMinMax.x, gHDRPlot->mMinMax.y);
    ImGui::SliderFloat("White", &White, 0.f, 60.f);
    ImGui::SliderFloat("Middle Gray", &MidGray, 0.f, 60.f);
    ImGui::SliderFloat("Adaptation rate", &gAdaptation, 0.f, 10.f);
    ImGui::SliderFloat("Bloom Scale", &gBloomScale, 0.f, 4.f);
    ImGui::SliderFloat("Bloom Threshold", &gBloomThres, 0.f, 10.f);
    ImGui::SliderFloat("Far start", &gFarStart, 0.f, 400.f);
    ImGui::SliderFloat("Far range", &gFarRange, 1.f, 150.f);
    ImGui::SliderFloat("Bokeh Threshold", &gBokehThreshold, 0.f, 25.f);
    ImGui::SliderFloat("Bokeh Color Scale", &gBokehColorScale, 0.f, 1.f);
    ImGui::SliderFloat("Bokeh Radius Scale", &gBokehRadiusScale, 0.f, 1.f);

    // Update flags
    //                        0            1            2
    std::array<bool, 8> _val{ gRenderSSLR, gRenderSSAO, gRenderEyeAdaptation,
        //  3                    4             5             6               7
            gRenderDepthOfField, gRenderBloom, gRenderBokeh, gRenderDiffuse, gRenderLight };

    for( size_t i = 0; i < _val.size(); i++ ) if( _val[i] ) _RenderFlags |= 1 << i;

    // 
    const float gFarScale = 100.f;

    // 
    CameraComponent* cam = mScene->GetCamera(0)->cCam;
    CameraComponent* l_cam = mScene->GetCamera(1)->cCam;
    float fNear = cam->fNear;
    float fFar = cam->fFar;
    float fQ = fFar / (fNear - fFar);

    float l_fNear = l_cam->fNear;
    float l_fFar = l_cam->fFar;
    float l_fQ = l_fFar / (l_fNear - l_fFar);

    // Update constant buffers
    FinalPassInst *__q = gHDRPostProcess->MapFinalPass();
    __q->_LumWhiteSqr = White * White * MidGray * MidGray;
    __q->_MiddleGrey = MidGray;
    __q->_BloomScale = gBloomScale;
    __q->_ProjectedValues = { fNear * fQ, fQ };
    __q->_DoFFarValues = { gFarStart * gFarScale, 1.f / (gFarRange * gFarScale) };
    __q->_BokehThreshold = gBokehThreshold;
    __q->_ColorScale = gBokehColorScale;
    __q->_RadiusScale = gBokehRadiusScale;
    __q->_RenderFlags = _RenderFlags;
    gHDRPostProcess->UnmapFinalPass();

    const WindowConfig& cfg = gWindow->GetCFG();
    DownScaleInst* __c = gHDRPostProcess->MapDownScale();
        __c->_Res = { static_cast<uint32_t>(cfg.CurrentWidth / 4), static_cast<uint32_t>(cfg.CurrentHeight / 4) };
        __c->_Domain = __c->_Res.x * __c->_Res.y;
        __c->_GroupSize = __c->_Domain / 1024;
        __c->_Adaptation = gAdaptation / (float)gDirectX->GetConfig().RefreshRate;
        __c->_BloomThreshold = gBloomThres;
    gHDRPostProcess->UnmapDownScale();

    // Update settings
    gSSAOArgs._CameraFar = fFar;
    gSSAOArgs._CameraNear = fNear;
    gSSAOArgs._mView = cam->mView;
    gSSAOArgs._mProj = cam->mProj;
    gSSAOArgs._OffsetRad = gSSAOOffsetRad;
    gSSAOArgs._Radius = gSSAORadius;
    gSSAOArgs._Power = gSSAOPower;
    gSSAOArgs._Blur = gSSAOBlur;

    gSSLRArgs._mProj = cam->mProj;
    gSSLRArgs._CameraFar = fFar;
    gSSLRArgs._CameraNear = fNear;
    gSSLRArgs._ViewAngleThreshold = gViewAngleThreshold;
    gSSLRArgs._EdgeDistThreshold = gEdgeDistThreshold;
    gSSLRArgs._ReflScale = gReflScale;
    gSSLRArgs._DepthBias = gDepthBias;

    /*gCBuffArgs.Scaling = gCBuffScale;
    gCBuffArgs._CameraFar = fFar;
    gCBuffArgs._CameraNear = fNear;*/

    //     World View Proj
    //           View Proj
    // Inv       View Proj

    mfloat4x4 mInvViewProj = cam->mView * cam->mProj;
    mInvViewProj = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(mInvViewProj), mInvViewProj);

    gOITSettings.mInvViewProj = mInvViewProj;
    gOITSettings.fMaxFadeDist = gMaxFadeDist;
    gOITSettings.fMinFadeDist = gMinFadeDist;

    float4x4 mProjF;
    DirectX::XMStoreFloat4x4(&mProjF, cam->mProj);

    float4x4 mProj2F;
    DirectX::XMStoreFloat4x4(&mProj2F, l_cam->mProj);

    gVolumetricSettings._GScattering = gVLGScattering;
    gVolumetricSettings._Scaling     = { gVLScaling, gVLScaling };
    gVolumetricSettings._ProjValues  = { fNear * fQ, fQ, 1.f / mProjF.m[0][0], 1.f / mProjF.m[1][1] };
    gVolumetricSettings._ProjValues2 = { l_fNear * l_fQ, l_fQ, 1.f / mProj2F.m[0][0], 1.f / mProj2F.m[1][1] };

    // 
    ImGui::End();
    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void RendererDeferred::ClearMainRT() {
    gDirectX->gContext->ClearRenderTargetView(gDirectX->gRTV, s_clear.black_void2);
    gDirectX->gContext->ClearDepthStencilView(gDirectX->gDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.f, 0);
}

void RendererDeferred::DebugHUD() {
    if( !bDebugHUD ) return;
    ScopedRangeProfiler s0(L"Debug HUD");

    std::vector<ID3D11ShaderResourceView*> surfaces = {
        //rtTransparency->GetBufferSRV<0>(),
        //rtGBuffer->GetBufferSRV<1>(),
        //rtGBuffer->GetBufferSRV<0>(),
        //rtGBuffer->GetBufferSRV<2>(),
        rtCombinedGBuffer->GetBufferSRV<0>(), 
        mVolumetricLightAccum->GetSRV()
    };

    TransformComponent transf{};

    float w = (surfaces.size() == 1) ? Width()  : (Width() / surfaces.size() * .5f);
    float h = (surfaces.size() == 1) ? Height() : (w * .5f);

    // Bind "persistant" states
    shSimpleGUI->Bind();
    BindOrtho();
    s_states.blend.normal->Bind();
    s_material.sampl.point->Bind(Shader::Pixel);

    // Draw black rectangle to black everything before
    transf.vScale = { Width() * .5f, h, 1.f };
    transf.vPosition = { Width() * .5f, h, 0.f };
    s_material.ti.tex.black->Bind(Shader::Pixel);

    transf.Build();
    transf.Bind(cbTransform, Shader::Vertex, 0);

    // Draw call
    DXDraw(6, 0);

    // Draw HUD
    transf.vScale = { w, -h, 1.f };

    for( uint i = 0; i < surfaces.size(); i++ ) {
        // Bind resource
        Shader::Bind(surfaces[i], Shader::Pixel, 0);

        // Transform
        transf.vPosition = { w * (2 * i + 1), h, 0.f };
        transf.Build();
        transf.Bind(cbTransform, Shader::Vertex, 0);

        // Draw call
        DXDraw(6, 0);
    }

    // Discard SRV
    LunaEngine::PSDiscardSRV<1>();
}

void RendererDeferred::Shadows() {
    ScopedRangeProfiler s1(L"Shadows");

    {
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
            mScene->BindCamera(1, Shader::Domain | Shader::Vertex, 1); // Light camera
            mScene->Render(RendererFlags::ShadowPass | RendererFlags::OpaquePass, Shader::Vertex);
        }

        // TODO: CSM
        {
            //gCascadeShadowMapping->Begin(gCSMArgs);
            //mScene->Render(RendererFlags::ShadowPass | RendererFlags::OpaquePass, Shader::Vertex);
        }

        mScene->SetActiveCamera(old);
    }
}

void RendererDeferred::GBuffer() {
    ScopedRangeProfiler s1(L"GBuffer");

    // Bind render target
    rtGBuffer->Bind();
    rtGBuffer->Clear(0.f, 0, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
    //rtGBuffer->Clear(s_clear.black_void2);

    // Bind shader
    shSurface->Bind();

    // Bind default material textures
    s_material.sampl.point->Bind(Shader::Pixel, 0);
    s_material.sampl.linear->Bind(Shader::Pixel, 1);

    for( uint32_t i = 2; i < 8; i++ ) {
        s_material.sampl.point->Bind(Shader::Pixel, i);
    }

    s_material.ti.tex.checkboard->Bind(Shader::Pixel, 0);  // Albedo
    s_material.ti.tex.tile_normal->Bind(Shader::Pixel, 1); // Normal
    s_material.ti.tex.black->Bind(Shader::Pixel, 2);       // Metallic
    s_material.ti.tex.black->Bind(Shader::Pixel, 3);       // Roughness
    s_material.ti.tex.black->Bind(Shader::Pixel, 4);       // Emission
    s_material.ti.tex.white->Bind(Shader::Pixel, 5);       // Ambient Occlusion
    s_material.ti.tex.white->Bind(Shader::Pixel, 6);       // Heightmap
    s_material.ti.tex.white->Bind(Shader::Pixel, 7);       // Opacity

    // Bind ambient light
    mScene->BindAmbientLight(Shader::Pixel, 1);

    // Bind depth buffer
    rtDepth->Bind(0u, Shader::Pixel, 8);
    s_material.sampl.linear_comp->Bind(Shader::Pixel, 8);

    // Bind noise texture
    s_material.ti.tex.bluenoise_rg_512->Bind(Shader::Pixel, 9);
    s_material.sampl.point->Bind(Shader::Pixel, 9);

    // Bind cubemap
    s_material.mCubemap->Bind(Shader::Pixel, 10);
    s_material.sampl.linear->Bind(Shader::Pixel, 10);

    // Bind CBs
    mScene->BindCamera(0, Shader::Domain | Shader::Vertex, 1); // Main camera
    mScene->BindCamera(1, Shader::Domain | Shader::Vertex, 2); // Shadow camera

    // Bind material layers
    uint32_t old_ml = mScene->GetEnabledMaterialLayers();
    mScene->SetLayersState(Scene::Default);

    uint32_t old = mScene->GetActiveCamera();
    mScene->SetActiveCamera(0);

    UINT dc = gDrawCallCount + gDrawCallInstanceCount + gDispatchCallCount;

    {
        mScene->Render(RendererFlags::OpaquePass, Shader::Vertex);
    }

    mOpaqueAmount = (gDrawCallCount + gDrawCallInstanceCount + gDispatchCallCount) - dc;

    // Restore states
    mScene->SetActiveCamera(old);
    mScene->SetLayersState(old_ml);

    // Unbind textures
    //LunaEngine::PSDiscardSRV<8>();
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
    mTransparencyAmount = (gDrawCallCount + gDrawCallInstanceCount + gDispatchCallCount) - dc;

    // Done
    if( mTransparencyAmount ) {
        IdentityTransf->Bind(cbTransform, Shader::Vertex, 0);
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
    ScopedRangeProfiler s1(L"Deferred pass");

    // Light pass
    {
        ScopedRangeProfiler s2(L"Lights");
        rtDeferred->Bind();

        // Store states
        STopologyState::Push();
        STopologyState::Bind(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

        // Get camera info
        CameraComponent* cam = mScene->GetCamera(0)->cCam;
        float fNear = cam->fNear;
        float fFar  = cam->fFar;
        float fQ    = fFar / (fNear - fFar);
        float fHalfTanFov = tanf(DirectX::XMConvertToRadians(cam->fFOV_X * .5f)); // dtan(fov * .5)

        mfloat4x4 mProj = cam->mProj;
        mfloat4x4 mView = cam->mView;
        mfloat4x4 mInvProj = cam->mInvProj;
        mfloat4x4 mInvView = cam->mInvView;

        float4x4 mProjF;
        DirectX::XMStoreFloat4x4(&mProjF, mProj);

        // Bind data
        {
            {
                ScopeMapConstantBuffer<DeferredGlobal> map(cbDeferredGlobal);
                map.data->_mInvView = mInvView;
                map.data->_mInvProj = mInvProj;
                map.data->_TanAspect = { fHalfTanFov * cam->fAspect, -fHalfTanFov };
                map.data->_Texel = { 1.f / Width(), 1.f / Height() };
                map.data->_Far = fFar;
                map.data->_ProjValues = { fNear * fQ, fQ, 1.f / mProjF.m[0][0], 1.f / mProjF.m[1][1] };
            }

            cbDeferredGlobal->Bind(Shader::Pixel, 0); // CB

            s_material.sampl.point->Bind(Shader::Pixel, 0); // Samplers
            s_material.sampl.point->Bind(Shader::Pixel, 1); // Samplers
        }

        if( mOpaqueAmount )
        {
            ScopedRangeProfiler s3(L"Opaque");

            rtGBuffer->Bind(0u, Shader::Pixel, 0); // Opaque Depth Buffer
            rtGBuffer->Bind(2u, Shader::Pixel, 1); // Opaque Normal Buffer

            //rtGBuffer->Bind(1u, Shader::Pixel, 2); // Direct light
            //rtGBuffer->Bind(3u, Shader::Pixel, 3); // Ambient light

            DeferredLights();
        }

        if( mTransparencyAmount )
        {
            ScopedRangeProfiler s3(L"Transparent");

            rtTransparency->Bind(0u, Shader::Pixel, 0); // Opaque Depth Buffer
            rtTransparency->Bind(2u, Shader::Pixel, 1); // Opaque Normal Buffer

            //rtTransparency->Bind(1u, Shader::Pixel, 2); // Direct light
            //rtTransparency->Bind(3u, Shader::Pixel, 3); // Ambient light

            DeferredLights();
        }

        LunaEngine::PSDiscardSRV<2>(); // TODO: Use defines
        STopologyState::Pop();
    }
    

}

void RendererDeferred::DeferredLights() {
    // Point lights
    {
        ScopedRangeProfiler s2(L"Point");
        //shDeferredPointLights->Bind();

        {
            //ScopeMapConstantBuffer<DeferredLight> map(cbDeferredLight);


        }

        //DXDraw(2, 0);

        // TODO: 
        //DXDrawInstanced(2, CulledLightCount, 0, 0);
    }

    // Spot lights
    {

    }

    // Line lights (GPU Pro 3)
    {

    }

    // TODO: Maybe interchangable with line lights
    // Capsule lights
    {

    }

    // Area lights
    {

    }
}

void RendererDeferred::SSAO() {
    //CameraComponent *cam = mScene->GetCamera(0)->cCam;

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
    rtTransparency->Bind(1, Shader::Pixel, 3);

    // Draw call
    DXDraw(6, 0);

    // 
    BlendState::Pop();
    LunaEngine::PSDiscardSRV<3>();

    // Copy result
    //gDirectX->gContext->CopyResource(rtGBuffer->GetBufferTexture<0>(), rtCombinedGBuffer->GetBufferTexture<0>());

}

void RendererDeferred::HDR() {
    gHDRPostProcess->Begin(rtGBuffer);
}

void RendererDeferred::VolumetricLight() {
    ScopedRangeProfiler q(L"Volumetric Light");

    // Update constant buffer
    {
        ScopeMapConstantBufferCopy<VolumetricSettings> q(cbVolumetricSettings, &gVolumetricSettings._ProjValues.x);
    }

    TransformComponent* l_transf = mScene->GetCamera(1)->cTransf;

    // Bind resources
    mVolumetricLightAccum->Bind(Shader::Compute, 0u, true); // UAV
    l_transf->Bind(cbTransform, Shader::Compute, 0u);       // CB
    mScene->BindCamera(1,       Shader::Compute, 1u);       // CB
    mScene->BindCamera(0,       Shader::Compute, 2u);       // CB
    mScene->BindWorldLight(     Shader::Compute, 3u);       // CB
    cbVolumetricSettings->Bind( Shader::Compute, 4u);       // CB
    rtDepth->Bind(0u,           Shader::Compute, 0u);       // SRV
    rtGBuffer->Bind(0u,         Shader::Compute, 1u);       // SRV

    // Dispatch
    uint X = mVolumetricLightAccum->GetWidth();
    uint Y = mVolumetricLightAccum->GetHeight();
    
    shVolumetricLight->Dispatch(X, Y, 1u);

    // Discard
    LunaEngine::CSDiscardUAV<1>();
    LunaEngine::CSDiscardSRV<2>();
    LunaEngine::CSDiscardCB<5>();
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
    mScene->GetCamera(2)->ViewIdentity();
    mScene->BindCamera(2, Shader::Vertex, 1);

    IdentityTransf->Bind(cbTransform, Shader::Vertex, 0);
}
