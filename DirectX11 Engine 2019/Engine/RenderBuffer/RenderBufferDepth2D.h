#pragma once

#include "RenderBufferBase.h"

class RenderBufferDepth2D: public RenderBufferBase {
private:
    sRenderBuffer *sDepth;

public:
    void Create(int w, int h, UINT bpp);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Release();

    ID3D11DepthStencilView *GetTarget();
};
