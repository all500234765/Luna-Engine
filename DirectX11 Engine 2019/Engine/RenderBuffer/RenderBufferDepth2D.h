#pragma once

#include "RenderBufferBase.h"

class RenderBufferDepth2D: public RenderBufferBase {
private:
    sRenderBuffer *sDepth;
    sRenderBuffer *sMSAAColor[0 + 2]; // Non-MSAA Target

public:
    void Create(int w, int h, UINT bpp);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Release() {
        sDepth->Release();
        delete sDepth;

        if( mMSAA ) {
            sMSAAColor[0]->Release();
            sMSAAColor[1]->Release();
            delete sMSAAColor[0];
            delete sMSAAColor[1];
        }
    }

    void Resize(int w, int h);
    void Clear(UINT flags=D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth=1, UINT8 stencil=0);

    void SetName(const char* name) { _SetName(sDepth->pTexture2D, name); }

    ID3D11DepthStencilView *GetTarget();

    sRenderBuffer* GetColor0() override { return nullptr; }
    sRenderBuffer* GetDepthB();
    sRenderBuffer* GetDepthBF() override { return sDepth; }
    sRenderBuffer* GetDepthBF1() override { return sMSAAColor[0]; }
    sRenderBuffer* GetDepthBF2() override { return sMSAAColor[1]; }
    //sRenderBuffer* GetDepthBFA() override { return ; }

    void MSAAResolve() override {
        if( mMSAA ) {
            // Unbind views
            ID3D11RenderTargetView* nullRTV = nullptr;
            gDirectX->gContext->OMSetRenderTargets(1, &nullRTV, nullptr);

            // Depth
            MSAAResolveDepth(this);
        }
    }
};
