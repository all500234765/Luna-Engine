#pragma once

#include "pc.h"

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
    float mWidth, mHeight;
    const char* mText;

public:
    Text(Mesh* in, float w, float h, const char* txt): data(in), mWidth(w), mHeight(h), mText(txt) {};

    inline void SetMesh(Mesh *in) { data    = in; }
    inline void SetSizeW(float w) { mWidth  = w ; }
    inline void SetSizeH(float h) { mHeight = h ; }

    inline float GetSizeW() const { return mWidth ; }
    inline float GetSizeH() const { return mHeight; }

    inline const char* GetText() const { return mText; }
    inline       Mesh* GetMesh() const { return data; }

    void Clear();
    void Release(); // Reserved for future

    void Draw();
};
