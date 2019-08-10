#pragma once

#include "RenderBufferBase.h"

class RenderBufferColor3Depth: public RenderBufferBase {
private:
    sRenderBuffer *sColor0, *sColor1, *sColor2, *sDepth;

public:
    void SetSize(int w, int h);
    void CreateColor0(DXGI_FORMAT format);
    void CreateColor1(DXGI_FORMAT format);
    void CreateColor2(DXGI_FORMAT format);
    void CreateDepth(UINT bpp);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Release();
    void Clear(const FLOAT Color0[4], UINT flags=D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth=1, UINT8 stencil=0);
    void Clear(const FLOAT Color0[4], const FLOAT Color1[4], const FLOAT Color2[4], UINT flags=D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth=1, UINT8 stencil=0);

    void Resize(int w, int h);
    
    // i=-1 Depth; i [0; 2] Color
    void SetName(int i, const char* name) {
        switch( i ) {
            case -1: _SetName(sDepth->pTexture2D, name); break;
            case 0: _SetName(sColor0->pTexture2D, name); break;
            case 1: _SetName(sColor1->pTexture2D, name); break;
            case 2: _SetName(sColor2->pTexture2D, name); break;
        }
    }

    void SetName(const char* name) {
        _SetName(sDepth->pTexture2D, name);
        _SetName(sColor0->pTexture2D, name);
        _SetName(sColor1->pTexture2D, name);
        _SetName(sColor2->pTexture2D, name);
    }

    sRenderBuffer* GetColor0();
    sRenderBuffer* GetColor1();
    sRenderBuffer* GetColor2();
    sRenderBuffer* GetDepth();
};
