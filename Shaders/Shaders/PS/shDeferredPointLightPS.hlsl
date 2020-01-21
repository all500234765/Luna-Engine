cbuffer bGlobal : register(b0) {
    #include "Deferred/Global.h"
}

cbuffer bLightData : register(b1) {
    float3 _LightDiffuse;
    float1 PADDING1;
    float2 _LightData; // Empty, Intensity
    float4 vPosition; // Light pos, w - unused
};

Texture2D<float> _DepthTexture : register(t0);
SamplerState _DepthSampler     : register(s0);

Texture2D<half2> _NormalTexture : register(t1);
SamplerState _NormalSampler     : register(s1);

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
    return _ProjValues.x / (z + _ProjValues.y);
}

float3 GetWorldPos(float2 ClipSpace, float z) {
    return mul(_mInvView, float4(ClipSpace * _ProjValues.zw * z, z, 1.)).xyz;
}

struct PS {
    float4 Position : SV_Position0;
    float2 Texcoord : TEXCOORD0;
    float4 LightPos : TEXCOORD1;
	float4 Color    : TEXCOORD2;
    uint InstanceID : TEXCOORD3;
};

half3 PointLight(float3 p, float3 n, float3 lpos) {
    float3 lDir = lpos                   - p;
    float3 eyeD = _mInvView._m03_m13_m23 - p;
    float dist = length(lDir);

    // Lambertian
    return dot(n, lDir / dist);// * _LightDiffuse * _LightData.y;

    // Specular

}

half4 main(PS In) : SV_Target0 {
    // Clip if 
    clip(In.LightPos.w < 1.f ? -1.f : 1.f); // Too small
    clip(((int)_LightCount - 1) - In.InstanceID); // Exceeds the limit
    
    // Unpack GBuffer
    float LinDepth = Depth2Linear(1.f - _DepthTexture.Sample(_DepthSampler, In.Texcoord));
    //half4 Diffuse = ;
    half3 Normal = NormalDecode(_NormalTexture.Sample(_NormalSampler, In.Texcoord));

    // Reconstruct world position
    float3 WorldPos = GetWorldPos(float2(In.Texcoord.x, 1.f - In.Texcoord.y) * 2.f - 1.f, LinDepth);

    // Calculate point light
    half4 Final = half4(In.Color.rgb * PointLight(In.Position.xyz, Normal, In.LightPos.xyz), 1.);


    return In.InstanceID / 10.f;
    return half4(half3(In.Texcoord, 0.f)*0 + 0*Normal + 1*Final.rgb + 0*(WorldPos), 1.f);
}
