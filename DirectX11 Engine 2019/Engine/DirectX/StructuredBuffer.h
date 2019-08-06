#pragma once

#include "Buffer.h"
#include "Shader.h"

template<typename T>
class StructuredBuffer: public Buffer {
    ID3D11ShaderResourceView *pSRV;

public:
    void CreateDefault(UINT num, void* data);

    void Bind(Shader::ShaderType type, UINT slot);
};
