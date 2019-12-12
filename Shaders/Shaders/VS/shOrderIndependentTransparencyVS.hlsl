cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct VS {
    float3 Position  : POSITION0;
    float3 Normal    : NORMAL0;
    float2 Texcoord  : TEXCOORD0;
    float3 Tangent   : TANGENT0;
    float3 BiTangent : BINORMAL0;
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
    
    PS Out;
        Out.Position = mul(mProj, mul(mView, WorldPos));
        Out.Texcoord = In.Texcoord;
        Out.Normal   = WorldNor;
        Out.WorldPos = WorldPos;
    return Out;
}
