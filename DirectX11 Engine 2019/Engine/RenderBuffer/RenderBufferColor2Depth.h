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

    sRenderBuffer* GetColor0();
    sRenderBuffer* GetColor1();
    sRenderBuffer* GetDepth();
};
