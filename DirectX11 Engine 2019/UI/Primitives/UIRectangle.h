#pragma once

#include "pc.h"

#include "../UIManager.h"
#include "../UIPrimitive.h"

class UIRectangle: UIPrimitive {
public:
    UIRectangle(float2 TopLeft, float2 BottomRight);
    UIRectangle(float x0, float y0, float x1, float y1);
};
