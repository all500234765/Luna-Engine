cbuffer _DepthResolve : register(b0) {
    uint2 _Dim;         // Depth Buffer dims
    uint1 _SampleCount; // MSAA Sample count
    uint1 _Alignment;   // Empty
}

Texture2DMS<float> _In  : register(t0);
RWTexture2D<float> _Out : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    if( DTid.x > _Dim.x || DTid.y > _Dim.y ) { return; }

    // For inverse depth buffer
    float depth = 0.f;
    
    [unroll(16)] // 32 - is max; 8 - max for me
    for( uint i = 0; i < _SampleCount; i++ )
        depth = max(depth, _In.Load(DTid.xy, i));

    _Out[DTid.xy] = depth;
}
