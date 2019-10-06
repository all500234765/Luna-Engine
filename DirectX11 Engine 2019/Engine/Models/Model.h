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

#include <iostream>
#include <string>
#include <vector>

#include "Engine/Materials/Texture.h"
#include "Engine/DirectX/Buffer.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/IndexBuffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/Models/Mesh.h"

struct BaseTexturePreset {
    bool bDiffuse;
    bool bNormals;
    bool bOpacity;
    bool bSpecular;
    bool mEmpty[11];

    BaseTexturePreset(): bDiffuse(true), bNormals(true), bOpacity(true), bSpecular(true) {};
    BaseTexturePreset(bool diff, bool norm=false, bool opac=false, bool spec=false): 
        bDiffuse(diff), bNormals(norm), bOpacity(opac), bSpecular(spec) {};
};

class Model: public DirectXChild {
protected:
    static Texture *gDefaultTexture, *gDefaultTextureOpacity, *gDefaultTextureSpecular;
    static ID3D11ShaderResourceView* gNullRes;

private:
    typedef enum {
        Diffuse, Normal, Bump, Opacity, Specular, Rougness

    } eTextureType;

    struct tTuple {
        aiString str;
        eTextureType type;

        tTuple(aiString s, eTextureType t): str(s), type(t) {};
    };

    const char* sName; // Model name
    std::vector<Mesh*> MeshBuffer;
    std::vector<tTuple*> FilenameBuffer;
    std::vector<aiString> FNB_Diffuse, FNB_Normal, FNB_Opacity, FNB_Specular, FNB_Rougness;
    std::vector<Texture*> DiffuseTextureBuffer, NormalTextureBuffer, OpacityTextureBuffer, SpecularTextureBuffer, 
                          RougnessTextureBuffer;
    std::vector<int> DiffuseMapIndex, NormalMapIndex, OpacityMapIndex, SpecularMapIndex, RougnessMapIndex;
    int num, mVertexSize;
    bool bUseDefaultTexture = true;

    ConstantBuffer *cbBoolTexturesInst;

    struct cbBoolTextures {
        bool bDiffuse;
        bool bNormals;
        bool bOpacity;
        bool bSpecular;
        bool bCubemap;
        bool PADDING[11];
    };

    //cbBoolTextures cbBoolTexturesInstData;
public:
    Model(const char* name, int SizeOfVertex);
    Model(const char* name);

    static void SetDefaultTexture(Texture* def);
    static void SetDefaultTextureOpacity(Texture* def);
    static void SetDefaultTextureSpecular(Texture* def);

    void Render();
    void Render(UINT num);

    template<typename VertexT=Vertex_PNT/*, TexturesT=DefaultTexturePreset*/> void  LoadModel(std::string fname);
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
    const aiScene *scene = importer.ReadFile(fname, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

    if( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
        std::cout << "Can't load model! (" << fname << ")" << std::endl;
        return;
    }

    // Process scene
    ProcessNode<VertexT>(scene->mRootNode, scene);

    // Load textures
    stbi_set_flip_vertically_on_load(1);
    int i = 0, j = 0, k = 0, l = 0, m = 0;
    for( tTuple* f : FilenameBuffer ) {
        // Load texture
        Texture *tex = new Texture();
        tex->Load(("../Models/" + std::string(f->str.C_Str())).c_str(), DXGI_FORMAT_R8G8B8A8_UNORM);

        switch( f->type ) {
            case eTextureType::Diffuse:
                DiffuseTextureBuffer.push_back(tex);

                // Use default texture
                if( tex->GetWidth() == 0 ) {
                    DiffuseMapIndex.at(i) = -1;
                } else {
                    std::cout << "Diffuse(" << f->str.C_Str() << ")" << std::endl;
                }

                i++;
                break;

            case eTextureType::Normal:
                NormalTextureBuffer.push_back(tex);

                // Use default texture
                if( tex->GetWidth() == 0 ) {
                    NormalMapIndex.at(j) = -1;
                } else {
                    std::cout << "Normal(" << f->str.C_Str() << ")" << std::endl;
                }

                j++;
                break;

            case eTextureType::Opacity:
                OpacityTextureBuffer.push_back(tex);

                // Use default texture
                if( tex->GetWidth() == 0 ) {
                    OpacityMapIndex.at(k) = -1;
                } else {
                    std::cout << "Opacity(" << f->str.C_Str() << ")" << std::endl;
                }

                k++;
                break;

            case eTextureType::Specular:
                SpecularTextureBuffer.push_back(tex);

                // Use default texture
                if( tex->GetWidth() == 0 ) {
                    SpecularMapIndex.at(l) = -1;
                } else {
                    std::cout << "Specular(" << f->str.C_Str() << ")" << std::endl;
                }

                l++;
                break;

            case eTextureType::Rougness:
                RougnessTextureBuffer.push_back(tex);

                // Use default texture
                if( tex->GetWidth() == 0 ) {
                    RougnessMapIndex.at(l) = -1;
                } else {
                    std::cout << "Rougness(" << f->str.C_Str() << ")" << std::endl;
                }

                m++;
                break;
        }
    }

    stbi_set_flip_vertically_on_load(0);

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
    unsigned int IndexNum = 0;

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
        aiString tFname;

        // Load diffuse textures
        tFname = " ";
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &tFname);
        if( !strcmp(tFname.C_Str(), "") || !strcmp(tFname.C_Str(), " ") || tFname.length == 0 || tFname.length == 1 ) {
            DiffuseMapIndex.push_back(-1);
        } else {
            auto index = std::find(FNB_Diffuse.begin(), FNB_Diffuse.end(), tFname);

            if( index == FNB_Diffuse.end() ) {
                // This texture wasn't loaded prev.
                DiffuseMapIndex.push_back(static_cast<int>(FNB_Diffuse.size()));
                FilenameBuffer.push_back(new tTuple(tFname, eTextureType::Diffuse));
                FNB_Diffuse.push_back(tFname);
            } else {
                DiffuseMapIndex.push_back(static_cast<int>(std::distance(FNB_Diffuse.begin(), index)));
            }
        }

        // Load normal textures
        tFname = " ";
        mat->GetTexture(aiTextureType_HEIGHT, 0, &tFname);
        if( !strcmp(tFname.C_Str(), "") || !strcmp(tFname.C_Str(), " ") || tFname.length == 0 || tFname.length == 1 ) {
            NormalMapIndex.push_back(-1);
        } else {
            auto index = std::find(FNB_Normal.begin(), FNB_Normal.end(), tFname);

            if( index == FNB_Normal.end() ) {
                // This texture wasn't loaded prev.
                NormalMapIndex.push_back(static_cast<int>(FNB_Normal.size()));
                FilenameBuffer.push_back(new tTuple(tFname, eTextureType::Normal));
                FNB_Normal.push_back(tFname);
            } else {
                NormalMapIndex.push_back(static_cast<int>(std::distance(FNB_Normal.begin(), index)));
            }
        }

        // Load opacity textures
        tFname = " ";
        mat->GetTexture(aiTextureType_OPACITY, 0, &tFname);
        if( !strcmp(tFname.C_Str(), "") || !strcmp(tFname.C_Str(), " ") || tFname.length == 0 || tFname.length == 1 ) {
            OpacityMapIndex.push_back(-1);
        } else {
            auto index = std::find(FNB_Opacity.begin(), FNB_Opacity.end(), tFname);

            if( index == FNB_Opacity.end() ) {
                // This texture wasn't loaded prev.
                OpacityMapIndex.push_back(static_cast<int>(FNB_Opacity.size()));
                FilenameBuffer.push_back(new tTuple(tFname, eTextureType::Opacity));
                FNB_Opacity.push_back(tFname);
            } else {
                OpacityMapIndex.push_back(static_cast<int>(std::distance(FNB_Opacity.begin(), index)));
            }
        }

        // Load specular textures
        tFname = " ";
        mat->GetTexture(aiTextureType_SPECULAR, 0, &tFname);
        if( !strcmp(tFname.C_Str(), "") || !strcmp(tFname.C_Str(), " ") || tFname.length == 0 || tFname.length == 1 ) {
            SpecularMapIndex.push_back(-1);
        } else {
            auto index = std::find(FNB_Specular.begin(), FNB_Specular.end(), tFname);

            if( index == FNB_Specular.end() ) {
                // This texture wasn't loaded prev.
                SpecularMapIndex.push_back(static_cast<int>(FNB_Specular.size()));
                FilenameBuffer.push_back(new tTuple(tFname, eTextureType::Specular));
                FNB_Specular.push_back(tFname);
            } else {
                SpecularMapIndex.push_back(static_cast<int>(std::distance(FNB_Specular.begin(), index)));
            }
        }

        // Load roughness textures
        tFname = " ";
        mat->GetTexture(aiTextureType_UNKNOWN, 0, &tFname);
        if( !strcmp(tFname.C_Str(), "") || !strcmp(tFname.C_Str(), " ") || tFname.length == 0 || tFname.length == 1 ) {
            RougnessMapIndex.push_back(-1);
        } else {
            auto index = std::find(FNB_Rougness.begin(), FNB_Rougness.end(), tFname);

            if( index == FNB_Rougness.end() ) {
                // This texture wasn't loaded prev.
                RougnessMapIndex.push_back(static_cast<int>(FNB_Rougness.size()));
                FilenameBuffer.push_back(new tTuple(tFname, eTextureType::Rougness));
                FNB_Rougness.push_back(tFname);
            } else {
                RougnessMapIndex.push_back(static_cast<int>(std::distance(FNB_Rougness.begin(), index)));
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
