#pragma once

#define _____LUNA___ENGINE___DISCARD____ 1
#include "pc.h"
#include "Engine/Window/Window.h"

#include "Engine/Textures/CubemapTexture.h"

#include "Effects/HDRPostProcess.h"
#include "Effects/SSAOPostProcess.h"
#include "Effects/SSLRPostProcess.h"
#include "Effects/SSLFPostProcess.h"
#include "Effects/CascadeShadowMapping.h"
#include "Effects/OrderIndendentTransparency.h"
//#include "Effects/CoverageBuffer.h"

#include "Engine/Scene/Scene.h"

#include "Other/DrawCall.h"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_win32.h"
#include "ImGUI/imgui_impl_dx11.h"

struct RendererConfig {
    float r_width = 1366.f;
    float r_height = 768.f;
    float r_scale_x = 1.f;
    float r_scale_y = 1.f;

    float r_shadow_map_size = 2048.f;
    float r_shadow_cubemap_size = 512.f;
    //float r_shadow_csm;

    float hud_scale_x = 1.f;
    float hud_scale_y = 1.f;

    bool msaa = false;
    uint32_t msaa_levels = 8; // 1-16



    // TODO: Add more settings; Add config file; Add console; Add console variables
};

class RendererBase {
protected:
    uint32_t mMSAALevel; // 1-16
    bool mMSAA;

    bool bIsWireframe{};
    bool bDebugHUD{};

    // Draw calls
    uint32_t mTransparencyAmount{};
    uint32_t mOpaqueAmount{};

public:
    //RendererBase() {};
    virtual ~RendererBase() {};

    virtual void Init() { return; };
    virtual void Render() { return; };
    virtual void Release() { return; };
    virtual void ImGui() { return; };
    virtual void ClearMainRT() { return; };
    virtual void Resize() { return; };
    virtual void FinalScreen() { return; };
    virtual void DebugHUD() { return; }

    // Setters
    inline void SetMSAA(bool msaa) { mMSAA = msaa; }
    inline void SetMSAALevel(uint32_t level) { mMSAALevel = level; }



    // Getters
    inline bool GetMSAA() const { return mMSAA; }
    inline bool GetMSAALevel() const { return mMSAALevel; }

    inline uint32_t GetTransparencyAmount() const { return mTransparencyAmount; }
    inline uint32_t GetOpaqueAmount() const { return mOpaqueAmount; }

    inline uint32_t Width() const { return Window::Current()->GetCFG().CurrentWidth; }
    inline uint32_t Height() const { return Window::Current()->GetCFG().CurrentHeight; }
};

