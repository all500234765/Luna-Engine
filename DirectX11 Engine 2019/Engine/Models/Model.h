#pragma once

#ifdef _WIN64
#pragma comment(lib, "Assimp/x64/assimp-x64.lib")
#else
#pragma comment(lib, "Assimp/x86/assimp-x86.lib")
#endif

#undef min
#undef max
#include "Vendor/Assimp/Importer.hpp"
#include "Vendor/Assimp/scene.h"
#include "Vendor/Assimp/postprocess.h"

#include <string>
#include <iostream>
#include <vector>

#include "Engine/Models/Mesh.h"

class Model {
private:
    const char* sName; // Model name
    std::vector<Mesh*> MeshBuffer;
    int num;

public:
    Model(const char* name);

    void Render();
    void Render(UINT num);
    
    template<typename VertexT=Vertex_PNT> void  LoadModel(std::string fname);
    template<typename VertexT=Vertex_PNT> void  ProcessNode(aiNode* node, const aiScene* scene);
    template<typename VertexT=Vertex_PNT> Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);

    void Release();
};

template<typename VertexT>
void Model::LoadModel(std::string fname) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fname, aiProcess_Triangulate);

    if( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
        std::cout << "Can't load model! (" << fname << ")" << std::endl;
    }

    // Process scene
    ProcessNode<VertexT>(scene->mRootNode, scene);
}

template<typename VertexT>
void Model::ProcessNode(aiNode* node, const aiScene* scene) {
    // Process meshes
    for( size_t i = 0; i < node->mNumMeshes; i++ ) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        MeshBuffer.push_back(ProcessMesh<VertexT>(mesh, scene));
    }

    // Process children
    for( size_t i = 0; i < node->mNumChildren; i++ ) {
        ProcessNode<VertexT>(node->mChildren[i], scene);
    }
}

template<typename VertexT>
Mesh* Model::ProcessMesh(aiMesh* inMesh, const aiScene* scene) {
    // Output
    Mesh* mesh = new Mesh;

    // Buffers
    VertexBuffer *vb = new VertexBuffer;
    IndexBuffer  *ib = new IndexBuffer;

    // Source data
    std::vector<int> indices;
    std::vector<VertexT> vertices;// vertices.resize(inMesh->mNumVertices);
    //Vertex* vertices = (Vertex*)malloc(sizeof(Vertex_PNT) * inMesh->mNumVertices);

    // Data size
    int IndexNum = 0;

    // Load mesh data
    // Process Vertices
    for( size_t i = 0; i < inMesh->mNumVertices; i++ ) {
        // Create new vertex
        VertexT v;

        // Load new vertex
        v.LoadVertex(inMesh, i);

        // Push new vertex
        vertices.push_back(v);
    }

    // Process indices
    for( size_t i = 0; i < inMesh->mNumFaces; i++ ) {
        aiFace face = inMesh->mFaces[i];
        IndexNum += face.mNumIndices;
        //std::cout << face.mNumIndices << std::endl; // 
        for( size_t j = 0; j < face.mNumIndices; j++ ) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Process materials
    // ...

    // Create buffers
    ib->CreateDefault(IndexNum, &indices[0]);
    vb->CreateDefault(inMesh->mNumVertices, sizeof(VertexT), &vertices[0]);

    // Debug
    ib->SetName((std::string(sName) + std::string("'s Index Buffer")).c_str());
    vb->SetName((std::string(sName) + std::string("'s Vertex Buffer")).c_str());

    // Create mesh
    mesh->SetBuffer(vb, ib);

    // 
    return mesh;
}
