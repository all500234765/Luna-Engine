cbuffer cbMatrixBuffer : register(b0) {
    float4x4 _mInvView;
    float4x4 _mInvProj;
    float4x4 _mProj;
    float4x4 _mView;
};

Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0);

Texture2D<half2> _NormalTexture : register(t1);
SamplerState _NormalSampler     : register(s1);

Texture2D<float4> _DepthTexture : register(t2);
SamplerState _DepthSampler      : register(s2);

struct PS {
    float4 Position  : SV_Position;
    float2 Texcoord  : TEXCOORD0;
    float4 CameraPos : TEXCOORD1;
};

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half3 NormalDecode(half2 enc) {
    half2 scth, ang = enc * 2.f - 1.f;
    sincos(ang.x * kPI, scth.x, scth.y);

    half2 scphi = half2(sqrt(1.f - ang.y * ang.y), ang.y);
    return half3(scth.y * scphi.x, scth.x * scphi.x, scphi.y);
}

// Screen space to World Space
float3 GetWorldPos(float2 uv, float z) {
    float4 p = float4(uv.x * 2. - 1., 1. - 2. * uv.y, z, 1.);
    p = mul(_mInvProj, p);
    p /= p.w;
    p = mul(_mInvView, p);
    return p.xyz;
}

// World Space to Screen Space
float3 GetUV(float3 p) {
    float4 vp = mul(_mView, float4(p, 1.));
    float4 pp = mul(_mProj, vp);
    pp /= pp.w;
    return float3(.5 + float2(.5, -.5) * pp.xy, pp.z);
}

half4 main(PS In) : SV_Target0 {
    // 
    half3 Normal = NormalDecode(_NormalTexture.Sample(_NormalSampler, In.Texcoord));
    float Depth  = _DepthTexture.Sample(_DepthSampler, In.Texcoord).x;

    // 
    float3 WPos = GetWorldPos(In.Texcoord, Depth);    // World position
    float3 vDir = normalize(WPos - In.CameraPos.xyz); // View direction
    float3 rDir = normalize(reflect(vDir, Normal));   // Reflect dir

    // Ray marching
    float3 cRay = 0.;
    float3 uv   = 0.;
    float L = .01;

    float error = 1.;

    for( int i = 0; i < 10; i++ ) {
        cRay = WPos + rDir * L;

        uv = GetUV(cRay);
        float z = _DepthTexture.Sample(_DepthSampler, uv.xy).x;

        float3 nPos = GetWorldPos(uv.xy, z);
        L = length(WPos - nPos);

        // 
        L = saturate(L * .9);
        error *= 1. - L;
    }

    // 
    half4 Diff = _Texture.Sample(_Sampler, uv.xy);
    return half4(Diff.rgb * error, 1.);
}
