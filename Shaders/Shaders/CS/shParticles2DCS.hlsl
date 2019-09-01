cbuffer DataBuffer : register(b0) {
    int _GroupDim; // Group number X
    uint _ParticleNum;
    uint _AffectorNum;
    float _DeltaTime;

    float2 _EmitterPosition;
    float1 _MaxLifeTime;
    float1 _Dummy;
};

struct Particle {
    float2 Position;
    float2 Velocity;
    float3 Color;
    float1 Life;
};

struct Affector {
    float2 Position;
    float1 Radius;
    float1 Strength;
};

RWStructuredBuffer<Particle> _Particles : register(u0);
StructuredBuffer<Affector>   _Affectors : register(t0);

#define X 32
#define Y 24
#define DIM ((X) * (Y))

float2 Integrate(float2 anch, float2 p, float r) {
    float2 dir = anch - p;
    float dist = length(dir);
    dir /= dist;

    return dir * max(.00001f, r / (dist * dist));
}

[numthreads(X, Y, 1)]
void main(uint GroupIndex : SV_GroupIndex, uint3 GroupID : SV_GroupID) {
    // Particle index
    uint index = GroupIndex + X * Y * (_GroupDim * GroupID.y + GroupID.x);

    // If index is over actual particle amount
    [flatten] if( index >= _ParticleNum ) { return; }

    // Integrate particle
    Particle p = _Particles[index];
        // Kill particle?
        [flatten] if( p.Life >= _MaxLifeTime ) {
            // No! Just reset it!
            float1 t = p.Life - sin(_DeltaTime * 3.14159f) * 3.14159f * .5f;
            float2 dir = 0.f * float2(cos(t), sin(t));

            p.Position = _EmitterPosition - 10.f * dir;
            p.Velocity = -dir * 100.f;

            // 
            while( p.Life >= _MaxLifeTime ) p.Life -= _MaxLifeTime;
        }
        
        // Integrate over time
        p.Position += p.Velocity * _DeltaTime;
        
        // Integrate velocity
        [loop] for( uint i = 0; i < _AffectorNum; i++ ) {
            [flatten] if( _Affectors[i].Strength != 0.f ) {
                p.Velocity += Integrate(_Affectors[i].Position, p.Position, _Affectors[i].Radius) * _Affectors[i].Strength;
            }
        }

        // It must live!
        p.Life += _DeltaTime;

    _Particles[index] = p;
}
