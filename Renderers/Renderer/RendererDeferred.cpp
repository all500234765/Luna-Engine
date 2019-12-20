#include "RendererDeferred.h"

void RendererDeferred::Init() {
    // Load shaders
    shSurface = new Shader();
    shSurface->SetLayoutGenerator(LgMesh);
    shSurface->LoadFile("shSurfaceVS.cso", Shader::Vertex);
    shSurface->LoadFile("shSurfacePS.cso", Shader::Pixel);



    shSurface->ReleaseBlobs();

}

void RendererDeferred::Render(uint32_t CameraIndex) {
    ScopedRangeProfiler q0(L"Deferred Renderer");

    {
        ScopedRangeProfiler q(L"Geometry rendering");

        Shadows();
        GBuffer();
        OIT();
        GBufferMSAAResolve();
    }

    {
        ScopedRangeProfiler q(L"Screen-Space");

        SSAO();
        SSLR();
        SSLF();
        FSSSSS();

        Combine();

        DOF();
        HDR();
    }

    {
        Final();
    }
}

void RendererDeferred::Release() {
    shSurface->Release();


}

void RendererDeferred::ImGui() {

}

void RendererDeferred::Shadows() {

}

void RendererDeferred::GBuffer() {

}

void RendererDeferred::OIT() {

}

void RendererDeferred::GBufferMSAAResolve() {

}

void RendererDeferred::SSAO() {

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
