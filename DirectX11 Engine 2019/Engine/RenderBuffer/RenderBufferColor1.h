#pragma once

#include "RenderBufferBase.h"

class RenderBufferColor1: public RenderBufferBase {
private:
    sRenderBuffer *sColor0;
    sRenderBuffer *sMSAAColor[1];

public:
    void CreateColor0(DXGI_FORMAT format);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Bind(sRenderBuffer* DSV);
    void Release();
    void Resize(int w, int h);
    void Clear(const FLOAT Color0[4]);

    void SetName(const char* name) { _SetName(sColor0->pTexture2D, name); }

    sRenderBuffer* GetColor0();
    sRenderBuffer* GetDepthB() override { return nullptr; }
    sRenderBuffer* GetDepthBF() override { return nullptr; }
    sRenderBuffer* GetDepthBF1() override { return nullptr; }
    sRenderBuffer* GetDepthBF2() override { return nullptr; }

    void MSAAResolve() override {
        if( mMSAA ) {
            // Unbind views
            ID3D11RenderTargetView* nullRTV = nullptr;
            gDirectX->gContext->OMSetRenderTargets(1, &nullRTV, nullptr);

            // Resolve color buffer
            gDirectX->gContext->ResolveSubresource(sMSAAColor[0]->pTexture2D, 0, sColor0->pTexture2D, 0, sColor0->format);
        }
    }
};
