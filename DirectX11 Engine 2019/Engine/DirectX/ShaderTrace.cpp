#include "pc.h"
#include "ShaderTrace.h"

void ShaderTrace::SetFactory(ID3D11ShaderTraceFactory* pFactory) {
    gFactory = pFactory;
}

void ShaderTrace::CreateTrace(const Shader& shader, ShaderType type) {

}

void ShaderTrace::Release() {
    pTrace->Release();
}

ID3D11ShaderTraceFactory *ShaderTrace::gFactory = 0;
