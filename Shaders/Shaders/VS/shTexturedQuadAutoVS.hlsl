cbuffer MeshBuffer : register(b0) {
    #include "Transform.h"
}

cbuffer MatrixBuffer : register(b1) {
    #include "Camera.h"
};

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
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

PS main(uint index : SV_VertexID, uint instance : SV_InstanceID) {
    float2 UV  = arrUV[index];
    float2 Pos = arrPos[index] + instance * vPosition.xy;

    PS Out;
        Out.Position  = mul(mProj0, mul(mView0, mul(mWorld, float4(Pos, 1., 1.))));
        Out.Texcoord  = UV;
    return Out;
}
