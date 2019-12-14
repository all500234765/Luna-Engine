cbuffer Downscaling : register(b0) {
    uint2  _Res;        // Resolution
    float2 _ResRcp;     // 1.f / _Res
    float2 _ProjValues; // 
    int1   _DownScale;  // Downscale on both axis
    int1   _DownScale2; // Downscale log2
};

Texture2D<float> _DepthIn  : register(t0);
RWTexture2D<float4> _Depth : register(u0);

float Depth2Linear(float z) {
    return _ProjValues.x / (z + _ProjValues.y);
}

[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 CurPixel = dispatchThreadID.xy;
    
    float depth = 0.f;
    uint3 FullR = uint3(CurPixel * _DownScale, 0);

    [unroll(8)]
    for( int i = 0; i < _DownScale; i++ ) {
        [unroll(8)]
        for( int j = 0; j < _DownScale; j++ ) {
            float z = _DepthIn.Load(FullR, int2(j, i));

            // I use inverse z-buffer
            // That's why i use max instead of min
            depth = max(z, depth);
        }
    }

    // Store depth
    _Depth[CurPixel] = 1.f - depth; //Depth2Linear(1.f - depth);
}
