cbuffer PlayerMatrixBuffer : register(b0) {
    #include "Camera.h"
};

cbuffer Settings : register(b1) {
    //                              fQ = fFar / (fNear - fFar);
    float4 _ProjValues;  // fNear * fQ, fQ, 1 / m[0][0], 1 / m[1][1] // Player
    float2 _Scaling;     // Width, Height / Downscaling Factor
    uint _FrameIndex;    // 0 -> Interleaved; based on frame index
    uint _Interleaved;   // pow(2, n)
    float _OcclusionRadius;
    float _OcclusionMaxDistance;
    uint2 _Padding;
};

static const int num_samples = 32;

static const float3 points[] = {
	float3(-0.134, 0.044, -0.825),
	float3(0.045, -0.431, -0.529),
	float3(-0.537, 0.195, -0.371),
	float3(0.525, -0.397, 0.713),
	float3(0.895, 0.302, 0.139),
	float3(-0.613, -0.408, -0.141),
	float3(0.307, 0.822, 0.169),
	float3(-0.819, 0.037, -0.388),
	float3(0.376, 0.009, 0.193),
	float3(-0.006, -0.103, -0.035),
	float3(0.098, 0.393, 0.019),
	float3(0.542, -0.218, -0.593),
	float3(0.526, -0.183, 0.424),
	float3(-0.529, -0.178, 0.684),
	float3(0.066, -0.657, -0.570),
	float3(-0.214, 0.288, 0.188),
	float3(-0.689, -0.222, -0.192),
	float3(-0.008, -0.212, -0.721),
	float3(0.053, -0.863, 0.054),
	float3(0.639, -0.558, 0.289),
	float3(-0.255, 0.958, 0.099),
	float3(-0.488, 0.473, -0.381),
	float3(-0.592, -0.332, 0.137),
	float3(0.080, 0.756, -0.494),
	float3(-0.638, 0.319, 0.686),
	float3(-0.663, 0.230, -0.634),
	float3(0.235, -0.547, 0.664),
	float3(0.164, -0.710, 0.086),
	float3(-0.009, 0.493, -0.038),
	float3(-0.322, 0.147, -0.105),
	float3(-0.554, -0.725, 0.289),
	float3(0.534, 0.157, -0.250),
};

Texture2D<float> _Depth : register(t0);
Texture2D<float2> _Normal : register(t1);

RWTexture2D<float4> _Accumulation : register(u0);

// 
float3 GetWorldPos(float2 ClipSpace, float z) {
    return mul(mInvView0, float4(ClipSpace * _ProjValues.zw * z, z, 1.)).xyz;
}

float Depth2Linear(float z) {
    return _ProjValues.x / (z + _ProjValues.y);
}

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half3 NormalDecode(half2 enc) {
    half2 scth, ang = enc * 2.f - 1.f;
    sincos(ang.x * kPI, scth.x, scth.y);

    half2 scphi = half2(sqrt(1.f - ang.y * ang.y), ang.y);
    return half3(scth.y * scphi.x, scth.x * scphi.x, scphi.y);
}

[numthreads(8, 4, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
    uint2 iuv = dtid.xy * _Scaling;
    
	float2 uv       = dtid.xy / float2(fWidth0, fHeight0) * _Scaling;
    float  LinDepth = Depth2Linear(1.f - _Depth[iuv]);
    float3 WorldPos = GetWorldPos(float2(uv.x * 2.f - 1.f, (1.f - uv.y) * 2.f - 1.f), LinDepth);
    float3 Normal = NormalDecode(_Normal[iuv]);
    
    const float3 EyePos = mInvView0._m03_m13_m23;       // The right one
    float cDepth = distance(EyePos, WorldPos);
    
    // 
    float r          = float(_OcclusionRadius / cDepth); // Radius
    float InvMaxDist = 1.f / (_OcclusionMaxDistance); // Give already inv value
    float attAngT    = .1f;
    
    // 
    float4 OccSH2 = 0.f;
    
    // Constants
    const float fl0 = 2.f;
    const float fl1 = 10.f;
    
    const float  wl0 = fl0 * .28209f; // .5 * sqrt(1. / pi)
    const float3 wl1 = fl1 * .48860f; // .5 * sqrt(3. / pi)
    
    const float4 sh2w = float4(wl1, wl0) / num_samples;
    
    // 
    float sn = 0.f, cs = 0.f;
    [unroll] for( int i = 0; i < num_samples; i++ ) {
        //sincos((i + iuv.x * iuv.y), sn, cs);
        float2 offset = (points[i].xy + float2(sn, cs)) * r; //reflect(points[i].xy, float2(sn, cs)).xy * r;
        
        uint2 liuv = clamp(iuv + offset, 0.f, uint2(fWidth0, fHeight0));
		float2 luv = saturate(uv + offset / float2(fWidth0, fHeight0));// * 200.f;
        float3 pos = GetWorldPos(float2(luv.x * 2.f - 1.f, (1.f - luv.y) * 2.f - 1.f), Depth2Linear(1.f - _Depth[liuv]));
        float3 c2s = pos - WorldPos;
        
        float dist = length(c2s);
        
        float3 c2sN = c2s / dist;
        
        float att = 1. - saturate(dist * InvMaxDist);
        float dp = dot(Normal, c2sN);
        
        att *= att * step(attAngT, dp);
        
        OccSH2 += att * sh2w * float4(c2sN, 1.);
    }

    _Accumulation[iuv] = OccSH2;
}
