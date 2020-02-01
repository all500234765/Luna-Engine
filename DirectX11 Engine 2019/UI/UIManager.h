#pragma once

#include "pc.h"

#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/States/DepthStencilState.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/States/RasterState.h"
#include "Engine/States/BlendState.h"
#include "Engine/Extensions/Safe.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/ScopedMapper.h"
#include "Other/DrawCall.h"

class UIManager {
public:
    static const uint gMaxLayers = 10u;
    static float gScaleX, gScaleY;

    struct UIVertex {
        float3 Position;
        float4 Color;
    };

protected:
    struct VSDataBuffer {
        #include "Shaders/Common/UIInclude.h"
    };

    static std::array<VertexBuffer*, gMaxLayers>         gVBLayer;
    static std::array<uint64_t, gMaxLayers>              gDCLayer; // DataCounter
    static std::vector<uint32_t>                         gLastActiveLayers;
    static std::array<std::vector<UIVertex>, gMaxLayers> gVertexLayer;

    // Rendering
    static ConstantBuffer                *cbVSDataBuffer, *cbPSDataBuffer;
    static RenderTarget2DColor1DepthMSAA* rtDestination;
    static Shader*                        shPrimitives;
    static Shader*                        shScreen;
    static RasterState*                   rsDefault;
    static BlendState*                    bsDefault;
    static DepthStencilState*             dsDefault;

public:
    static void Init();    // Init buffers
    static void Clear();       // 0. Reset state
    static void Submit();      // 1. Generate buffers
    static void Render();      // 2. Render to buffer
    static void Screen();      // 3. Render to screen
    static void Release(); // Clean up buffers

    //static void Resize(UINT Width, UINT Height);

    static float Width();
    static float Height();
};
