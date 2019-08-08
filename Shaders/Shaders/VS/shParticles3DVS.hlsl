cbuffer MatrixBuffer : register(b0) {
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   vPosition;
};

struct VS {
    float4 Position : SV_Position0;
    float3 Velocity : TEXCOORD0;
};

struct Particle {
    float3 Position;
    float3 Velocity;
};

StructuredBuffer<Particle> _Particles : register(t0);

VS main(uint index : SV_VertexID) {
    Particle p = _Particles[index];

    float3 Pos = p.Position;

    VS Out;
        Out.Position = mul(mView, float4(Pos, 1.));
        Out.Velocity = p.Velocity;
    return Out;
}
