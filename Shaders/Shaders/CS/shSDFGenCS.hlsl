RWTexture3D<float> _Dest : register(u0);

ByteAddressBuffer _Position : register(t0);
ByteAddressBuffer _Normal   : register(t1);
ByteAddressBuffer _Index    : register(t2);

cbuffer cbDims : register(b0) {
    uint _VertexCount;
    uint _IndexCount;
    uint _Width;
    uint _Height;
    
    uint _Depth;
    uint _Spread;
    uint2 _Align;
};

[numthreads(1, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
    float SDF = 0.f;
    float s = 20.f; //_Spread
	float3 P = dtid * s;
	

    for( uint t = 0; t < _IndexCount; t += 3 ) {
        // Load indices of triangle from index buffer
        uint i0 = _Index.Load(t + 0) * 3;
        uint i1 = _Index.Load(t + 2) * 3;
        uint i2 = _Index.Load(t + 1) * 3;

        // Load vertices of triangle from vertex buffer:
        float3 p0 = asfloat(_Position.Load3(i0));
        float3 p1 = asfloat(_Position.Load3(i1));
        float3 p2 = asfloat(_Position.Load3(i2));
        
		[flatten] if( distance(p0, P) > distance(SDF, P) ) SDF = p0;
		[flatten] if( distance(p1, P) > distance(SDF, P) ) SDF = p1;
		[flatten] if( distance(p2, P) > distance(SDF, P) ) SDF = p2;
		
        [flatten]
        if( distance(p0, P) > s * 2.f
         && distance(p1, P) > s * 2.f
         && distance(p2, P) > s * 2.f ) continue;
        
        SDF += asfloat(_Normal.Load3(i0)); //dot(asfloat(_Normal.Load3(i0)), normalize(P));
    }
    
    _Dest[dtid] = SDF; //(dtid.x + dtid.y * _Width + dtid.z * _Width * _Height);
}
