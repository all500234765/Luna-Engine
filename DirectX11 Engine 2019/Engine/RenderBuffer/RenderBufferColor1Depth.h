#pragma once

#include "RenderBufferBase.h"

class RenderBufferColor1Depth: public RenderBufferBase {
private:
    sRenderBuffer *sColor0, *sDepth;

public:
    void SetSize(int w, int h);
    void CreateColor0(DXGI_FORMAT format);
    void CreateDepth(UINT bpp);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Release();
};
