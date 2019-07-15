#pragma once

#include "Engine/DirectX/DirectXChild.h"

class RasterState: public DirectXChild {
private:
    ID3D11RasterizerState *pState = 0;

public:
    void Create(D3D11_RASTERIZER_DESC pDesc);
    void Bind();
    void Release();
};
