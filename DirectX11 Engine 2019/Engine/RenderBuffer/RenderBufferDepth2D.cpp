#include "RenderBufferDepth2D.h"

void RenderBufferDepth2D::Create(int w, int h, UINT bpp) {
    sDepth = CreateDSV2D(w, h, bpp);
    CreateNonMSAA(bpp, sMSAAColor);
}

void RenderBufferDepth2D::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(GetDepthB(), type, slot);
}

void RenderBufferDepth2D::Bind() {
    BindTarget(sDepth);
}

void RenderBufferDepth2D::Resize(int w, int h) {
    // Depth
    UINT bpp = sDepth->bpp;
    sDepth->Release();
    sDepth = CreateDSV2D(w, h, bpp);

    if( mMSAA ) {
        sMSAAColor[0]->Release(); // MSAA
        CreateNonMSAA(bpp, sMSAAColor);
    }
}

void RenderBufferDepth2D::Clear(UINT flags, FLOAT depth, UINT8 stencil) {
    if( sDepth->pDSV ) gDirectX->gContext->ClearDepthStencilView(sDepth->pDSV, flags, depth, stencil);
}

ID3D11DepthStencilView* RenderBufferDepth2D::GetTarget() {
    return mMSAA ? sMSAAColor[0]->pDSV : sDepth->pDSV;
}

sRenderBuffer* RenderBufferDepth2D::GetDepthB() {
    return mMSAA ? sMSAAColor[0] : sDepth;
}
