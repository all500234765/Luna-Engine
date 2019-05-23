#include "RenderBufferDepth2D.h"

void RenderBufferDepth2D::Create(int w, int h, UINT bpp) {
    Width = w;
    Height = h;
    sDepth = CreateDSV2D(w, h, bpp);
}

void RenderBufferDepth2D::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(sDepth, type, slot);
}

void RenderBufferDepth2D::Bind() {
    BindTarget(sDepth);
}

void RenderBufferDepth2D::Release() {
    sDepth->Release();
}

ID3D11DepthStencilView* RenderBufferDepth2D::GetTarget() {
    return sDepth->pDSV;
}

sRenderBuffer* RenderBufferDepth2D::GetDepth() {
    return sDepth;
}
