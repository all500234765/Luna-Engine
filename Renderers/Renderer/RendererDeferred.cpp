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

    rtTransparency = new RenderTarget2DColor2DepthMSAA(Width(), Height(), 1, "Transparency");
    rtTransparency->Create(32);                                     // Depth
    rtTransparency->CreateList(0, DXGI_FORMAT_R8G8B8A8_UNORM,       // Color
                                  DXGI_FORMAT_R16G16B16A16_FLOAT);  // Normals


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

        SSAO();
        SSLR();
        SSLF();
        FSSSSS();

        Combine();

        DOF();
    }

    {
        ScopedRangeProfiler q(L"Final post processing");

        HDR();
        Final();
    }
}

void RendererDeferred::Release() {
    // Shaders
    SAFE_RELEASE(shSurface);
    SAFE_RELEASE(shVertexOnly);
    SAFE_RELEASE(rtTransparency);

}

void RendererDeferred::ImGui() {

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
    mCubemap->Bind(Shader::Pixel, 8);
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
