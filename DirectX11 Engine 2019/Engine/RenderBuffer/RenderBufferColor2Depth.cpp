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

void RenderBufferColor2Depth::Resize(int w, int h) {
    SetSize(w, h);

    DXGI_FORMAT f1 = sColor0->format;
    sColor0->Release();
    sColor0 = CreateRTV2D(w, h, f1);

    f1 = sColor1->format;
    sColor1->Release();
    sColor1 = CreateRTV2D(w, h, f1);

    UINT bpp = sDepth->bpp;
    sDepth->Release();
    sDepth = CreateDSV2D(w, h, bpp);
}

void RenderBufferColor2Depth::Clear(const FLOAT Color0[4], UINT flags, FLOAT depth, UINT8 stencil) {
    gDirectX->gContext->ClearRenderTargetView(sColor0->pRTV, Color0);
    gDirectX->gContext->ClearRenderTargetView(sColor1->pRTV, Color0);
    gDirectX->gContext->ClearDepthStencilView(sDepth->pDSV, flags, depth, stencil);
}

void RenderBufferColor2Depth::Clear(const FLOAT Color0[4], const FLOAT Color1[4], UINT flags, FLOAT depth, UINT8 stencil) {
    gDirectX->gContext->ClearRenderTargetView(sColor0->pRTV, Color0);
    gDirectX->gContext->ClearRenderTargetView(sColor1->pRTV, Color1);
    gDirectX->gContext->ClearDepthStencilView(sDepth->pDSV, flags, depth, stencil);
}

sRenderBuffer* RenderBufferColor2Depth::GetColor0() {
    return sColor0;
}

sRenderBuffer* RenderBufferColor2Depth::GetColor1() {
    return sColor1;
}

sRenderBuffer* RenderBufferColor2Depth::GetDepthB() {
    return sDepth;
}
