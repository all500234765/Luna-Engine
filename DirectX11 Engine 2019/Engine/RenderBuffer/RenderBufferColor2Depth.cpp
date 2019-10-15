#include "RenderBufferColor2Depth.h"

void RenderBufferColor2Depth::SetSize(int w, int h) {
    Width = w;
    Height = h;
}

void RenderBufferColor2Depth::CreateColor0(DXGI_FORMAT format) {
    sColor0 = CreateRTV2D(Width, Height, format);
    CreateNonMSAA(format, sMSAAColor, 2);
}

void RenderBufferColor2Depth::CreateColor1(DXGI_FORMAT format) {
    sColor1 = CreateRTV2D(Width, Height, format);
    CreateNonMSAA(format, sMSAAColor, 3);
}

void RenderBufferColor2Depth::CreateDepth(UINT bpp) {
    sDepth = CreateDSV2D(Width, Height, bpp);
    CreateNonMSAA(bpp, sMSAAColor);
}

void RenderBufferColor2Depth::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(GetDepthB(), type, slot + 0U);
    BindResource(GetColor0(), type, slot + 1U);
    BindResource(GetColor1(), type, slot + 2U);
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

    delete sDepth;
    delete sColor0;
    delete sColor1;

    if( mMSAA ) {
        for( uint32_t i = 0; i < 2 + 2; i++ ) {
            sMSAAColor[i]->Release();
            delete sMSAAColor[i];
        }
    }
}

void RenderBufferColor2Depth::Resize(int w, int h) {
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

    // Color 2
    f1 = sColor1->format;
    mMSAA = ((sColor1->Flags & IsMSAA) == IsMSAA);
    sColor1->Release();
    sColor1 = CreateRTV2D(w, h, f1);

    if( mMSAA ) {
        sMSAAColor[3]->Release(); // MSAA
        CreateNonMSAA(f1, sMSAAColor, 3);
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
    return mMSAA ? sMSAAColor[2] : sColor0;
}

sRenderBuffer* RenderBufferColor2Depth::GetColor1() {
    return mMSAA ? sMSAAColor[3] : sColor1;
}

sRenderBuffer* RenderBufferColor2Depth::GetDepthB() {
    return mMSAA ? sMSAAColor[0] : sDepth;
}
