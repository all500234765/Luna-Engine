cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition; // Light x, y, z, range
};

struct VS {
    float3 Position : POSITION0;
    float2 Texcoord : TEXCOORD0;
};

struct PS {
    float4 Position : SV_Position;
    float4 LightPos : TEXCOORD0;
    float3 vLightPs : TEXCOORD1; // View
    float4 WorldPos : TEXCOORD2;
};

PS main(VS In) {
    In.Position.xyz *= vPosition.w;

    float4 WorldPos = mul(mWorld, float4(In.Position, 1.f));

    PS Out;
        Out.Position = mul(mProj, mul(mView, WorldPos));
        Out.vLightPs = mul(mView, float4(vPosition.xyz, 1.)).xyz;
        Out.LightPos = vPosition;
        Out.WorldPos = Out.Position;
    return Out;
}
