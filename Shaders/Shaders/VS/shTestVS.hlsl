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
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
    float4 WorldPos : TEXCOORD1;
    float4 LightPos : TEXCOORD2;
    float3 InputPos : TEXCOORD3;
    float3 ViewDir  : TEXCOORD4;
};

PS main(VS In) {
    float4 WorldPos = mul(mWorld, float4(In.Position, 1.f));
    
    PS Out;
        Out.Position = mul(mProj, mul(mView, WorldPos));
        Out.Normal   = mul(mWorld, float4(In.Normal, 0.f)).xyz;
        Out.LightPos = mul(mLightProj, mul(mLightView, float4(WorldPos.xyz, 1.)));
        Out.ViewDir  = (vPosition.xyz - WorldPos.xyz);
        Out.Texcoord = In.Texcoord;
        Out.WorldPos = WorldPos;
        Out.InputPos = In.Position.xyz;
    return Out;
}
