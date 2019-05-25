cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct VS {
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

struct PS {
    float4 Position : SV_Position;
};

PS main(VS In) {
    PS Out;
        Out.Position = mul(mProj, mul(mView, mul(mWorld, float4(In.Position, 1.))));
    return Out;
}
