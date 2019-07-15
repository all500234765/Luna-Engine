#include "RenderBufferColor1.h"

void RenderBufferColor1::CreateColor0(DXGI_FORMAT format) {
    sColor0 = CreateRTV2D(Width, Height, format);
}

void RenderBufferColor1::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(sColor0, type, slot + 0U);
}

void RenderBufferColor1::Bind() {
    BindTarget(sColor0);
}

void RenderBufferColor1::Release() {
    sColor0->Release();
}

void RenderBufferColor1::Resize(int w, int h) {
    SetSize(w, h);

    DXGI_FORMAT f1 = sColor0->format;
    sColor0->Release();
    sColor0 = CreateRTV2D(w, h, f1);
}

void RenderBufferColor1::Clear(const FLOAT Color0[4]) {
    gDirectX->gContext->ClearRenderTargetView(sColor0->pRTV, Color0);
}

sRenderBuffer* RenderBufferColor1::GetColor0() {
    return sColor0;
}
