cbuffer MeshBuffer : register(b0) {
    #include "Transform.h"
}

cbuffer MatrixBuffer : register(b1) {
    #include "Camera.h"
};

struct VS {
    float3 Position : POSITION0;
    /*float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;*/
};

struct PS {
    float4 Position : SV_Position;
    float3 InputPos : TEXCOORD0;
};

PS main(VS In) {
    float4 WorldPos = mul(mWorld, float4(In.Position, 1.));

    PS Out;
        Out.Position = mul(mProj0, mul(mView0, WorldPos));
        Out.InputPos = In.Position.xyz;
    return Out;
}
