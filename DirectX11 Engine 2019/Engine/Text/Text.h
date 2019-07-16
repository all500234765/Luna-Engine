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
    int mWidth, mHeight;

public:
    Text(Mesh* in, int w, int h): data(in), mWidth(w), mHeight(h) {};

    inline void SetMesh(Mesh *in) { data    = in; }
    inline void SetSizeW(int w)   { mWidth  = w ; }
    inline void SetSizeH(int h)   { mHeight = h ; }

    inline int GetSizeW() const { return mWidth ; }
    inline int GetSizeH() const { return mHeight; }

    void Clear();
    void Release(); // Reserved for future

    void Draw();
};
