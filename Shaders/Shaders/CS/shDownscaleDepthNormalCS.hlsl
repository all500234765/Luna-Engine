cbuffer Downscaling : register(b0) {
    uint2  _Res;        // Resolution
    float2 _ResRcp;     // 1.f / _Res
    float4 _ProjValues; // 
    float _OffsetRad;   // Radius for random points
    float _Radius;      // Sphere radius
    float _SSAOPower;   // Power of SSAO
    float _Alignment;   // Empty
    float4x4 _mView;    // View matrix
};

Texture2D<float> _Depth   : register(t0);
Texture2D<float2> _Normal : register(t1);
RWStructuredBuffer<float4> _DepthNDS : register(u0);

float Depth2Linear(float z) {
    return _ProjValues.z / (z + _ProjValues.w);
}

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half2 EncodeNormal(half3 n) {
    return half2(atan2(n.y, n.x) / kPI, n.z) * .5f + .5f;
}

half3 NormalDecode(half2 enc) {
    half2 scth, ang = enc * 2.f - 1.f;
    sincos(ang.x * kPI, scth.x, scth.y);

    half2 scphi = half2(sqrt(1.f - ang.y * ang.y), ang.y);
    return half3(scth.y * scphi.x, scth.x * scphi.x, scphi.y);
}

[numthreads(1024, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 CurPixel = uint2(dispatchThreadID.x % _Res.x, dispatchThreadID.x / _Res.x);

    if( CurPixel.y < _Res.y ) {
        float depth = 0.f;
        float3 avgN = 0.f;
        uint3 FullR = uint3(CurPixel * 2, 0);

        [unroll]
        for( int i = 0; i < 2; i++ ) {
            [unroll]
            for( int j = 0; j < 2; j++ ) {
                float z = _Depth.Load(FullR, int2(j, i));

                // I use inverse z-buffer
                // That's why i use max instead of min
                depth = max(z, depth);

                // Encoded normals in spherical coordinates
                float3 N = NormalDecode(_Normal.Load(FullR, int2(j, i)));
                //N = mul((float3x3)_mView, N);

                avgN += (N);
            }
        }

        // Store depth
        _DepthNDS[dispatchThreadID.x].x = Depth2Linear(1.f - depth);

        // Store normal
        float3 N = mul((float3x3)_mView, avgN * .25f);
        _DepthNDS[dispatchThreadID.x].yzw = normalize(N);
    }
}
