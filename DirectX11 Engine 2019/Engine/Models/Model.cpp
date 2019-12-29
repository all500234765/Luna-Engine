#include "pc.h"
#include "Model.h"

Model::Model(const char* name, int SizeOfVertex) {
    sName = name;
    mVertexSize = SizeOfVertex;
    cbBoolTexturesInst = new ConstantBuffer();
    cbBoolTexturesInst->CreateDefault(sizeof(cbBoolTextures));
}

Model::Model(const char* name) {
    sName = name;
    cbBoolTexturesInst = new ConstantBuffer();
    cbBoolTexturesInst->CreateDefault(sizeof(cbBoolTextures));
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

Texture* Model::gDefaultTexture = nullptr;
Texture* Model::gDefaultTextureOpacity = nullptr;
Texture* Model::gDefaultTextureSpecular = nullptr;
ID3D11ShaderResourceView* Model::gNullRes = nullptr;

void Model::Render(UINT num, bool bBindTextures) {
    int i = 0;
    for( Mesh* mesh : MeshBuffer ) {
        // Bind mesh settings
        mesh->Bind();

        //cbBoolTextures *cbBoolTexturesInstData = (cbBoolTextures*)cbBoolTexturesInst->Map();
        
        if( bBindTextures ) {
            // Bind textures
            int index = DiffuseMapIndex[i];
            if( index > -1 && DiffuseTextureBuffer.size() > 0 && index < DiffuseTextureBuffer.size() ) {
                DiffuseTextureBuffer[index]->Bind(Shader::Pixel, 0);
                //cbBoolTexturesInstData->bDiffuse = true;
            } else {
                // Bind default texture
                if( gDefaultTexture && bUseDefaultTexture ) {
                    gDefaultTexture->Bind(Shader::Pixel, 0);
                    //cbBoolTexturesInstData->bDiffuse = true;
                } else {
                    // Unbind any textures
                    //cbBoolTexturesInstData->bDiffuse = false;
                    gDirectX->gContext->PSSetShaderResources(1, 1, &gNullRes);
                }
            }

            index = NormalMapIndex[i];
            if( index > -1 && NormalTextureBuffer.size() > 0 && index < NormalTextureBuffer.size() ) {
                NormalTextureBuffer[index]->Bind(Shader::Pixel, 1);
                //cbBoolTexturesInstData->bNormals = true;
            } else {
                // Bind default texture
                //if( gDefaultTexture && bUseDefaultTexture ) 
                {
                    //gDefaultTexture->Bind(Shader::Pixel, 1);

                    //cbBoolTexturesInstData->bNormals = false;
                    gDirectX->gContext->PSSetShaderResources(1, 1, &gNullRes);
                }
            }

            index = OpacityMapIndex[i];
            if( index > -1 && OpacityTextureBuffer.size() > 0 && index < OpacityTextureBuffer.size() ) {
                OpacityTextureBuffer[index]->Bind(Shader::Pixel, 2);
                //cbBoolTexturesInstData->bOpacity = true;
            } else {
                // Bind default texture
                //if( gDefaultTextureOpacity && bUseDefaultTexture ) 
                {
                    gDefaultTextureOpacity->Bind(Shader::Pixel, 2);

                    //cbBoolTexturesInstData->bOpacity = false;
                    //gDirectX->gContext->PSSetShaderResources(2, 1, &gNullRes);
                }
            }

            index = SpecularMapIndex[i];
            if( index > -1 && SpecularTextureBuffer.size() > 0 && index < SpecularTextureBuffer.size() ) {
                SpecularTextureBuffer[index]->Bind(Shader::Pixel, 3);
                //cbBoolTexturesInstData->bSpecular = true;
            } else {
                // Bind default texture
                //if( gDefaultTextureSpecular && bUseDefaultTexture ) 
                {
                    //gDefaultTextureSpecular->Bind(Shader::Pixel, 3);

                    //cbBoolTexturesInstData->bSpecular = false;
                    gDirectX->gContext->PSSetShaderResources(3, 1, &gNullRes);
                }
            }

            index = RougnessMapIndex[i];
            if( index > -1 && RougnessTextureBuffer.size() > 0 && index < RougnessTextureBuffer.size() ) {
                RougnessTextureBuffer[index]->Bind(Shader::Pixel, 7);
                //cbBoolTexturesInstData->bRougness = true;
            } else {
                // Bind default texture
                //if( gDefaultTextureRougness && bUseDefaultTexture ) 
                {
                    //gDefaultTextureSpecular->Bind(Shader::Pixel, 3);

                    //cbBoolTexturesInstData->bSpecular = false;
                    gDirectX->gContext->PSSetShaderResources(7, 1, &gNullRes);
                }
            }
            
            i++;
        }

        //ZeroMemory(&cbBoolTexturesInstData->PADDING, sizeof(cbBoolTextures::PADDING));

        // Copy new constant buffer to gpu
        //memcpy(cbBoolTexturesInst->Map(), &cbBoolTexturesInstData, sizeof(cbBoolTextures));
        //cbBoolTexturesInst->Unmap();

        // Render mesh
        if( num == 1 ) mesh->Render();
        else           mesh->Render(num);
    }
}

void Model::Release() {
    for( auto m : MeshBuffer ) {
        if( m ) {
            m->Release();
            delete m;
        }
    }

    for( auto t : DiffuseTextureBuffer  ) { t->Release(); delete t; }
    for( auto t : NormalTextureBuffer   ) { t->Release(); delete t; }
    for( auto t : OpacityTextureBuffer  ) { t->Release(); delete t; }
    for( auto t : SpecularTextureBuffer ) { t->Release(); delete t; }
    for( auto t : RougnessTextureBuffer ) { t->Release(); delete t; }
}

//template<typename VertexT=Vertex_PNT> void  LoadModel(std::string fname);
//template<typename VertexT=Vertex_PNT> void  ProcessNode(aiNode* node, const aiScene* scene);
//template<typename VertexT=Vertex_PNT> Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);

/*template<typename VertexT>
void LoadModel(std::string fname) {
    Model::LoadModel<VertexT>(fname);
}*/

/*template<typename VertexT>
void ProcessNode(aiNode* node, const aiScene* scene) {
    Model::ProcessNode<VertexT>(scene);
}

template<typename VertexT>
Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    return Model::ProcessMesh(mesh, scene);
}*/
