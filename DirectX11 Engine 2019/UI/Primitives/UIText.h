#pragma once

#include "pc.h"

#include "../UIManager.h"
#include "../UIPrimitive.h"

#include "Engine/Text/TextController.h"

class UIText: public UIPrimitive {
protected:
    static std::array<VertexBuffer*, gMaxLayers>         gVertexBuffers;
    static std::array<std::vector<UIVertex>, gMaxLayers> gVertices;
    static std::array<uint64_t, gMaxLayers>              gVertexNum;

    static ConstantBuffer*  cbTextEffects;
    static ConstantBuffer*  cbSDFSettings;
    static TextFactory*     gTextFactory;
    static Font*            gDefaultFont;
    static Font*            gCurrentFont;
    static Shader*          gSDFShader;

public:
    UIText(float2 pos, const char* text);
    UIText(float x, float y, const char* text);
    
    static void Init();
    static void Release();

    static void Clear();

    static void Submit();
    static void Render();


};
