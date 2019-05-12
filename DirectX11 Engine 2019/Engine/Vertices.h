#pragma once

#include <DirectXMath.h>

struct Vertex {
    DirectX::XMFLOAT3 Position;
};

struct Vertex_P: public Vertex {
};

struct Vertex_PT: public Vertex {
    DirectX::XMFLOAT2 Texcoord;
};

struct Vertex_PNT: public Vertex {
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texcoord;
};

struct Vertex_PN: public Vertex {
    DirectX::XMFLOAT3 Normal;
};
