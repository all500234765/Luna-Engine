#include "RenderBufferColor1Depth.h"

void RenderBufferColor1Depth::SetSize(int w, int h) {
    Width = w;
    Height = h;
}

void RenderBufferColor1Depth::CreateColor0(DXGI_FORMAT format) {
    sColor0 = CreateRTV2D(Width, Height, format);
    CreateNonMSAA(format, sMSAAColor, 2);
}

void RenderBufferColor1Depth::CreateDepth(UINT bpp) {
    sDepth = CreateDSV2D(Width, Height, bpp);
    CreateNonMSAA(bpp, sMSAAColor);
}

void RenderBufferColor1Depth::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(GetDepthB(), type, slot + 0U);
    BindResource(GetColor0(), type, slot + 1U);
}

void RenderBufferColor1Depth::Bind() {
    BindTarget(sColor0, sDepth);
}

void RenderBufferColor1Depth::Release() {
    sColor0->Release();
    sDepth->Release();

    delete sDepth;
    delete sColor0;

    if( mMSAA ) {
        for( uint32_t i = 0; i < 1 + 2; i++ ) {
            sMSAAColor[i]->Release();
            delete sMSAAColor[i];
        }
    }
}

void RenderBufferColor1Depth::Resize(int w, int h) {
    SetSize(w, h);

    bool oldMSAA = mMSAA;

    // Color 1
    DXGI_FORMAT f1 = sColor0->format;
    mMSAA = ((sColor0->Flags & IsMSAA) == IsMSAA);
    sColor0->Release();
    sColor0 = CreateRTV2D(w, h, f1);

    if( mMSAA ) {
        sMSAAColor[2]->Release(); // MSAA
        CreateNonMSAA(f1, sMSAAColor, 2);
    }

    // Depth
    UINT bpp = sDepth->bpp;
    mMSAA = ((sDepth->Flags & IsMSAA) == IsMSAA);
    sDepth->Release();
    sDepth = CreateDSV2D(w, h, bpp);

    if( mMSAA ) {
        sMSAAColor[0]->Release(); // MSAA
        sMSAAColor[1]->Release(); // MSAA
        CreateNonMSAA(bpp, sMSAAColor);
    }

    mMSAA = oldMSAA;
}

void RenderBufferColor1Depth::Clear(const FLOAT Color0[4], UINT flags, FLOAT depth, UINT8 stencil) {
    gDirectX->gContext->ClearRenderTargetView(sColor0->pRTV, Color0);
    gDirectX->gContext->ClearDepthStencilView(sDepth->pDSV, flags, depth, stencil);
}

sRenderBuffer* RenderBufferColor1Depth::GetColor0() {
    return mMSAA ? sMSAAColor[2] : sColor0;
}

sRenderBuffer* RenderBufferColor1Depth::GetDepth() {
    return mMSAA ? sMSAAColor[0] : sDepth;
}
