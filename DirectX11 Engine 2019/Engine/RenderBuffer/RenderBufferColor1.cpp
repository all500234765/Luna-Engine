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

sRenderBuffer* RenderBufferColor1::GetColor0() {
    return sColor0;
}
