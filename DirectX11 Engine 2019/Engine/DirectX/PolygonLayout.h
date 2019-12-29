#pragma once

#include "pc.h"
#include "Shader.h"
#include "Engine/States/PipelineState.h"


class PolygonLayout: public PipelineState<PolygonLayout>, DirectXChild {
private:
    ID3D11InputLayout *il;
    std::vector<D3D11_INPUT_ELEMENT_DESC> descs;
    bool Released = false;

public:
    //~PolygonLayout() { Release(); }

    ID3D11InputLayout* GetLayout();
    void Begin();
    void Push(D3D11_INPUT_ELEMENT_DESC desc);
    bool End(Shader* shader);
    void Release();
    void Bind();
    void Assign(ID3D11InputLayout* inIL);
};
