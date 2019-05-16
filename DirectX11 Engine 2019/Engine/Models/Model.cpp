#include "Model.h"

#include <string>
#include <iostream>
#include <vector>

Model::Model(const char* name) {
    sName = name;
}

void Model::Render() {
    for( Mesh* mesh : MeshBuffer ) {
        mesh->Bind();
        mesh->Render();
    }
}

void Model::Render(UINT num) {
    for( Mesh* mesh : MeshBuffer ) {
        mesh->Bind();
        mesh->Render(num);
    }
}

void Model::LoadModel(std::string fname) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fname, aiProcess_Triangulate);

    if( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
        std::cout << "Can't load model! (" << fname << ")" << std::endl;
    }

    // Process scene
    ProcessNode(scene->mRootNode, scene);
}

void Model::Release() {
    for( int i = 0; i < num; i++ ) {
        MeshBuffer[i]->Release();
    }
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
    // Process meshes
    for( UINT i = 0; i < node->mNumMeshes; i++ ) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        MeshBuffer.push_back(ProcessMesh(mesh, scene));
    }

    // Process children
    for( UINT i = 0; i < node->mNumChildren; i++ ) {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh* Model::ProcessMesh(aiMesh* inMesh, const aiScene* scene) {
    // Output
    Mesh* mesh = new Mesh;
    
    // Buffers
    VertexBuffer *vb = new VertexBuffer;
    IndexBuffer  *ib = new IndexBuffer;

    // Source data
    std::vector<int> indices;
    std::vector<Vertex_PNT> vertices;// vertices.resize(inMesh->mNumVertices);
    //Vertex* vertices = (Vertex*)malloc(sizeof(Vertex_PNT) * inMesh->mNumVertices);

    // Data size
    int IndexNum = 0;

    // Load mesh data
    // Process Vertices
    for( UINT i = 0; i < inMesh->mNumVertices; i++ ) {
        Vertex_PNT v;
            
            v.Position = DirectX::XMFLOAT3(inMesh->mVertices[i].x, inMesh->mVertices[i].y, inMesh->mVertices[i].z);
            v.Normal   = DirectX::XMFLOAT3(inMesh->mNormals[i].x, inMesh->mNormals[i].y, inMesh->mNormals[i].z);
            if( inMesh->mTextureCoords[0] ) {
                v.Texcoord = DirectX::XMFLOAT2(inMesh->mTextureCoords[0][i].x, inMesh->mTextureCoords[0][i].y);
            } else {
                v.Texcoord = DirectX::XMFLOAT2(0, 0);
            }

        vertices.push_back(v);
    }

    // Process indices
    for( UINT i = 0; i < inMesh->mNumFaces; i++ ) {
        aiFace face = inMesh->mFaces[i];
        IndexNum += face.mNumIndices;
        //std::cout << face.mNumIndices << std::endl; // 
        for( UINT j = 0; j < face.mNumIndices; j++ ) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Process materials
    // ...

    // Create buffers
    ib->CreateDefault(IndexNum, &indices[0]);
    vb->CreateDefault(inMesh->mNumVertices, sizeof(Vertex_PNT), &vertices[0]);

    // Debug
    ib->SetName( (std::string(sName) + std::string("'s Index Buffer")).c_str() );
    vb->SetName( (std::string(sName) + std::string("'s Vertex Buffer")).c_str() );

    // Create mesh
    mesh->SetBuffer(vb, ib);

    // 
    return mesh;
}
