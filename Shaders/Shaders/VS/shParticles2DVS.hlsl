cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct PS {
    float4 Position : SV_Position;
    float3 Color    : COLOR0;
};

struct Particle {
    float2 Position;
    float2 Velocity;
    float3 Color;
};

StructuredBuffer<Particle> _Particles : register(t0);

PS main(uint index : SV_VertexID) {
    float2 Pos = _Particles[index].Position;

    PS Out;
        Out.Position  = mul(mProj, mul(mView, mul(mWorld, float4(Pos, 1., 1.))));
        Out.Color     = _Particles[index].Color;
    return Out;
}
