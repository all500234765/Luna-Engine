#pragma once

#include "RenderBufferBase.h"

class RenderBufferColor1: public RenderBufferBase {
private:
    sRenderBuffer *sColor0;

public:
    void CreateColor0(DXGI_FORMAT format);
    void BindResources(Shader::ShaderType type, UINT slot);
    void Bind();
    void Release();

    sRenderBuffer* GetColor0();
};
