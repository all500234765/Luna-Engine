cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct GS {
    float4 Position : SV_Position;
    float4 Color    : COLOR0;
};

struct Particle {
    float2 Position;
    float2 Velocity;
    float3 Color;
    float1 Life;
};

StructuredBuffer<Particle> _Particles : register(t0);

GS main(uint index : SV_VertexID) {
    float2 Pos = _Particles[index].Position;

    GS Out;
        Out.Position  = mul(mView, float4(Pos, 1.f, 1.f));
        Out.Color     = float4(_Particles[index].Color, _Particles[index].Life);
    return Out;
}
