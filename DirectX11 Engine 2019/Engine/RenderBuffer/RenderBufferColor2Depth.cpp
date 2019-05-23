#include "RenderBufferColor2Depth.h"

void RenderBufferColor2Depth::SetSize(int w, int h) {
    Width = w;
    Height = h;
}

void RenderBufferColor2Depth::CreateColor0(DXGI_FORMAT format) {
    sColor0 = CreateRTV2D(Width, Height, format);
}

void RenderBufferColor2Depth::CreateColor1(DXGI_FORMAT format) {
    sColor1 = CreateRTV2D(Width, Height, format);
}

void RenderBufferColor2Depth::CreateDepth(UINT bpp) {
    sDepth = CreateDSV2D(Width, Height, bpp);
}

void RenderBufferColor2Depth::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(sDepth , type, slot + 0U);
    BindResource(sColor0, type, slot + 1U);
    BindResource(sColor1, type, slot + 2U);
}

void RenderBufferColor2Depth::Bind() {
    std::vector<sRenderBuffer*> pRTV;
    pRTV.push_back(sColor0);
    pRTV.push_back(sColor1);
    BindTarget(pRTV, sDepth);
}

void RenderBufferColor2Depth::Release() {
    sColor0->Release();
    sColor1->Release();
    sDepth->Release();
}

sRenderBuffer* RenderBufferColor2Depth::GetColor0() {
    return sColor0;
}

sRenderBuffer* RenderBufferColor2Depth::GetColor1() {
    return sColor1;
}

sRenderBuffer* RenderBufferColor2Depth::GetDepth() {
    return sDepth;
}
