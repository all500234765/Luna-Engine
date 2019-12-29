#include "pc.h"
#include "Sampler.h"

void Sampler::Create(D3D11_SAMPLER_DESC pDesc) {
    gDirectX->gDevice->CreateSamplerState(&pDesc, &pSampler);
}

void Sampler::Bind(Shader::ShaderType type, UINT slot) {
    if( !pSampler ) { return; }
    switch( type ) {
        case Shader::Vertex  : gDirectX->gContext->VSSetSamplers(slot, 1, &pSampler); break;
        case Shader::Pixel   : gDirectX->gContext->PSSetSamplers(slot, 1, &pSampler); break;
        case Shader::Geometry: gDirectX->gContext->GSSetSamplers(slot, 1, &pSampler); break;
        case Shader::Hull    : gDirectX->gContext->HSSetSamplers(slot, 1, &pSampler); break;
        case Shader::Domain  : gDirectX->gContext->DSSetSamplers(slot, 1, &pSampler); break;
        case Shader::Compute : gDirectX->gContext->CSSetSamplers(slot, 1, &pSampler); break;
    }
}

bool Sampler::IsCreated() {
    return (pSampler != 0);
}

void Sampler::Release() {
    if( pSampler ) pSampler->Release();
}
