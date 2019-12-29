#pragma once

#include "pc.h"

// Don't use this struct as Vertex type
// Use Vertex_P instead
struct Vertex {
    Vertex() {};
    Vertex(aiMesh* inMesh, size_t i);
};

struct Vertex_P: public Vertex {
    DirectX::XMFLOAT3 Position;

    Vertex_P() {};
    Vertex_P(aiMesh* inMesh, size_t i);
};

struct Vertex_PT: public Vertex {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT2 Texcoord;

    Vertex_PT() {};
    Vertex_PT(aiMesh* inMesh, size_t i);
};

struct Vertex_PNT: public Vertex {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texcoord;

    Vertex_PNT() {};
    Vertex_PNT(aiMesh* inMesh, size_t i);
};

struct Vertex_PNTC: public Vertex {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texcoord;
    DirectX::XMFLOAT3 Color;

    Vertex_PNTC() {};
    Vertex_PNTC(aiMesh* inMesh, size_t i);
};

struct Vertex_PN: public Vertex {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;

    Vertex_PN() {};
    Vertex_PN(aiMesh* inMesh, size_t i);
};

struct Vertex_PNT_TgBn: public Vertex {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texcoord;
    DirectX::XMFLOAT3 Tangent;
    DirectX::XMFLOAT3 BiTangent;

    Vertex_PNT_TgBn() {};
    Vertex_PNT_TgBn(aiMesh* inMesh, size_t i);
};
