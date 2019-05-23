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

PS main(VS In, uint index : SV_VertexID) {
    In.Position.x--;
    In.Position.y++;

    PS Out;
        Out.Position = mul(mWorld, float4(In.Position.xy, 1., 1.));
        Out.Texcoord = float2(In.Texcoord.x, 1. - In.Texcoord.y);
    return Out;
}
