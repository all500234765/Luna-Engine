#pragma once

#include "Engine/DirectX/DirectXChild.h"

// 
#include "Engine/Materials/Texture.h"
#include "Engine/Materials/Sampler.h"

class Font: public DirectXChild {
private:
    Texture *tFont;
    Sampler *sFont;

    UINT mCharW;
    UINT mCharH;

public:
    Font(Texture *t, Sampler *s): tFont(t), sFont(s) {};
    void Bind(UINT slot=0);

    inline int GetCharW() const { return mCharW; }
    inline int GetCharH() const { return mCharH; }


};
