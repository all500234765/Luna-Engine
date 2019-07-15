#include "BlendState.h"

#include <iostream>

void BlendState::Create(D3D11_BLEND_DESC pDesc, DirectX::XMFLOAT4 f, UINT SampleMask) {
    auto res = gDirectX->gDevice->CreateBlendState(&pDesc, &pState);
    if( FAILED(res) ) {
        std::cout << "[Error]: Failed to create blend state." << std::endl;
    }
}

void BlendState::Bind() {
    if( !pState ) { return; }
    float f[4] = {Factor.x, Factor.y, Factor.z, Factor.w};

    gDirectX->gContext->OMSetBlendState(pState, f, SampleMask);
}

void BlendState::Release() {
    if( pState ) pState->Release();
}
