#pragma once

#include "Engine/DirectX/DirectXChild.h"

class BlendState: public DirectXChild {
private:
    ID3D11BlendState *pState = 0;
    DirectX::XMFLOAT4 Factor = {1.f, 1.f, 1.f, 1.f};
    UINT SampleMask = 1;

public:
    void Create(D3D11_BLEND_DESC pDesc, DirectX::XMFLOAT4 f, UINT SampleMask=1);
    void Bind();
    void Release();
};
