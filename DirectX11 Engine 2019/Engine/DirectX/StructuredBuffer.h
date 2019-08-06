#pragma once

#include "Buffer.h"
#include "Shader.h"

class StructuredBuffer: public Buffer {
    ID3D11ShaderResourceView *pSRV;

public:
    void CreateDefault(UINT num, UINT size, void* data);

    void Bind(Shader::ShaderType type, UINT slot);
};
