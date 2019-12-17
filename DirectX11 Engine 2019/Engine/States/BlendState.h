#pragma once

#include "PipelineState.h"
#include "Engine/DirectX/DirectXChild.h"

class BlendState: public PipelineState<BlendState>, DirectXChild {
private:
    ID3D11BlendState *pState = 0;
    DirectX::XMFLOAT4 Factor = { 1.f, 1.f, 1.f, 1.f };
    UINT SampleMask = 0xFFFFFFFF;

public:
    void Create(D3D11_BLEND_DESC pDesc, DirectX::XMFLOAT4 f, UINT SampleMask= 0xFFFFFFFF);
    void Bind();
    void Release();
};
