#pragma once

#include <DirectXMath.h>
#include "Vendor/Assimp/Importer.hpp"


// Don't use this struct as Vertex type
// Use Vertex_P instead
struct Vertex {
    DirectX::XMFLOAT3 Position;

    virtual void LoadVertex(const aiMesh& inMesh, UINT i) = 0;
};

struct Vertex_P: public Vertex {
    void LoadVertex(const aiMesh& inMesh, UINT i) override {
        this->Position = DirectX::XMFLOAT3(inMesh.mVertices[i].x, inMesh.mVertices[i].y, inMesh.mVertices[i].z);
    };
};

struct Vertex_PT: public Vertex {
    DirectX::XMFLOAT2 Texcoord;

    void LoadVertex(const aiMesh& inMesh, UINT i) override {
        this->Position = DirectX::XMFLOAT3(inMesh.mVertices[i].x, inMesh.mVertices[i].y, inMesh.mVertices[i].z);
        if( inMesh.mTextureCoords[0] ) {
            this->Texcoord = DirectX::XMFLOAT2(inMesh.mTextureCoords[0][i].x, inMesh.mTextureCoords[0][i].y);
        } else {
            this->Texcoord = DirectX::XMFLOAT2(0, 0);
        }
    };
};

struct Vertex_PNT: public Vertex {
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 Texcoord;

    void LoadVertex(const aiMesh& inMesh, UINT i) override {
        this->Position = DirectX::XMFLOAT3(inMesh.mVertices[i].x, inMesh.mVertices[i].y, inMesh.mVertices[i].z);
        this->Normal = DirectX::XMFLOAT3(inMesh.mNormals[i].x, inMesh.mNormals[i].y, inMesh.mNormals[i].z);

        if( inMesh.mTextureCoords[0] ) {
            this->Texcoord = DirectX::XMFLOAT2(inMesh.mTextureCoords[0][i].x, inMesh.mTextureCoords[0][i].y);
        } else {
            this->Texcoord = DirectX::XMFLOAT2(0, 0);
        }
    };
};

struct Vertex_PN: public Vertex {
    DirectX::XMFLOAT3 Normal;

    void LoadVertex(const aiMesh& inMesh, UINT i) override {
        this->Position = DirectX::XMFLOAT3(inMesh.mVertices[i].x, inMesh.mVertices[i].y, inMesh.mVertices[i].z);
        this->Normal = DirectX::XMFLOAT3(inMesh.mNormals[i].x, inMesh.mNormals[i].y, inMesh.mNormals[i].z);
    };
};
