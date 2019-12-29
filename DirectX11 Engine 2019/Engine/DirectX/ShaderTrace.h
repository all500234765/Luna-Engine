#pragma once

#include "pc.h"
#include "Shader.h"

class ShaderTrace: public Shader {
protected:
    static ID3D11ShaderTraceFactory *gFactory;

private:
    ID3D11ShaderTrace *pTrace;

public:
    static void SetFactory(ID3D11ShaderTraceFactory* pFactory);

    void CreateTrace(const Shader& shader, ShaderType type);

    void Release();
};
