#pragma once

#include "PipelineState.h"

class RasterState: public PipelineState<RasterState> {
private:
    ID3D11RasterizerState *pState = 0;

public:
    void Create(D3D11_RASTERIZER_DESC pDesc);
    void Bind();
    void Release();
};
