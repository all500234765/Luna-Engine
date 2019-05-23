#include "RenderBufferColor3Depth.h"

void RenderBufferColor3Depth::SetSize(int w, int h) {
    Width = w;
    Height = h;
}

void RenderBufferColor3Depth::CreateColor0(DXGI_FORMAT format) {
    sColor0 = CreateRTV2D(Width, Height, format);
}

void RenderBufferColor3Depth::CreateColor1(DXGI_FORMAT format) {
    sColor1 = CreateRTV2D(Width, Height, format);
}

void RenderBufferColor3Depth::CreateColor2(DXGI_FORMAT format) {
    sColor2 = CreateRTV2D(Width, Height, format);
}

void RenderBufferColor3Depth::CreateDepth(UINT bpp) {
    sDepth = CreateDSV2D(Width, Height, bpp);
}

void RenderBufferColor3Depth::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(sDepth , type, slot + 0U);
    BindResource(sColor0, type, slot + 1U);
    BindResource(sColor1, type, slot + 2U);
    BindResource(sColor2, type, slot + 2U);
}

void RenderBufferColor3Depth::Bind() {
    std::vector<sRenderBuffer*> pRTV;
    pRTV.push_back(sColor0);
    pRTV.push_back(sColor1);
    pRTV.push_back(sColor2);
    BindTarget(pRTV, sDepth);
}

void RenderBufferColor3Depth::Release() {
    sColor0->Release();
    sColor1->Release();
    sColor2->Release();
    sDepth->Release();
}

sRenderBuffer* RenderBufferColor3Depth::GetColor0() {
    return sColor0;
}

sRenderBuffer* RenderBufferColor3Depth::GetColor1() {
    return sColor1;
}

sRenderBuffer* RenderBufferColor3Depth::GetColor2() {
    return sColor2;
}

sRenderBuffer* RenderBufferColor3Depth::GetDepth() {
    return sDepth;
}
