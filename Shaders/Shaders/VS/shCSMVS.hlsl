cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct VS {
    float3 Position : POSITION0;
};

struct GS {
    float4 Position : SV_Position0;
};

GS main(VS In) {
    GS Out;
        Out.Position = mul(mWorld, float4(In.Position, 1.f));
    return Out;
}