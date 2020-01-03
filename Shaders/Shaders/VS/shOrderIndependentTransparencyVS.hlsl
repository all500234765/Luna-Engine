cbuffer MeshBuffer : register(b0) {
    #include "Transform.h"
}

cbuffer MatrixBuffer : register(b1) {
    #include "Camera.h"
};

struct VS {
    float3 Position  : POSITION0;
    float2 Texcoord  : TEXCOORD0;
    float3 Normal    : NORMAL0;
    /*float3 Tangent   : TANGENT0;
    float3 BiTangent : BINORMAL0;*/
};

struct PS {
    float4 Position : SV_Position0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
    float4 WorldPos : TEXCOORD1;
};

PS main(VS In) {
    float4 WorldPos = mul(mWorld, float4(In.Position , 1.f));
    float3 WorldNor = mul(mWorld, float4(In.Normal   , 0.f)).xyz;
    float4 ViewPos  = mul(mView0, WorldPos);
    
    PS Out;
        Out.Position = mul(mProj0, ViewPos);
        Out.Texcoord = In.Texcoord;
        Out.Normal   = WorldNor;
        Out.WorldPos = float4(WorldPos.xyz, Out.Position.z / Out.Position.w);
    return Out;
}
