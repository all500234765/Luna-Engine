Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0);

Texture2D<half2> _NormalTexture : register(t1);
SamplerState _NormalSampler     : register(s1);

Texture2D<float4> _DepthTexture : register(t2);
SamplerState _DepthSampler      : register(s2);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
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

half4 main(PS In) : SV_Target0 {
    // 
    half4 Diff = _Texture.Sample(_Sampler, In.Texcoord);

    // 
    return Diff;
}
