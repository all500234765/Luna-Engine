#pragma once

#include "pc.h"
#include "TextureMaps.h"
#include "Sampler.h"
#include "Engine/DirectX/DirectXChild.h"
#include "Engine/DirectX/Shader.h"

class Material: public DirectXChild {
private:
    DiffuseMap *mDiffuse;
    NormalMap *mNormal;
    RougnessMap *mRoughness;
    Sampler *sSampler;

public:
    Material();

    void SetDiffuse(DiffuseMap* map);
    void SetNormal(NormalMap* map);
    void SetRoughness(RougnessMap* map);
    void SetSampler(Sampler* sampl);

    void BindTextures(Shader::ShaderType type, UINT slot=0);
    void Release();
};
