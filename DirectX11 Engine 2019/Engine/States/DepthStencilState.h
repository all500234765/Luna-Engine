#pragma once

#include "PipelineState.h"
#include "Engine/DirectX/DirectXChild.h"

class DepthStencilState: public PipelineState<DepthStencilState>, DirectXChild {
private:
    ID3D11DepthStencilState *pState = 0;
    UINT StencilRef = 0;

public:
    void Create(D3D11_DEPTH_STENCIL_DESC pDesc, UINT SRef=0);
    void Bind();
    void Release();
};
