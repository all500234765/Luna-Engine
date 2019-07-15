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

void RenderBufferColor1Depth::Resize(int w, int h) {
    SetSize(w, h);

    DXGI_FORMAT f1 = sColor0->format;
    sColor0->Release();
    sColor0 = CreateRTV2D(w, h, f1);

    UINT bpp = sDepth->bpp;
    sDepth->Release();
    sDepth = CreateDSV2D(w, h, bpp);
}

void RenderBufferColor1Depth::Clear(const FLOAT Color0[4], UINT flags, FLOAT depth, UINT8 stencil) {
    gDirectX->gContext->ClearRenderTargetView(sColor0->pRTV, Color0);
    gDirectX->gContext->ClearDepthStencilView(sDepth->pDSV, flags, depth, stencil);
}

sRenderBuffer* RenderBufferColor1Depth::GetColor0() {
    return sColor0;
}

sRenderBuffer* RenderBufferColor1Depth::GetDepth() {
    return sDepth;
}
