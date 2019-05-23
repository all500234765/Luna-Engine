#pragma once

#undef min
#undef max

#include "Vendor/Assimp/Importer.hpp"
#include "Vendor/Assimp/scene.h"
#include "Vendor/Assimp/postprocess.h"

#include <DirectXMath.h>

// Don't use this struct as Vertex type
// Use Vertex_P instead
struct Vertex {
    DirectX::XMFLOAT3 Position;

    Vertex() {};
    Vertex(aiMesh* inMesh, size_t i);
};

struct Vertex_P: public Vertex {
    Vertex_P() {};
    Vertex_P(aiMesh* inMesh, size_t i);
};

struct Vertex_PT: public Vertex {
    DirectX::XMFLOAT2 Texcoord;

    Vertex_PT() {};
    Vertex_PT(aiMesh* inMesh, size_t i);
};

struct Vertex_PNT: public Vertex {
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texcoord;

    Vertex_PNT() {};
    Vertex_PNT(aiMesh* inMesh, size_t i);
};

struct Vertex_PN: public Vertex {
    DirectX::XMFLOAT3 Normal;

    Vertex_PN() {};
    Vertex_PN(aiMesh* inMesh, size_t i);
};
