cbuffer MeshBuffer : register(b0) {
    #include "Transform.h"
}

cbuffer MatrixBuffer : register(b1) {
    #include "Camera.h"
};

cbuffer LightMatrixBuffer : register(b2) {
    #include "Camera.h"
};

struct VS {
    float3 Position  : POSITION0;
    float2 Texcoord  : TEXCOORD0;
    float3 Normal    : NORMAL0;
    float3 Tangent   : TANGENT0;
    //float3 BiTangent : BINORMAL0;
};

struct PS {
    float4   Position : SV_Position;
    float3x3 WorldTBN : TEXCOORD0;
    float2   Texcoord : TEXCOORD3;
    float3   WorldPos : TEXCOORD4;
    float4   LightPos : TEXCOORD5;
    float3   LightPs2 : TEXCOORD6;
    float3   ViewDir  : TEXCOORD7;
};

PS main(VS In) {
    float4 WorldPos = mul(mWorld, float4(In.Position , 1.f));
    float3 WorldNor = mul(mWorld, float4(In.Normal   , 0.f)).xyz;
    float3 WorldTan = mul(mWorld, float4(In.Tangent  , 0.f)).xyz;
    WorldTan = mad(dot(WorldTan, WorldNor), -WorldNor, WorldTan); // Orthogonolize Tangent vector
    float3 WorldBiT = cross(WorldNor, WorldTan);

    // Tangent, BiTangent, Normal matrix
    float3x3 TBN = float3x3(WorldTan, WorldBiT, WorldNor);
    
    PS Out;
        Out.Position = mul(mProj0, mul(mView0, WorldPos));
        Out.WorldTBN = TBN;
        Out.LightPos = mul(mProj1, mul(mView1, float4(WorldPos.xyz, 1.)));
        Out.ViewDir  = mInvView0._m03_m13_m23 - WorldPos.xyz;
        Out.Texcoord = In.Texcoord;
        Out.WorldPos = WorldPos.xyz;
        Out.LightPs2 = mInvView1._m03_m13_m23;
    return Out;
}
