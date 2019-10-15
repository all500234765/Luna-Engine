#include "RasterState.h"

#include <iostream>

void RasterState::Create(D3D11_RASTERIZER_DESC pDesc) {
    auto res = gDirectX->gDevice->CreateRasterizerState(&pDesc, &pState);
    if( FAILED(res) ) {
        std::cout << "[Error]: Failed to create rasterizer state." << std::endl;
    }
}

void RasterState::Bind() {
    if( !pState ) { return; }
    gState = this;

    gDirectX->gContext->RSSetState(pState);
}

void RasterState::Release() {
    if( pState ) pState->Release();
}
