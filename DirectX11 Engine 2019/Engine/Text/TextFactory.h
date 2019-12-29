#pragma once

#include "pc.h"
#include "Engine/DirectX/DirectXChild.h"

// 
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/Buffer.h"
#include "Engine/DirectX/ConstantBuffer.h"

// 
#include "Font.h"
#include "Text.h"

// 
#include "Engine/Vertices.h"

enum TextFlags {
    TextAlignment_V_Top    = 1,
    TextAlignment_V_Center = 2,
    TextAlignment_V_Bottom = 4,
    
    TextAlignment_H_Left   = 8,
    TextAlignment_H_Middle = 16,
    TextAlignment_H_Right  = 32,
};

struct TextEffects {
    // Glow


    // Outline


    // Color
    DirectX::XMFLOAT4 _Color;

    // Etc...


};

struct SDFSettings {
    float _CharWidth   = .4f;
    float _Softening   = .1f;
    float _BorderWidth = .5f;
    float _BorderSoft  = .1f;
};

class TextFactory: public DirectXChild {
private:
    UINT TextAlignment = TextAlignment_V_Top | TextAlignment_H_Left;
    
    Shader *mShader;
    Font *mFont = nullptr;

    ConstantBuffer *cbTextEffects, *cbSDFSettings;
public:
    TextFactory(Shader* shader);

    inline UINT GetAligment()   const { return  TextAlignment; }
    inline UINT GetHAlignment() const { return (TextAlignment & 0x38); }
    inline UINT GetVAlignment() const { return (TextAlignment & 0x07); }

    inline void SetAlignment(UINT align)      { TextAlignment  =  align; }
    inline void AddAlignment(TextFlags align) { TextAlignment |=  align; }
    inline void DelAlignment(TextFlags align) { TextAlignment &= ~align; }
    inline void DelAlignment(bool hor)        { TextAlignment &= ~(7 << (hor ? 3 : 0)); } // TODO: Test

    inline Font* GetFont() const   { return mFont; };
    inline void SetFont(Font *fnt) { mFont = fnt; };

    Text* Build(const char* text, float maxWidth=-1);
    Text* Build(Text* old, const char* text, float maxWidth=-1);

    void Draw(Text* text);

    void Release();

    inline TextEffects* MapTextEffects() const { return (TextEffects*)cbTextEffects->Map(); };
    inline void UnmapTextEffects() const { cbTextEffects->Unmap(); }

    inline SDFSettings* MapSDFSettings() const { return (SDFSettings*)cbSDFSettings->Map(); };
    inline void UnmapSDFSettings() const { cbSDFSettings->Unmap(); }
};
