#include "Vertices.h"

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

Vertex_PNTC::Vertex_PNTC(aiMesh* inMesh, size_t i) {
    Position = DirectX::XMFLOAT3(inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z);
    
    if( inMesh->mColors[0] ) {
        Color = DirectX::XMFLOAT3(inMesh->mColors[0][i].r, inMesh->mColors[0][i].g, inMesh->mColors[0][i].b);
    } else {
        Color = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
    }

    if( inMesh->mTextureCoords[0] ) {
        Texcoord = DirectX::XMFLOAT2(inMesh->mTextureCoords[0][i].x, inMesh->mTextureCoords[0][i].y);
    } else {
        Texcoord = DirectX::XMFLOAT2(0, 0);
    }
}

Vertex_PNT_TgBn::Vertex_PNT_TgBn(aiMesh* inMesh, size_t i) {
    Position  = DirectX::XMFLOAT3(inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z);
    Normal    = DirectX::XMFLOAT3(inMesh->mNormals[i].x, inMesh->mNormals[i].y, inMesh->mNormals[i].z);
    
    if( inMesh->mTangents ) {
        Tangent = DirectX::XMFLOAT3(inMesh->mTangents[i].x, inMesh->mTangents[i].y, inMesh->mTangents[i].z);
    } else {
        Tangent = DirectX::XMFLOAT3(0, 0, 0);
    }

    if( inMesh->mBitangents ) {
        BiTangent = DirectX::XMFLOAT3(inMesh->mBitangents[i].x, inMesh->mBitangents[i].y, inMesh->mBitangents[i].z);
    } else {
        BiTangent = DirectX::XMFLOAT3(0, 0, 0);
    }
    
    if( inMesh->mTextureCoords[0] ) {
        Texcoord = DirectX::XMFLOAT2(inMesh->mTextureCoords[0][i].x, inMesh->mTextureCoords[0][i].y);
    } else {
        Texcoord = DirectX::XMFLOAT2(0, 0);
    }
}
