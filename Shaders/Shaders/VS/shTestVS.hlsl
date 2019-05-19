cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

cbuffer LightMatrixBuffer : register(b1) {
    float4x4 mLightWorld;
    float4x4 mLightView;
    float4x4 mLightProj;
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
};

static float4x4 _LightMatrix = mul(mLightView, mLightProj);

PS main(VS In) {
    float4 WorldPos = mul(mWorld, float4(In.Position, 1.));

    PS Out;
        Out.Position = mul(mProj, mul(mView, WorldPos));
        Out.Texcoord = In.Texcoord;
        Out.Normal   = mul(mWorld, float4(In.Normal, 0.)).xyz;
        Out.WorldPos = WorldPos;
        Out.LightPos = mul(_LightMatrix, float4(WorldPos.xyz, 1.));
    return Out;
}
