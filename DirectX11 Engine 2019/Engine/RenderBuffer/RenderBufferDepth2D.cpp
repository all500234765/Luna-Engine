#include "RenderBufferDepth2D.h"

void RenderBufferDepth2D::Create(int w, int h, UINT bpp) {
    sDepth = CreateDSV2D(w, h, bpp);
}

void RenderBufferDepth2D::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(sDepth, type, slot);
}

void RenderBufferDepth2D::Bind() {
    BindTarget(sDepth);
}

void RenderBufferDepth2D::Resize(int w, int h) {
    UINT bpp = sDepth->bpp;
    sDepth->Release();
    sDepth = CreateDSV2D(w, h, bpp);
}

void RenderBufferDepth2D::Clear(UINT flags, FLOAT depth, UINT8 stencil) {
    gDirectX->gContext->ClearDepthStencilView(sDepth->pDSV, flags, depth, stencil);
}

ID3D11DepthStencilView* RenderBufferDepth2D::GetTarget() {
    return sDepth->pDSV;
}

sRenderBuffer* RenderBufferDepth2D::GetDepth() {
    return sDepth;
}
