cbuffer DataBuffer : register(b0) {
    int _GroupDim; // Group number X
    uint _ParticleNum;
    uint _AffectorNum;
    float _DeltaTime;
};

struct Particle {
    float2 Position;
    float2 Velocity;
};

struct Affector {
    float2 Position;
    float1 Strength;
};

RWStructuredBuffer<Particle> _Particles : register(u0);
StructuredBuffer<Affector>   _Affectors : register(t0);

#define X 32
#define Y 24
#define DIM ((X) * (Y))

float2 Integrate(float2 anch, float2 p) {
    float2 dir = anch - p;
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
        
        [loop] for( uint i = 0; i < _AffectorNum; i++ ) {
            if( _Affectors[i].Strength != 0.f ) {
                p.Velocity += Integrate(_Affectors[i].Position, p.Position) * _Affectors[i].Strength;
            }
        }

    _Particles[index] = p;
}
