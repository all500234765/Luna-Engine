#include "pc.h"

#include "UIImageRectangle.h"

UIImageRectangle::UIImageRectangle(UIAtlasItem* img, float2 TopLeft, float2 BottomRight) {
    UIImageRectangle(img, TopLeft.x, TopLeft.y, BottomRight.x - TopLeft.x, BottomRight.y - TopLeft.y);
}

UIImageRectangle::UIImageRectangle(UIAtlasItem* img, float x0, float y0, float w, float h) {
    if( !img->usable ) return;

    float t_width  = UIAtlas::GetWidth();
    float t_height = UIAtlas::GetHeight();

    float flip   = img->flipped;
    float w_     = w;
    float h_     = h;
    float width  = img->w / t_width;
    float height = img->h / t_height;
    float u0     = img->x / t_width;
    float v0     = img->y / t_height;
    float u1     = u0 + width;
    float v1     = v0 + height;

    float2 uvs[4];
    if( flip ) {
        std::swap(w_, h_);

        uvs[0] = { u0, v1 };
        uvs[1] = { u0, v0 };
        uvs[2] = { u1, v0 };
        uvs[3] = { u1, v1 };
    } else {
        uvs[0] = { u0, v0 };
        uvs[1] = { u1, v0 };
        uvs[2] = { u1, v1 };
        uvs[3] = { u0, v1 };
    }

    // Create vertices
    float x1 = x0 + w_;
    float y1 = y0 + h_;

    UIVertex v_0{}, v_1{}, v_2{};
    float z = (float)gLayerID;

    v_0.Color = gColor; v_0.States = 0x1;
    v_1.Color = gColor; v_1.States = 0x1;
    v_2.Color = gColor; v_2.States = 0x1;

    // Tri 1
    v_0.Position = float3(x0, y0, z); v_0.Texcoord = uvs[0];
    v_1.Position = float3(x1, y0, z); v_1.Texcoord = uvs[1];
    v_2.Position = float3(x0, y1, z); v_2.Texcoord = uvs[3];
    AddTriangle(v_0, v_1, v_2);

    // Tri 2
    v_0.Position = float3(x1, y0, z); v_0.Texcoord = uvs[1];
    v_1.Position = float3(x1, y1, z); v_1.Texcoord = uvs[2];
    v_2.Position = float3(x0, y1, z); v_2.Texcoord = uvs[3];
    AddTriangle(v_0, v_1, v_2);

}
