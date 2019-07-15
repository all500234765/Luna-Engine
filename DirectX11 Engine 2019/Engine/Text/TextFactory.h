#pragma once

#include "Engine/DirectX/DirectXChild.h"

// 
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/Buffer.h"
#include "Engine/DirectX/ConstantBuffer.h"

// 
#include "Font.h"
#include "Text.h"

#include "Engine/Vertices.h"

#include <DirectXMath.h>

class TextFactory: public DirectXChild {
private:
    Font *mFont = nullptr;
    ConstantBuffer *mCBTransform;

    struct TextTransform {
        DirectX::XMMATRIX _mWorld;
    };

public:
    TextFactory();

    inline Font* GetFont(Font *fnt) const { return mFont; };
    inline void SetFont(Font *fnt)        { mFont = fnt; };

    Text* Build(char* text);
    Text* Build(Text* old, char* text);

    void Draw(Text* text, float x=0.f, float y=0.f);

    void Release();
};
