#pragma once

#include "pc.h"

#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/States/DepthStencilState.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/States/RasterState.h"
#include "Engine/States/BlendState.h"
#include "Engine/Extensions/Safe.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Input/Keyboard.h"
#include "Engine/Input/Gamepad.h"
#include "Engine/Scene/Sampler.h"
#include "Engine/ScopedMapper.h"
#include "Engine/Input/Mouse.h"
#include "Other/DrawCall.h"
#include "UIAtlas.h"

extern Mouse    *gMouse;
extern Keyboard *gKeyboard;
extern Gamepad  *gGamepad[NUM_GAMEPAD];

typedef enum class UICFlag : uint32_t {
    VScrollbar = 1,
    HScrollbar = 2, 

};

struct UIVertex {
    float3 Position;
    float2 Texcoord{};
    uint   States{};
    uint Padding{};
    float4 Color;
};

struct UIContainer {
    float3 Offset{ 0.f, 0.f, 0.f };
    float3 Size{ 1.f, 1.f, 1.f };
    bool bActive{};
    uint32_t flags;

    UIContainer() {};
    UIContainer(float3 off, float3 sz, bool act): Offset(off), Size(sz), bActive(act) {};
    UIContainer(float3 off, float3 sz);
    UIContainer(float2 off, float2 sz);
    UIContainer(float x, float y, float w, float h);

    ~UIContainer();

    void Init();
    bool Inside(const UIVertex& v) const;
    bool Inside(const UIVertex& v0, const UIVertex& v1, const UIVertex& v2) const;
    bool AtleastInside(const UIVertex& v0, const UIVertex& v1, const UIVertex& v2) const;
    UIVertex&& Clamp(const UIVertex& v) const;
};

class UIManager {
public:
    static const uint gMaxLayers = 10u;
    static const uint gMaxContainers = 10u;
    static const uint gMaxScrollbars = 10u; // Shit, no, fuck...
    static float gScaleX, gScaleY;
    
protected:
    friend struct UIContainer;
    friend class UIScrollbar;

    struct UIScrollbarState {
        float start;
        bool started;
    };

    // Per layer data for vertex buffer gen
    static std::array<VertexBuffer*, gMaxLayers>         gVBLayer;
    static std::array<uint64_t, gMaxLayers>              gDCLayer; // DataCounter
    static std::vector<uint32_t>                         gLastActiveLayers;
    static std::array<std::vector<UIVertex>, gMaxLayers> gVertexLayer;

    // Containers
    static std::array<std::array<UIContainer*, gMaxContainers>, gMaxLayers> gContainerStackLayer;
    static std::array<uint32_t, gMaxLayers>                                 gContainerStackIDLayer;
    static std::array<float3, gMaxLayers>                                   gContainerOffsetLayer;

    // Rendering
    static RenderTarget2DColor1DepthMSAA* rtDestination;
    static Shader*                        shPrimitives;
    static Shader*                        shScreen;
    static RasterState*                   rsDefault;
    static RasterState*                   rsWire;
    static BlendState*                    bsDefault;
    static DepthStencilState*             dsDefault;
    static Sampler*                       gPointSampler;
    static Sampler*                       gLinearSampler;

    // Misc
    static std::array<std::array<std::array<float2, gMaxScrollbars>, gMaxContainers>, gMaxLayers> gScrollbarContentSize;
    static std::array<std::array<std::array<UIScrollbarState*, gMaxScrollbars>, gMaxContainers>, gMaxLayers> gScrollbarState;
    static uint32_t gCirclePrecision;

public:
    static ConstantBuffer                *cbVSDataBuffer, *cbPSDataBuffer;
    struct VSDataBuffer {
        #include "Shaders/Common/UIInclude.h"
    };

    static void Init();                                                  // Init buffers
    static void Clear();                                                     // 0. Reset state
    static void Submit();                                                    // 1. Generate buffers
    static void Render(UIAtlas* atlas, void(*f)(void), bool dw=false);       // 2. Render to buffer
    static void Screen();                                                    // 3. Render to screen
    static void Release();                                               // Clean up buffers

    //static void Resize(UINT Width, UINT Height);

    static float3 GetOffset();
    static std::vector<UIVertex> GetVertexData(uint32_t LID) { return gVertexLayer[LID]; }

    static float Width();
    static float Height();
};
