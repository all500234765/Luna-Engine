#include "pc.h"

#include "UIRectangle.h"

UIRectangle::UIRectangle(float2 TopLeft, float2 BottomRight) {
    UIVertex v0, v1, v2;
    float z = (float)gLayerID;

    v0.Color = gColor;
    v1.Color = gColor;
    v2.Color = gColor;


    // Tri 1
    v0.Position = float3(TopLeft.x, TopLeft.y, z);
    v1.Position = float3(BottomRight.x, TopLeft.y, z);
    v2.Position = float3(TopLeft.x, BottomRight.y, z);
    AddTriangle(v0, v1, v2);

    // Tri 2
    v0.Position = float3(BottomRight.x, TopLeft.y, z);
    v1.Position = float3(BottomRight.x, BottomRight.y, z);
    v2.Position = float3(TopLeft.x, BottomRight.y, z);
    AddTriangle(v0, v1, v2);
}

UIRectangle::UIRectangle(float x0, float y0, float x1, float y1) {
    UIVertex v0, v1, v2;
    float z = (float)gLayerID;

    v0.Color = gColor;
    v1.Color = gColor;
    v2.Color = gColor;


    // Tri 1
    v0.Position = float3(x0, y0, z);
    v1.Position = float3(x1, y0, z);
    v2.Position = float3(x0, y1, z);
    AddTriangle(v0, v1, v2);

    // Tri 2
    v0.Position = float3(x1, y0, z);
    v1.Position = float3(x1, y1, z);
    v2.Position = float3(x0, y1, z);
    AddTriangle(v0, v1, v2);
}
