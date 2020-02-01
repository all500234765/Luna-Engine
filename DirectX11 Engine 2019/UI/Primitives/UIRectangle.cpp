#include "pc.h"

#include "UIRectangle.h"

UIRectangle::UIRectangle(float2 TopLeft, float2 BottomRight) {
    std::array<UIManager::UIVertex, 6> vert;
    UIManager::UIVertex v;
    float z = (float)gLayerID;

    v.Color = gColor;


    // Tri 1
    v.Position = float3(TopLeft.x, TopLeft.y, z);
    vert[0] = v;

    v.Position = float3(BottomRight.x, TopLeft.y, z);
    vert[1] = v;

    v.Position = float3(TopLeft.x, BottomRight.y, z);
    vert[2] = v;


    // Tri 2
    v.Position = float3(BottomRight.x, TopLeft.y, z);
    vert[3] = v;

    v.Position = float3(BottomRight.x, BottomRight.y, z);
    vert[4] = v;

    v.Position = float3(TopLeft.x, BottomRight.y, z);
    vert[5] = v;

    // 
    AddVertex(vert);
}

UIRectangle::UIRectangle(float x0, float y0, float x1, float y1) {
    std::array<UIManager::UIVertex, 6> vert;
    UIManager::UIVertex v;
    float z = (float)gLayerID;

    v.Color = gColor;


    // Tri 1
    v.Position = float3(x0, y0, z);
    vert[0] = v;

    v.Position = float3(x1, y0, z);
    vert[1] = v;

    v.Position = float3(x0, y1, z);
    vert[2] = v;


    // Tri 2
    v.Position = float3(x1, y0, z);
    vert[3] = v;

    v.Position = float3(x1, y1, z);
    vert[4] = v;

    v.Position = float3(x0, y1, z);
    vert[5] = v;

    // 
    AddVertex(vert);
}
