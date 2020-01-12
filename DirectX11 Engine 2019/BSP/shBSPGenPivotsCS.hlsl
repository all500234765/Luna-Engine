struct Pivot {
    uint front, back;
};

Buffer<float3>   _Position : register(t0);
ByteAddressBuffer _Index   : register(t1);

RWStructuredBuffer<Pivot> _Pivot : register(u0);

cbuffer cbData : register(b0) {
    uint _IndexCount;
    uint _VertexCount;
    uint _Pivot;
    uint _StartIndex;
};

struct Plane {
    float3 n;
    float p;
};

uint SplitType(uint a, uint b, uint c) {
    return (a + 1) * 16 + (b + 1) * 4 + c;
}

Plane CalculatePlane(float3 v0, float3 v1, float3 v2) {
    Plane p;
        p.n = normalize(cross(v1 - v0, v2 - v0));
        p.p = dot(p.n, v0);
    return p;
}

float Plane2VertexSign(Plane p, float3 v) {
    return sign(dot(p.n, v) - p.p);
}

Pivot EvaluatePivot(int pivot) {
    Pivot e;
    e.back = 0;
    e.front = 0;
    
    uint index = _Index.Load(pivot);
    Plane plane = CalculatePlane(_Position[index + 0], 
                                 _Position[index + 1], 
                                 _Position[index + 2]);
    
    for( uint i = 0; i < _IndexCount; i += 3 ) {
        uint split = SplitType(Plane2VertexSign(plane, _Position[i + 0], 
                                                       _Position[i + 1], 
                                                       _Position[i + 2]));
        
        
        switch( split ) {
            case SplitType(-1, -1, -1): //  0 + 0 + 0 = 0; 
            case SplitType(-1, -1,  0): //  0 + 0 + 1 = 1;
            case SplitType(-1,  0, -1): //  0 + 4 + 0 = 4;
            case SplitType(-1,  0,  0): //  0 + 4 + 1 = 5;
            case SplitType( 0, -1, -1): // 16 + 0 + 0 = 16; (split & 0x11) > 0
            case SplitType( 0, -1,  0): // 16 + 0 + 1 = 17; 
            case SplitType( 0,  0, -1): // 16 + 4 + 0 = 20; frac(split / 20.f) == 0.f
              e.back++;
              break;

            case SplitType( 0,  0,  1): // 16 + 4 + 2 = 22
            case SplitType( 0,  1,  0): // 16 + 8 + 1 = 25
            case SplitType( 0,  1,  1): // 16 + 8 + 2 = 26
            case SplitType( 1,  0,  0): // 16 + 4 + 1 = 37
            case SplitType( 1,  0,  1): // 32 + 4 + 2 = 38
            case SplitType( 1,  1,  0): // 32 + 8 + 1 = 41
            case SplitType( 1,  1,  1): // 32 + 8 + 2 = 42
              e.front++;
              break;

            //case SplitType( 0,  0,  0): break;

            case SplitType(-1, -1,  1): //  0 + 0 + 2 = 2
            case SplitType(-1,  1, -1): //  0 + 8 + 0 = 8
            case SplitType( 1, -1, -1): // 32 + 0 + 0 = 32
                e.back += 2;
                e.front++;
                break;

            case SplitType(-1,  0,  1): // 
            case SplitType( 1,  0, -1): // 
            case SplitType(-1,  1,  0): // 
            case SplitType( 1, -1,  0): // 
            case SplitType( 0, -1,  1): // 
            case SplitType( 0,  1, -1): // 
                e.back++;
                e.front++;
                break;

            case SplitType(-1,  1,  1): // 
            case SplitType( 1, -1,  1): // 
            case SplitType( 1,  1, -1): // 
                e.back++;
                e.front += 2;
                break;
        }
    }
    
    return Pivot;
}

[numthreads(1, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
    if( dtid.x < _IndexCount ) {
        _Pivot[_StartIndex + dtid.x] = EvaluatePivot(_StartIndex + dtid.x);
    }
    
}
