cbuffer bGlobal : register(b0) {
    float2 _TanAspect;  // dtan(fov * .5) * aspect, - dtan(fov / 2)
    float2 _Texel;      // 1 / target width, 1 / target height
    float1 _Far;        // Far
    float1 PADDING0;    // 
    float4 _ProjValues; // 1 / m[0][0], 1 / m[1][1], m[3][2], -m[2][2]
    float4x4 _mInvView; // 
}

cbuffer bLightData : register(b1) {
    float3 _LightDiffuse;
    float1 PADDING1;
    float2 _LightData; // Empty, Intensity
    float4 PADDING2;
};

Texture2D<half2> _NormalTexture : register(t0);
SamplerState _NormalSampler     : register(s0);

Texture2D _DepthTexture    : register(t1);
SamplerState _DepthSampler : register(s1);

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half3 NormalDecode(half2 enc) {
    half2 scth, ang = enc * 2.f - 1.f;
    sincos(ang.x * kPI, scth.x, scth.y);

    half2 scphi = half2(sqrt(1.f - ang.y * ang.y), ang.y);
    return half3(scth.y * scphi.x, scth.x * scphi.x, scphi.y);
}

float Depth2Linear(float z) {
    return _ProjValues.z / (z + _ProjValues.w);
}

float3 GetWorldPos(float2 ClipSpace, float lDepth) {
    float4 p = float4(ClipSpace * _ProjValues.xy * lDepth, lDepth, 1.);
    return mul(_mInvView, p).xyz;
}

struct PS {
    float4 Position : SV_Position0;
    float2 Texcoord : TEXCOORD0;
    float3 LightPos : TEXCOORD1;
    
};

half4 main(PS In) : SV_Target0 {
    // Unpack GBuffer
    int3 Location = int3(In.Position.xy, 0);

    float LinDepth = Depth2Linear(_DepthTexture.Load(Location).x);
    //half4 Diffuse = ;
    half3 Normal = NormalDecode(_NormalTexture.Load(Location));

    // Reconstruct world position
    float3 WorldPos = GetWorldPos(In.Texcoord, LinDepth);

    // 
    half4 Final = half4((LinDepth.xxx) * 100., 1.);


    return Final;
}
