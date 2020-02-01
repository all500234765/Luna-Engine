cbuffer _SSLRSettings : register(b0) {
    #include "../Common/SSLRSettings.h"
}

Texture2D<float1> _Depth   : register(t0);
Texture2D<float3> _Albedo  : register(t1);
Texture2D<float3> _Normal  : register(t2);
Texture2D<float4> _Shading : register(t3);

RWTexture2D<float3> _SSLR  : register(u0);

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

[numthreads(8, 4, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
    
    
}


