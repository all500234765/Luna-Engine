#pragma once

#include "Engine/DirectX/DirectXChild.h"

#include "Engine/DirectX/Buffer.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/IndexBuffer.h"

// Mesh building
// TODO: Use compute shader
#include "Engine/Models/Mesh.h"

class Text: public DirectXChild {
private:
    Mesh *data = nullptr;

public:
    Text(Mesh* in): data(in) {};

    inline void SetMesh(Mesh *in) { data = in; }

    void Clear();
    void Release(); // Reserved for future

    void Draw();
};
