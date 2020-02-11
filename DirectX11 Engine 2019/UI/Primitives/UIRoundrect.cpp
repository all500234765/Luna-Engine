#include "pc.h"

#include "UIRoundrect.h"

// TopLeft, TopRight, BottomRight, BottomLeft
const float4 UIRoundrect::lRounding = float4(16.f, 16.f, 16.f, 16.f);

UIRoundrect::UIRoundrect(float2 TopLeft, float2 BottomRight) {
    UIRoundrect::UIRoundrect(TopLeft.x, TopLeft.y, BottomRight.x, BottomRight.y, lRounding);
}

UIRoundrect::UIRoundrect(float2 TopLeft, float2 BottomRight, float Rounding) {
    UIRoundrect::UIRoundrect(TopLeft.x, TopLeft.y, BottomRight.x, BottomRight.y, { Rounding, Rounding, Rounding, Rounding });
}

UIRoundrect::UIRoundrect(float2 TopLeft, float2 BottomRight, float4 Rounding) {
    UIRoundrect::UIRoundrect(TopLeft.x, TopLeft.y, BottomRight.x, BottomRight.y, Rounding);
}

UIRoundrect::UIRoundrect(float x0, float y0, float x1, float y1) {
    UIRoundrect::UIRoundrect(x0, y0, x1, y1, lRounding);
}

UIRoundrect::UIRoundrect(float x0, float y0, float x1, float y1, float Rounding) {
    UIRoundrect::UIRoundrect(x0, y0, x1, y1, { Rounding, Rounding, Rounding, Rounding });
}

UIRoundrect::UIRoundrect(float x0, float y0, float x1, float y1, float4 Rounding) {
    UIVertex v0, v1, v2;
    float z = (float)gLayerID;

    v0.Color = gColor;
    v1.Color = gColor;
    v2.Color = gColor;

    // Circle center pos
    float2 tr = float2(x1 - Rounding.y, y0 + Rounding.y); // 0-90
    float2 tl = float2(x0 + Rounding.x, y0 + Rounding.x); // 90-180
    float2 bl = float2(x0 + Rounding.w, y1 - Rounding.w); // 180-270
    float2 br = float2(x1 - Rounding.z, y1 - Rounding.z); // 270-360

    /// Rectangles
    {
        /// Inside
        // Tri 1
        v0.Position = float3(tl.x, tl.y, z);
        v1.Position = float3(tr.x, tr.y, z);
        v2.Position = float3(bl.x, bl.y, z);
        AddTriangle(v0, v1, v2);

        // Tri 2
        v0.Position = float3(tr.x, tr.y, z);
        v1.Position = float3(br.x, br.y, z);
        v2.Position = float3(bl.x, bl.y, z);
        AddTriangle(v0, v1, v2);


        /// Top
        // Tri 1
        v0.Position = float3(tl.x, y0, z);
        v1.Position = float3(tr.x, y0, z);
        v2.Position = float3(tl.x, tl.y, z);
        AddTriangle(v0, v1, v2);

        // Tri 2
        v0.Position = float3(tr.x, y0, z);
        v1.Position = float3(tr.x, tr.y, z);
        v2.Position = float3(tl.x, tl.y, z);
        AddTriangle(v0, v1, v2);


        /// Bottom
        // Tri 1
        v0.Position = float3(bl.x, bl.y, z);
        v1.Position = float3(br.x, br.y, z);
        v2.Position = float3(br.x, y1, z);
        AddTriangle(v2, v1, v0);

        // Tri 2
        v0.Position = float3(bl.x, bl.y, z);
        v1.Position = float3(br.x, y1, z);
        v2.Position = float3(bl.x, y1, z);
        AddTriangle(v0, v1, v2);


        /// Left
        // Tri 1
        v0.Position = float3(x0, tl.y, z);
        v1.Position = float3(tl.x, tl.y, z);
        v2.Position = float3(x0, bl.y, z);
        AddTriangle(v0, v1, v2);

        // Tri 2
        v0.Position = float3(tl.x, tl.y, z);
        v1.Position = float3(bl.x, bl.y, z);
        v2.Position = float3(x0, bl.y, z);
        AddTriangle(v0, v1, v2);


        /// Right
        // Tri 1
        v0.Position = float3(tr.x, tr.y, z);
        v1.Position = float3(x1, tr.y, z);
        v2.Position = float3(br.x, br.y, z);
        AddTriangle(v0, v1, v2);

        // Tri 2
        v0.Position = float3(x1, tr.y, z);
        v1.Position = float3(x1, br.y, z);
        v2.Position = float3(br.x, br.y, z);
        AddTriangle(v0, v1, v2);
    }

    /// Circles
    {
        //float2 p[4] = { br, tr, tl, bl };
        float2 p[4] = { bl, br, tr, tl };
        float r[4] = { Rounding.y, Rounding.x, Rounding.w, Rounding.z };

        const float l_to_rad = 1.f / 180.f * LunaEngine::Math::PI;
        float step = 360.f / (4.f + gCirclePrecision);
        for( uint32_t i = 0.f; i < 360.f; i += step ) {
            uint32_t index = (uint32_t)std::max(0.f, fmodf(ceil(i / 90.f), 4.f));
            float R  = r[index]; // Radius
            float2 C = p[index]; // Center
            float angle = (i - step) * l_to_rad;
            float anpdx = i * l_to_rad;

            float s1 = R * sinf(angle);
            float c1 = R * cosf(angle);

            float s2 = R * sinf(anpdx);
            float c2 = R * cosf(anpdx);

            // Tri
            v0.Position = float3(C.x, C.y, z);
            v1.Position = float3(C.x + s1, C.y + c1, z);
            v2.Position = float3(C.x + s2, C.y + c2, z);

            AddTriangle(v0, v1, v2);
        }
    }
}
