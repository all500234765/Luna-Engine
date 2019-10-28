#pragma once

#include "PipelineState.h"

class DepthStencilState: public PipelineState<DepthStencilState> {
private:
    ID3D11DepthStencilState *pState = 0;
    UINT StencilRef = 0;

public:
    void Create(D3D11_DEPTH_STENCIL_DESC pDesc, UINT SRef=0);
    void Bind();
    void Release();
};
