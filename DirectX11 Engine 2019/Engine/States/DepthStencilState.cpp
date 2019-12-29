#include "pc.h"
#include "DepthStencilState.h"

void DepthStencilState::Create(D3D11_DEPTH_STENCIL_DESC pDesc, UINT SRef) {
    gDirectX->gDevice->CreateDepthStencilState(&pDesc, &pState);
    StencilRef = SRef;
}

void DepthStencilState::Bind() {
    if( !pState ) return;
    gState = this;

    gDirectX->gContext->OMSetDepthStencilState(pState, StencilRef);
}

void DepthStencilState::Release() {
    if( pState ) pState->Release();
}
