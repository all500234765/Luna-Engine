#pragma once

#include "Shader.h"
#include "DirectXChild.h"

#include <d3d11.h>
#include <vector>

class PolygonLayout: public DirectXChild {
private:
    ID3D11InputLayout *il;
    std::vector<D3D11_INPUT_ELEMENT_DESC> descs;

public:
    ID3D11InputLayout* GetLayout();
    void Begin();
    void Push(D3D11_INPUT_ELEMENT_DESC desc);
    bool End(Shader* shader);
    void Release();
    void Bind();
    void Assign(ID3D11InputLayout* inIL);
};
