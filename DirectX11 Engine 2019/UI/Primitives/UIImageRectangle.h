#pragma once

#include "pc.h"

#include "../UIManager.h"
#include "../UIPrimitive.h"

class UIImageRectangle: UIPrimitive {
public:
    UIImageRectangle(UIAtlasItem* img, float2 TopLeft, float2 BottomRight);
    UIImageRectangle(UIAtlasItem* img, float x0, float y0, float w, float h);
    UIImageRectangle(UIAtlasItem* img, float x0, float y0, float w, float h, float z2);
};
