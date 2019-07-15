#pragma once

#include "RenderBufferBase.h"

class RenderBufferColor2Depth: public RenderBufferBase {
private:
    sRenderBuffer *sColor0, *sColor1, *sDepth;

public:
    void SetSize(int w, int h);
    void CreateColor0(DXGI_FORMAT format);
    void CreateColor1(DXGI_FORMAT format);
    void CreateDepth(UINT bpp);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Release();
    void Resize(int w, int h);
    void Clear(const FLOAT Color0[4], UINT flags=D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth=1, UINT8 stencil=0);
    void Clear(const FLOAT Color0[4], const FLOAT Color1[4], UINT flags=D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth=1, UINT8 stencil=0);

    sRenderBuffer* GetColor0();
    sRenderBuffer* GetColor1();
    sRenderBuffer* GetDepth();
};
