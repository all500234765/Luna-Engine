#pragma once

#include "pc.h"

#include "../UIManager.h"
#include "../UIPrimitive.h"

class UIRoundrect: UIPrimitive {
protected:
    static const float4 lRounding;

public:
    UIRoundrect(float2 TopLeft, float2 BottomRight);
    UIRoundrect(float2 TL, float2 BR, float4 round);
    UIRoundrect(float2 TL, float2 BR, float round);

    UIRoundrect(float x0, float y0, float x1, float y1);
    UIRoundrect(float x0, float y0, float x1, float y1, float4 round);
    UIRoundrect(float x0, float y0, float x1, float y1, float round);

};
