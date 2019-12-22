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

    

    // Release blobs
    shSurface->ReleaseBlobs();
    shVertexOnly->ReleaseBlobs();
#pragma endregion

#pragma region Render Targets
    rtGBuffer = new RenderTarget2DColor4DepthMSAA(Width(), Height(), 1, "GBuffer");
    rtGBuffer->Create(32);                                     // Depth
    rtGBuffer->CreateList(0, DXGI_FORMAT_R16G16B16A16_FLOAT,   // Direct
                             DXGI_FORMAT_R16G16B16A16_FLOAT,   // Normals
                             DXGI_FORMAT_R16G16B16A16_FLOAT,   // Ambient
                             DXGI_FORMAT_R16G16B16A16_FLOAT);  // Emission

    rtDepth = new RenderTarget2DDepthMSAA(Width(), Height(), 1, "World light shadowmap");
    rtDepth->Create(32);

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
        s_material.sampl.point_comp = new Sampler();
        s_material.sampl.point_comp->Create(pDesc);

        // Linear sampler
        pDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        pDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        s_material.sampl.linear = new Sampler();
        s_material.sampl.linear->Create(pDesc);

        // Compare linear sampler
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


}

void RendererDeferred::Render() {
    ScopedRangeProfiler q0(L"Deferred Renderer");
    mScene = Scene::Current();
    if( !mScene ) {
        printf_s("[%s]: No scene is bound!\n", __FUNCTION__);
        return;
    }

    {
        ScopedRangeProfiler q(L"Geometry rendering");

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
        Combine();
    }

    {
        ScopedRangeProfiler q(L"Final post processing");

        // Final post-processing
        DOF();
        HDR();
        Final();
    }
}

void RendererDeferred::Resize(float W, float H) {

}

void RendererDeferred::Release() {
    // Shaders
    SAFE_RELEASE(shSurface);
    SAFE_RELEASE(shVertexOnly);

    // Render Targets
    SAFE_RELEASE(rtTransparency);
    SAFE_RELEASE(rtGBuffer);
    SAFE_RELEASE(rtDepth);

    // Textures
    SAFE_RELEASE(s_material.tex.bluenoise_rg_512);
    SAFE_RELEASE(s_material.tex.checkboard);
    SAFE_RELEASE(s_material.mCubemap);

    // Samplers
    SAFE_RELEASE(s_material.sampl.point);
    SAFE_RELEASE(s_material.sampl.point_comp);
    SAFE_RELEASE(s_material.sampl.linear);
    SAFE_RELEASE(s_material.sampl.linear_comp);

    // Effects
    SAFE_DELETE(gHDRPostProcess            );
    SAFE_DELETE(gSSAOPostProcess           );
    SAFE_DELETE(gSSLRPostProcess           );
    SAFE_DELETE(gCascadeShadowMapping      );
    //SAFE_DELETE(gCoverageBuffer            );
    SAFE_DELETE(gOrderIndendentTransparency);
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
    mScene->BindCamera(0, Shader::Vertex, 1); // Main camera
    mScene->Render(RendererFlags::OpacityPass, Shader::Vertex);

    // Done
    gOrderIndendentTransparency->End(rtTransparency, gOITSettings);
}

void RendererDeferred::CoverageBuffer() {

}

void RendererDeferred::Deferred() {

}

void RendererDeferred::SSAO() {
    // Opaque
    gSSAOPostProcess->Begin(rtGBuffer, gSSAOArgs);

    // TODO: Add separate RT for OIT
    // Transparent
    gSSAOPostProcess->Begin(rtTransparency, gSSAOArgs);
}

void RendererDeferred::SSLR() {

}

void RendererDeferred::SSLF() {

}

void RendererDeferred::FSSSSS() {

}

void RendererDeferred::Combine() {

}

void RendererDeferred::DOF() {

}

void RendererDeferred::HDR() {

}

void RendererDeferred::Final() {

}
