cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct GS {
    float4 Position : SV_Position;
    float3 Color    : COLOR0;
};

struct Particle {
    float2 Position;
    float2 Velocity;
    float3 Color;
};

StructuredBuffer<Particle> _Particles : register(t0);

GS main(uint index : SV_VertexID) {
    float2 Pos = _Particles[index].Position;

    GS Out;
        Out.Position  = mul(mView, float4(Pos, 1.f, 1.f));
        Out.Color     = _Particles[index].Color;
    return Out;
}
