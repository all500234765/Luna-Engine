#pragma once

#include "Engine/Window/Window.h"

#include "Engine/Textures/CubemapTexture.h"

#include "Effects/HDRPostProcess.h"
#include "Effects/SSAOPostProcess.h"
#include "Effects/SSLRPostProcess.h"
#include "Effects/SSLFPostProcess.h"
#include "Effects/CascadeShadowMapping.h"
#include "Effects/OrderIndendentTransparency.h"
//#include "Effects/CoverageBuffer.h"

#include "Engine/Model/Scene.h"

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

    // TODO: Add more settings; Add config file; Add console; Add console variables
};

class RendererBase {
private:


public:
    virtual void Init() { return; };
    virtual void Render() { return; };
    virtual void Release() { return; };
    virtual void ImGui() { return; };
    virtual void ClearMainRT() { return; };
    virtual void Resize(float W, float H) { return; };
    virtual void FinalScreen() { return; };

    inline uint32_t Width() const { return Window::Current()->GetCFG().CurrentWidth; }
    inline uint32_t Height() const { return Window::Current()->GetCFG().CurrentHeight2; }
};

