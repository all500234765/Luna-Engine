#pragma once

#include "RenderBufferBase.h"

class RenderBufferDepth2D: public RenderBufferBase {
private:
    sRenderBuffer *sDepth;

public:
    void Create(int w, int h, UINT bpp);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Release() { sDepth->Release(); }
    void Resize(int w, int h);
    void Clear(UINT flags=D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, FLOAT depth=1, UINT8 stencil=0);

    void SetName(const char* name) { _SetName(sDepth->pTexture2D, name); }

    ID3D11DepthStencilView *GetTarget();

    sRenderBuffer* GetDepth();
};
