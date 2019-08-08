cbuffer DataBuffer : register(b0) {
    int _GroupDim; // Group number X
    uint _ParticleNum;
    float _DeltaTime;
    float _Dummy;
};

struct Particle {
    float3 Position;
    float3 Velocity;
};

RWStructuredBuffer<Particle> _Particles : register(u0);

#define X 32
#define Y 24
#define DIM ((X) * (Y))

float3 Integrate(float3 anch, float3 p) {
    float3 dir = anch - p;
    float dist = length(dir);
    dir /= dist;

    return dir * max(.01f, 1.f / (dist * dist));
}

[numthreads(X, Y, 1)]
void main(uint GroupIndex : SV_GroupIndex, uint3 GroupID : SV_GroupID) {
    uint index = GroupIndex + X * Y * (_GroupDim * GroupID.y + GroupID.x);

    // If index is over actual particle amount
    [flatten] if( index >= _ParticleNum ) { return; }

    // Integrate particle
    Particle p = _Particles[index];
        p.Position += p.Velocity * _DeltaTime;
        p.Velocity += Integrate(+float3(200.f, 0.f, 0.f), p.Position);
        p.Velocity += Integrate(-float3(200.f, 0.f, 0.f), p.Position);
    _Particles[index] = p;
}
