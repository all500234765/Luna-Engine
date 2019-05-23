cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

struct VS {
    float3 Position : POSITION0;
    float2 Texcoord : TEXCOORD0;
};

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

PS main(VS In) {
    PS Out;
        Out.Position = mul(mProj, mul(mView, mul(mWorld, float4(In.Position, 1.))));
        Out.Texcoord = float2(In.Texcoord.x, 1. - In.Texcoord.y);
    return Out;
}
