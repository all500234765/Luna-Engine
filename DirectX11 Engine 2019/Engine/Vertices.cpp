#include "Vertices.h"

Vertex::Vertex() {
}

Vertex_P::Vertex_P() {
}

Vertex_PT::Vertex_PT() {
}

Vertex_PNT::Vertex_PNT() {
}

Vertex_PN::Vertex_PN() {
}

Vertex::Vertex(aiMesh* inMesh, size_t i) {
    Position = DirectX::XMFLOAT3(inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z);
}

Vertex_PNT::Vertex_PNT(aiMesh* inMesh, size_t i) {
    Position = DirectX::XMFLOAT3(inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z);
    Normal = DirectX::XMFLOAT3(inMesh->mNormals[i].x, inMesh->mNormals[i].y, inMesh->mNormals[i].z);

    if( inMesh->mTextureCoords[0] ) {
        Texcoord = DirectX::XMFLOAT2(inMesh->mTextureCoords[0][i].x, inMesh->mTextureCoords[0][i].y);
    } else {
        Texcoord = DirectX::XMFLOAT2(0, 0);
    }
}

Vertex_PT::Vertex_PT(aiMesh* inMesh, size_t i) {
    Position = DirectX::XMFLOAT3(inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z);
    if( inMesh->mTextureCoords[0] ) {
        Texcoord = DirectX::XMFLOAT2(inMesh->mTextureCoords[0][i].x, inMesh->mTextureCoords[0][i].y);
    } else {
        Texcoord = DirectX::XMFLOAT2(0, 0);
    }
}

Vertex_PN::Vertex_PN(aiMesh* inMesh, size_t i) {
    Position = DirectX::XMFLOAT3(inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z);
    Normal = DirectX::XMFLOAT3(inMesh->mNormals[i].x, inMesh->mNormals[i].y, inMesh->mNormals[i].z);
}

Vertex_P::Vertex_P(aiMesh* inMesh, size_t i) {
    Position = DirectX::XMFLOAT3(inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z);
}
