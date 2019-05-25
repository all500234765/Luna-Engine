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
#include "Engine/Materials/Texture.h"

class Model {
protected:
    static Texture* gDefaultTexture;

private:
    const char* sName; // Model name
    std::vector<Mesh*> MeshBuffer;
    std::vector<aiString> FilenameBuffer;
    std::vector<Texture*> DiffuseTextureBuffer;
    std::vector<int> DiffuseMapIndex;
    int num, mVertexSize;
    bool bUseDefaultTexture = true;

public:
    Model(const char* name, int SizeOfVertex);
    Model(const char* name);

    static void SetDefaultTexture(Texture* def);

    void Render();
    void Render(UINT num);

    template<typename VertexT=Vertex_PNT> void  LoadModel(std::string fname);
    template<typename VertexT=Vertex_PNT> void  ProcessNode(aiNode* node, const aiScene* scene);
    template<typename VertexT=Vertex_PNT> Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);

    void Release();

    void DisableDefaultTexture() { bUseDefaultTexture = false; };
    void EnableDefaultTexture()  { bUseDefaultTexture = true; };
};

template<typename VertexT>
void Model::LoadModel(std::string fname) {
    if( mVertexSize == 0 ) mVertexSize = sizeof(VertexT);

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fname, aiProcess_Triangulate);

    if( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
        std::cout << "Can't load model! (" << fname << ")" << std::endl;
    }

    // Process scene
    ProcessNode<VertexT>(scene->mRootNode, scene);

    // Load textures
    stbi_set_flip_vertically_on_load(true);
    int i = 0;
    for( aiString f : FilenameBuffer ) {
        // Load diffuse texture
        Texture *tex = new Texture();
            tex->Load(("../Models/" + std::string(f.C_Str())).c_str(), DXGI_FORMAT_R8G8B8A8_UNORM);
        DiffuseTextureBuffer.push_back(tex);

        // Use default texture
        if( tex->GetWidth() == 0 ) {
            DiffuseMapIndex.at(i) = -1;
        }

        i++;
    }

    stbi_set_flip_vertically_on_load(false);

    // Clear buffer
    //FilenameBuffer.clear();
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
        VertexT v(inMesh, i);

        // Push new vertex
        vertices.push_back(v);
    }

    // Process indices
    for( size_t i = 0; i < inMesh->mNumFaces; i++ ) {
        aiFace face = inMesh->mFaces[i];
        IndexNum += face.mNumIndices;
        for( size_t j = 0; j < face.mNumIndices; j++ ) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Process materials
    // ...
    // Load texture maps
    if( inMesh->mMaterialIndex >= 0 ) {
        aiMaterial *mat = scene->mMaterials[inMesh->mMaterialIndex];

        // Load diffuse textures
        aiString tFname;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &tFname);
        
        if( tFname.C_Str() == "" ) {
            DiffuseMapIndex.push_back(-1);
        } else {
            auto index = std::find(FilenameBuffer.begin(), FilenameBuffer.end(), tFname);

            if( index == FilenameBuffer.end() ) {
                // This texture wasn't loaded prev.
                DiffuseMapIndex.push_back(FilenameBuffer.size());
                FilenameBuffer.push_back(tFname);
            } else {
                DiffuseMapIndex.push_back(std::distance(FilenameBuffer.begin(), index));
            }
        }
    }

    // Create buffers
    ib->CreateDefault(IndexNum, &indices[0]);
    vb->CreateDefault(inMesh->mNumVertices, mVertexSize, &vertices[0]);

    // Debug
    ib->SetName((std::string(sName) + std::string("'s Index Buffer")).c_str());
    vb->SetName((std::string(sName) + std::string("'s Vertex Buffer")).c_str());

    // Create mesh
    mesh->SetBuffer(vb, ib);

    // 
    return mesh;
}
