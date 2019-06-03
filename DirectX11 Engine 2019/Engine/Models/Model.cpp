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

void Model::SetDefaultTextureOpacity(Texture* def) {
    gDefaultTextureOpacity = def;
}

void Model::SetDefaultTextureSpecular(Texture* def) {
    gDefaultTextureSpecular = def;
}

Texture* Model::gDefaultTexture = 0;
Texture* Model::gDefaultTextureOpacity = 0;
Texture* Model::gDefaultTextureSpecular = 0;

void Model::Render(/*bool bBindTextures*/) {
    int i = 0;
    for( Mesh* mesh : MeshBuffer ) {
        // Bind mesh settings
        mesh->Bind();

        // Bind textures
        int index = DiffuseMapIndex[i];
        if( index > -1 && DiffuseTextureBuffer.size() > 0 && index < DiffuseTextureBuffer.size() ) {
            DiffuseTextureBuffer[index]->Bind(Shader::Pixel, 0);
        } else {
            // Bind default texture
            if( gDefaultTexture && bUseDefaultTexture ) {
                gDefaultTexture->Bind(Shader::Pixel, 0);
            }
        }

        index = NormalMapIndex[i];
        if( index > -1 && NormalTextureBuffer.size() > 0 && index < NormalTextureBuffer.size() ) {
            NormalTextureBuffer[index]->Bind(Shader::Pixel, 1);
        } else {
            // Bind default texture
            if( gDefaultTexture && bUseDefaultTexture ) {
                //gDefaultTexture->Bind(Shader::Pixel, 1);
                ID3D11ShaderResourceView* pNullRes = nullptr;
                gDirectX->gContext->PSSetShaderResources(1, 1, &pNullRes);
            }
        }

        index = OpacityMapIndex[i];
        if( index > -1 && OpacityTextureBuffer.size() > 0 && index < OpacityTextureBuffer.size() ) {
            OpacityTextureBuffer[index]->Bind(Shader::Pixel, 2);
        } else {
            // Bind default texture
            if( gDefaultTextureOpacity && bUseDefaultTexture ) {
                //gDefaultTextureOpacity->Bind(Shader::Pixel, 2);
                //ID3D11ShaderResourceView* pNullRes = nullptr;
                //gDirectX->gContext->PSSetShaderResources(2, 1, &pNullRes);
            }
        }

        index = SpecularMapIndex[i];
        if( index > -1 && SpecularTextureBuffer.size() > 0 && index < SpecularTextureBuffer.size() ) {
            SpecularTextureBuffer[index]->Bind(Shader::Pixel, 3);
        } else {
            // Bind default texture
            if( gDefaultTextureSpecular && bUseDefaultTexture ) {
                //gDefaultTextureSpecular->Bind(Shader::Pixel, 3);
                //ID3D11ShaderResourceView* pNullRes = nullptr;
                //gDirectX->gContext->PSSetShaderResources(3, 1, &pNullRes);
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
    for( auto m : MeshBuffer ) m->Release();

    for( auto t : DiffuseTextureBuffer  ) t->Release();
    for( auto t : NormalTextureBuffer   ) t->Release();
    for( auto t : OpacityTextureBuffer  ) t->Release();
    for( auto t : SpecularTextureBuffer ) t->Release();
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
