#include "RenderBufferColor1Depth.h"

void RenderBufferColor1Depth::SetSize(int w, int h) {
    Width = w;
    Height = h;
}

void RenderBufferColor1Depth::CreateColor0(DXGI_FORMAT format) {
    sColor0 = CreateRTV2D(Width, Height, format);
}

void RenderBufferColor1Depth::CreateDepth(UINT bpp) {
    sDepth = CreateDSV2D(Width, Height, bpp);
}

void RenderBufferColor1Depth::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(sDepth , type, slot + 0U);
    BindResource(sColor0, type, slot + 1U);
}

void RenderBufferColor1Depth::Bind() {
    BindTarget(sColor0, sDepth);
}

void RenderBufferColor1Depth::Release() {
    sColor0->Release();
    sDepth->Release();
}

sRenderBuffer* RenderBufferColor1Depth::GetColor0() {
    return sColor0;
}

sRenderBuffer* RenderBufferColor1Depth::GetDepth() {
    return sDepth;
}
