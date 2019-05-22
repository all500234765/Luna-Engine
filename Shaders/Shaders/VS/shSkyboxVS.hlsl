cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
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
        Out.Position = mul(mProj, mul(mView, WorldPos));
        Out.InputPos = In.Position.xyz;
    return Out;
}
