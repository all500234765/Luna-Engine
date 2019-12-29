#pragma once

#include "pc.h"
#include "Buffer.h"
#include "Shader.h"

class ConstantBuffer: public Buffer {
public:
    ConstantBuffer(UINT size) { CreateDefault(size); };
    ConstantBuffer() {};

    void CreateDefault(UINT size);
    void* Map(D3D11_MAP map=D3D11_MAP_WRITE_DISCARD);
    void Unmap();
    void Bind(UINT type, UINT slot);
    void Bind(Shader::ShaderType type, UINT slot) { Bind(static_cast<UINT>(type), slot); }
};
