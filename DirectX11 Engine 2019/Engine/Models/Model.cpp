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

void Model::Release() {
    for( int i = 0; i < num; i++ ) {
        MeshBuffer[i]->Release();
    }
}
