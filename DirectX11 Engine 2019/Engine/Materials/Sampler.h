#pragma once

#include "pc.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/DirectXChild.h"

class Sampler: public DirectXChild {
private:
    ID3D11SamplerState *pSampler;

public:
    void Create(D3D11_SAMPLER_DESC pDesc);
    void Bind(Shader::ShaderType type, UINT slot=0);
    bool IsCreated();
    void Release();
};
