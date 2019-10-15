#include "RenderBufferColor3Depth.h"

void RenderBufferColor3Depth::SetSize(int w, int h) {
    Width = w;
    Height = h;
}

void RenderBufferColor3Depth::CreateColor0(DXGI_FORMAT format) {
    sColor0 = CreateRTV2D(Width, Height, format);
    CreateNonMSAA(format, sMSAAColor, 2);
}

void RenderBufferColor3Depth::CreateColor1(DXGI_FORMAT format) {
    sColor1 = CreateRTV2D(Width, Height, format);
    CreateNonMSAA(format, sMSAAColor, 3);
}

void RenderBufferColor3Depth::CreateColor2(DXGI_FORMAT format) {
    sColor2 = CreateRTV2D(Width, Height, format);
    CreateNonMSAA(format, sMSAAColor, 4);
}

void RenderBufferColor3Depth::CreateDepth(UINT bpp) {
    sDepth = CreateDSV2D(Width, Height, bpp);
    CreateNonMSAA(bpp, sMSAAColor);
}

void RenderBufferColor3Depth::BindResources(Shader::ShaderType type, UINT slot) {
    BindResource(GetDepthB(), type, slot + 0U);
    BindResource(GetColor0(), type, slot + 1U);
    BindResource(GetColor1(), type, slot + 2U);
    BindResource(GetColor2(), type, slot + 2U);
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

    delete sDepth;
    delete sColor0;
    delete sColor1;
    delete sColor2;

    if( mMSAA ) {
        for( uint32_t i = 0; i < 3 + 2; i++ ) {
            sMSAAColor[i]->Release();
            delete sMSAAColor[i];
        }
    }
}

void RenderBufferColor3Depth::Clear(const FLOAT Color0[4], UINT flags, FLOAT depth, UINT8 stencil) {
    gDirectX->gContext->ClearRenderTargetView(sColor0->pRTV, Color0);
    gDirectX->gContext->ClearRenderTargetView(sColor1->pRTV, Color0);
    gDirectX->gContext->ClearRenderTargetView(sColor2->pRTV, Color0);
    gDirectX->gContext->ClearDepthStencilView(sDepth->pDSV, flags, depth, stencil);
}

void RenderBufferColor3Depth::Clear(const FLOAT Color0[4], const FLOAT Color1[4], const FLOAT Color2[4], UINT flags, FLOAT depth, UINT8 stencil) {
    gDirectX->gContext->ClearRenderTargetView(sColor0->pRTV, Color0);
    gDirectX->gContext->ClearRenderTargetView(sColor1->pRTV, Color1);
    gDirectX->gContext->ClearRenderTargetView(sColor2->pRTV, Color2);
    gDirectX->gContext->ClearDepthStencilView(sDepth->pDSV, flags, depth, stencil);
}

void RenderBufferColor3Depth::Resize(int w, int h) {
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

    // Color 3
    f1 = sColor2->format;
    mMSAA = ((sColor2->Flags & IsMSAA) == IsMSAA);
    sColor2->Release();
    sColor2 = CreateRTV2D(w, h, f1);

    if( mMSAA ) {
        sMSAAColor[4]->Release(); // MSAA
        CreateNonMSAA(f1, sMSAAColor, 4);
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

sRenderBuffer* RenderBufferColor3Depth::GetColor0() {
    return mMSAA ? sMSAAColor[2] : sColor0;
}

sRenderBuffer* RenderBufferColor3Depth::GetColor1() {
    return mMSAA ? sMSAAColor[3] : sColor1;
}

sRenderBuffer* RenderBufferColor3Depth::GetColor2() {
    return mMSAA ? sMSAAColor[4] : sColor2;
}

sRenderBuffer* RenderBufferColor3Depth::GetDepthB() {
    return mMSAA ? sMSAAColor[0] : sDepth;
}
