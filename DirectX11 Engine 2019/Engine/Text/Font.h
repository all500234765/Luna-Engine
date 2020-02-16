#pragma once

#include "pc.h"
#include "Engine/DirectX/DirectXChild.h"

#include "Engine Includes/EngineDirectories.h"

// 
#include "Engine/Scene/Texture.h"
#include "Engine/Scene/Sampler.h"

#include "Engine/States/PipelineState.h"

class Font: public PipelineState<Font>, DirectXChild {
private:
    struct Character {
        int id;
        float u0, v0, u1, v1;
        float xOffset, yOffset;
        float sizeX, sizeY;
        float xStep;
    };

    float fScale = 16.f; // Custom
    float mScale, mSpacing = 1.f, mHeight;
    bool bSDF;

    Texture *tFont;
    Sampler *sFont;
    std::vector<Character> aChars;

public:
    Font(const char* file, Sampler *s, bool SDF=false);
    void Bind(UINT slot=0);

    inline void SetSampler(Sampler* s) { sFont = s; }

    void Release();

    // Font settings
    inline bool IsSDF() const { return bSDF; }
    inline void SetSDF(bool vl) { bSDF = vl; }

    // Between [-inf; +inf]
    inline float GetLineHeight() const { return mHeight; }
    inline void SetLineHeight(float lh) { mHeight = lh; }

    // Between [0; 1]
    inline float GetSpacing() const { return mSpacing; }
    inline void SetSpacing(float v) { mSpacing = v; }

    // Original font size in texture
    // Between [0; +inf]
    inline float GetScale() const { return mScale; }

    // Character
    inline float GetWidth (char ch) const { return aChars[ch].sizeX; }
    inline float GetHeight(char ch) const { return aChars[ch].sizeY; }

    inline float GetCharU0(char ch) const { return aChars[ch].u0; } // UV 0
    inline float GetCharV0(char ch) const { return aChars[ch].v0; }

    inline float GetCharU1(char ch) const { return aChars[ch].u1; } // UV 1
    inline float GetCharV1(char ch) const { return aChars[ch].v1; }

    inline float2 GetCharUV0(char ch) const { return float2(aChars[ch].u0, aChars[ch].v0); }
    inline float2 GetCharUV1(char ch) const { return float2(aChars[ch].u1, aChars[ch].v1); }

    inline float GetAdvance(char ch) const { return aChars[ch].xStep; }

    inline float GetCharX(char ch) const { return aChars[ch].xOffset; }
    inline float GetCharY(char ch) const { return aChars[ch].yOffset; }

    inline void SetScaling(float s) { fScale = s; }
    inline float GetScaling() const { return fScale; }
};
