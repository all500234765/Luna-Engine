#pragma once

#include "Engine/DirectX/DirectXChild.h"

#include "Engine Includes/EngineDirectories.h"

// 
#include "Engine/Materials/Texture.h"
#include "Engine/Materials/Sampler.h"

#include <fstream>
#include <vector>

// 
#include "Vendor/TinyDDSLoader/TinyDDSLoader.h"

class Font: public DirectXChild {
private:
    struct Character {
        int id;
        float u0, v0, u1, v1;
        float xOffset, yOffset;
        float sizeX, sizeY;
        float xStep;
    };

    int mHeight;

    Texture *tFont;
    Sampler *sFont;
    std::vector<Character> aChars;

public:
    Font(const char* file, Sampler *s);
    void Bind(UINT slot=0);

    inline void SetSampler(Sampler* s) { sFont = s; }

    inline int GetWidth (char ch) const { return aChars[ch].sizeX; }
    inline int GetHeight(char ch) const { return aChars[ch].sizeY; }

    inline int GetLineHeight() const { return mHeight; }

    inline int GetCharU0(char ch) const { return aChars[ch].u0; } // UV 0
    inline int GetCharV0(char ch) const { return aChars[ch].v0; }

    inline int GetCharU1(char ch) const { return aChars[ch].u1; } // UV 1
    inline int GetCharV1(char ch) const { return aChars[ch].v1; }

    inline DirectX::XMFLOAT2 GetCharUV0(char ch) const { return DirectX::XMFLOAT2(aChars[ch].u0, aChars[ch].v0); }
    inline DirectX::XMFLOAT2 GetCharUV1(char ch) const { return DirectX::XMFLOAT2(aChars[ch].u1, aChars[ch].v1); }

    inline int GetAdvance(char ch) const { return aChars[ch].xStep; }

    void Release();
};
