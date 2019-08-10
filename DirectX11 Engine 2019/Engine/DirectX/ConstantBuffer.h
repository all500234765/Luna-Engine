#pragma once

#include "Buffer.h"
#include "Shader.h"

class ConstantBuffer: public Buffer {
public:
    void CreateDefault(UINT size);
    void* Map();
    void Unmap();
    void Bind(UINT type, UINT slot);
    void Bind(Shader::ShaderType type, UINT slot) { Bind(static_cast<UINT>(type), slot); }
};
