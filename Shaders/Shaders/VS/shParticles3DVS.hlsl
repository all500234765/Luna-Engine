cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct PS {
    float4 Position : SV_Position0;
};

struct Particle {
    float3 Position;
    float3 Velocity;
};

StructuredBuffer<Particle> _Particles : register(t0);

PS main(uint index : SV_VertexID) {
    Particle p = _Particles[index];

    float3 Pos = p.Position;

    PS Out;
        Out.Position = mul(mProj, mul(mView, float4(Pos, 1.)));
    return Out;
}
