cbuffer MeshBuffer : register(b0) {
    #include "../../../DirectX11 Engine 2019/Engine/Model/Components/Transform.h"
}

cbuffer MatrixBuffer : register(b1) {
    #include "../../../DirectX11 Engine 2019/Engine/Model/Components/Camera.h"
};

static const float2 arrPos[6] = {
    // Tri 1
    float2(+1., +1.), // Top Right
    float2(+1., -1.), // Bottom Right
    float2(-1., +1.), // Top Left
    
    // Tri 2
    float2(-1., +1.), // Top Left
    float2(+1., -1.), // Bottom Right
    float2(-1., -1.)  // Bottom Left
};

static const float2 arrUV[6] = {
    // Tri 1
    float2(1., 0.), // Top Right
    float2(1., 1.), // Bottom Right
    float2(0., 0.), // Top Left

    // Tri 2
    float2(0., 0.), // Top Left
    float2(1., 1.), // Bottom Right
    float2(0., 1.)  // Bottom Left
};

struct PS {
    float4 Position  : SV_Position;
    float2 Texcoord  : TEXCOORD0;
    float4 CameraPos : TEXCOORD1;
};

PS main(uint index : SV_VertexID) {
    float2 UV  = arrUV[index];
    float2 Pos = arrPos[index];

    PS Out;
        Out.Position  = mul(mWorld, float4(Pos, 1., 1.));
        Out.Texcoord  = UV;
        Out.CameraPos = float4(vPosition, 0.f);
    return Out;
}
