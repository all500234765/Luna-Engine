#pragma once

#include "RenderBufferBase.h"

class RenderBufferColor1Depth: public RenderBufferBase {
private:
    sRenderBuffer *sColor0, *sDepth;
    sRenderBuffer *sMSAAColor[1 + 2]; // Non-MSAA Targets

public:
    void SetSize(int w, int h);
    void CreateColor0(DXGI_FORMAT format);
    void CreateDepth(UINT bpp);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Release();
    void Resize(int w, int h);
    void Clear(const FLOAT Color0[4], UINT flags=D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth=1, UINT8 stencil=0);

    sRenderBuffer* GetColor0();
    sRenderBuffer* GetDepth();
    sRenderBuffer* GetDepthB()   override { return mMSAA ? sMSAAColor[0] : sDepth; };
    sRenderBuffer* GetDepthBF()  override { return sDepth; }
    sRenderBuffer* GetDepthBF1() override { return sMSAAColor[0]; }
    sRenderBuffer* GetDepthBF2() override { return sMSAAColor[1]; }

    void MSAAResolve() override {
        if( mMSAA ) {
            // Unbind views
            ID3D11RenderTargetView* nullRTV = nullptr;
            gDirectX->gContext->OMSetRenderTargets(1, &nullRTV, nullptr);

            // Resolve color buffers
            if( (sColor0->Flags & IsMSAA) == IsMSAA ) 
                gDirectX->gContext->ResolveSubresource(sMSAAColor[2]->pTexture2D, 0, sColor0->pTexture2D, 0, sColor0->format);

            // Depth
            MSAAResolveDepth(this);
        }
    }
};
