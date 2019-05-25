#include "Model.h"

#include "Engine/DirectX/Buffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/IndexBuffer.h"

#include <string>
#include <iostream>
#include <vector>

Model::Model(const char* name, int SizeOfVertex) {
    sName = name;
    mVertexSize = SizeOfVertex;
}

Model::Model(const char* name) {
    sName = name;
    //mVertexSize = sizeof(Vertex_PNT);
}

void Model::SetDefaultTexture(Texture* def) {
    gDefaultTexture = def;
}

Texture* Model::gDefaultTexture = 0;

void Model::Render() {
    int i = 0;
    for( Mesh* mesh : MeshBuffer ) {
        // Bind mesh settings
        mesh->Bind();

        // Bind textures
        int index = DiffuseMapIndex[i];
        if( DiffuseTextureBuffer.size() > 0 && index < DiffuseTextureBuffer.size() ) {
            DiffuseTextureBuffer[index]->Bind(Shader::Pixel, 0);
        } else {
            // Bind default texture
            if( gDefaultTexture && bUseDefaultTexture ) {
                gDefaultTexture->Bind(Shader::Pixel, 0);
            }
        }
        
        i++;

        // Render mesh
        mesh->Render();
    }
}

void Model::Render(UINT num) {
    for( Mesh* mesh : MeshBuffer ) {
        mesh->Bind();
        mesh->Render(num);
    }
}

void Model::Release() {
    for( int i = 0; i < num; i++ ) {
        MeshBuffer[i]->Release();
    }
}

//template<typename VertexT=Vertex_PNT> void  LoadModel(std::string fname);
//template<typename VertexT=Vertex_PNT> void  ProcessNode(aiNode* node, const aiScene* scene);
//template<typename VertexT=Vertex_PNT> Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);

template<typename VertexT>
void LoadModel(std::string fname) {
    Model::LoadModel<VertexT>(fname);
}

/*template<typename VertexT>
void ProcessNode(aiNode* node, const aiScene* scene) {
    Model::ProcessNode<VertexT>(scene);
}

template<typename VertexT>
Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    return Model::ProcessMesh(mesh, scene);
}*/
