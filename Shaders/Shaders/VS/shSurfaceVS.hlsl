cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

cbuffer LightMatrixBuffer : register(b1) {
    float4x4 mLightWorld;
    float4x4 mLightView;
    float4x4 mLightProj;
    float4   vLightPosition;
};

struct VS {
    float3 Position  : POSITION0;
    float3 Normal    : NORMAL0;
    float2 Texcoord  : TEXCOORD0;
    float3 Tangent   : TANGENT0;
    float3 BiTangent : BINORMAL0;
};

struct PS {
    float4   Position : SV_Position;
    float3x3 WorldTBN : TEXCOORD0;
    float2   Texcoord : TEXCOORD3;
    float3   WorldPos : TEXCOORD4;
    float4   LightPos : TEXCOORD5;
    float3   InputPos : TEXCOORD6;
    float3   ViewDir  : TEXCOORD7;
};

PS main(VS In) {
    float4 WorldPos = mul(mWorld, float4(In.Position , 1.f));
    float3 WorldNor = mul(mWorld, float4(In.Normal   , 0.f)).xyz;
    float3 WorldTan = mul(mWorld, float4(In.Tangent  , 0.f)).xyz;
    float3 WorldBiT = mul(mWorld, float4(In.BiTangent, 0.f)).xyz;

    // Tangent, BiTangent, Normal matrix
    float3x3 TBN = float3x3(WorldTan, WorldBiT, WorldNor);

    PS Out;
        Out.Position = mul(mProj, mul(mView, WorldPos));
        Out.WorldTBN = TBN;
        Out.LightPos = mul(mLightProj, mul(mLightView, float4(WorldPos.xyz, 1.)));
        Out.ViewDir  = mul(TBN, vPosition.xyz - WorldPos.xyz);
        Out.Texcoord = In.Texcoord;
        Out.WorldPos = mul(TBN, WorldPos.xyz);
        Out.InputPos = In.Position.xyz;
    return Out;
}
