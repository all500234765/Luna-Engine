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
    void LoadModel(std::string fname);
    void Release();

    void  ProcessNode(aiNode* node, const aiScene* scene);
    Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
};
