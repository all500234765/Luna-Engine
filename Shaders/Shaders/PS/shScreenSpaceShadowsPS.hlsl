cbuffer cbMatrixBuffer : register(b0) {
    float4x4 _mInvView;
    float4x4 _mInvProj;
    float4x4 _mProj;
    float4x4 _mView;
};

cbuffer LightMatrixBuffer : register(b1) {
    float4x4 mLightWorld;
    float4x4 mLightView;
    float4x4 mLightProj;
    float4   vLightPosition;
};

struct LightData {
    float4x4 mLightView;
    float4x4 mLightProj;
    float3   _LightColor;
    float1   PADDING;
    float4   vLightPosition;
};

Texture2D<float1> _ScreenDepthTexture : register(t0);
SamplerState _ScreenDepthSampler      : register(s0);

Texture2D<float1> _DepthTexture       : register(t1);
SamplerComparisonState  _DepthSampler : register(s1) {
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = LESS;
};

Texture2D<float2> _NoiseTexture : register(t2);
SamplerState _NoiseSampler      : register(s2);

//StructuredBuffer<LightData> _LightBuffer : register(t3);

// Screen space to World Space
float3 GetWorldPos(float2 uv, float z) {
    float4 p = float4(uv.x * 2. - 1., 1. - 2. * uv.y, z, 1.);
    p = mul(_mInvProj, p);
    p /= p.w;
    p = mul(_mInvView, p);
    return p.xyz;
}

float SampleShadow(float4 lpos) {
    const float _ShadowMapTexel = 1.f / 2048.f;
    const float bias = .00001;// +_ShadowMapTexel;
    const float _Far = 10000.f;

    float3 projCoords = float3(.5 + (lpos.x / lpos.w) * .5,
                               .5 - (lpos.y / lpos.w) * .5,
                               (lpos.z / lpos.w));

    // Apply bias
    projCoords.z -= bias;

    // If out of bounds - no shadows should be applied
    if( saturate(projCoords.x) != projCoords.x || saturate(projCoords.y) != projCoords.y
       || projCoords.z > 1. ) return 0.;

    // Calculate dx and dy based on Noise
    float2 fNoise = _NoiseTexture.Sample(_NoiseSampler, projCoords.xy * 20.) * 2. - 1.; // * .00625;
    float2 dx = float2(fNoise.x * _ShadowMapTexel, 0.);
    float2 dy = float2(0., fNoise.y * _ShadowMapTexel);
    float2 p_dxdy = (dx + dy);
    float2 n_dxdy = (dx - dy);

    // 
    float2 samples[8] = { dx, -dx, dy, -dy, p_dxdy, -p_dxdy, n_dxdy, -n_dxdy };

    // 
    float sDepth = 0.;

    [unroll(8)]
    for( int i = 0; i < 8; i++ )
        sDepth += _DepthTexture.SampleCmpLevelZero(_DepthSampler, projCoords.xy + samples[i], projCoords.z);

    return sDepth * .125;
}

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

half4 main(PS In) : SV_Target0 {
    // Convert Screen-Space to World Space
    float z = _ScreenDepthTexture.Sample(_ScreenDepthSampler, In.Texcoord).x;
    float3 WPos = GetWorldPos(In.Texcoord, z);

    // 
    float3 Color = 0.;

    float3 _LightColor = float3(.6, .5, .7);

    // Calculate directional light shadow
    float4 LightSpace = mul(mLightProj, mul(mLightView, float4(WPos, 1.)));
    float Shadow = SampleShadow(LightSpace);

    // Add light
    Color += _LightColor * clamp(Shadow, .1, .9);

    // Result
    return half4(Color, 1.);
}
