#pragma once

#include "Engine Includes/MainInclude.h"

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
    virtual void Init() = 0;
    virtual void Render(uint32_t) = 0;
    virtual void Release() = 0;
    virtual void ImGui() = 0;

};

