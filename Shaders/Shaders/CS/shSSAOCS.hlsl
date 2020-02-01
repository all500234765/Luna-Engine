cbuffer Downscaling : register(b0) {
    uint2  _Res;        // Resolution
    float2 _ResRcp;     // 1.f / _Res
    float4 _ProjValues; // 
    float _OffsetRad;   // Radius for random points
    float _Radius;      // Sphere radius
    float _SSAOPower;   // Power of SSAO
    float _NoiseSize;   // Empty
    float4x4 _mView;    // View matrix
};

StructuredBuffer<float4> _DepthNDS  : register(t0);
Texture2D<float2> _BlueNoiseTexture : register(t1);
Texture2D<float3> _AlbedoTexture    : register(t2);

RWTexture2D<float4> _AO             : register(u0);

groupshared float _SharedDepth[1024];

#define NumSamples 8
static const float NumSamplesRcp = 1.f / float(NumSamples);
static const float2 SampleOffsets[8] = {
	float2(0.2803166, 0.08997212),
	float2(-0.5130632, 0.6877457),
	float2(0.425495, 0.8665376),
	float2(0.8732584, 0.3858971),
	float2(0.0498111, -0.6287371),
	float2(-0.9674183, 0.1236534),
	float2(-0.3788098, -0.09177673),
	float2(0.6985874, -0.5610316),
};

float GetDepth(int2 p) {
    int2 p2 = clamp(p, 0, _Res - 1);
    return _DepthNDS[p2.x + p2.y * _Res.x].x;
}

float3 GetNormal(int2 p) {
    int2 p2 = clamp(p, 0, _Res - 1);
    return _DepthNDS[p2.x + p2.y * _Res.x].yzw;
}

float3 GetAlbedo(int2 p) {
    int2 p2 = clamp(p, 0, _Res - 1) * 2;
    return _AlbedoTexture[p2];
}

float min2c(float a, float b) {
	[flatten] if( a < b ) return a;
	return 0.f;
}

// c = Center
float4 ComputeAO(int2 cPixel, float2 cClip) {
    // Get depth
    float cDepth = GetDepth(cPixel);
    float IsNSky = (cDepth);

    [flatten] if( IsNSky == 0.f ) return 0.f;

    // Center in view space in pixels
    float3 cPos = float3(cClip * _ProjValues.xy * cDepth, cDepth);

    // Get view space normal
    float3 cNormal = GetNormal(cPixel);
    
    // Get Albedo
    float3 cAlbedo = GetAlbedo(cPixel);

    // Prepare random sampling
    float rAng = dot(cClip, float2(73.f, 197.f));
    float2 rSinCos; sincos(rAng, rSinCos.x, rSinCos.y);
    float2x2 mRot = float2x2(rSinCos.y, -rSinCos.x, 
                             rSinCos.x, +rSinCos.y);

    float ao = 0.f;
    float3 color = cAlbedo;
    [unroll(NumSamples)]
    for( uint i = 0; i < NumSamples; i++ ) {
        // Get offset and sample depth
        float2 sOffset = _OffsetRad * _BlueNoiseTexture[cPixel % _NoiseSize]; //mul(mRot, SampleOffsets[i]);
        int2 offset = cPixel + sOffset * float2(1.f, -1.f);
        float curDepth = GetDepth(offset);

        // Calc view space pos
        float3 curPos = float3((cClip + 2.f * sOffset * _ResRcp) * _ProjValues.xy * curDepth, curDepth);

        float3 c2CurP = curPos - cPos;
        float LenC2CP = length(c2CurP);
		float3 Normal = c2CurP / LenC2CP;
        float aFactor = 1.f - dot(Normal, cNormal);
        float distFac = LenC2CP / _Radius;
        
        color += GetAlbedo(offset) * saturate(dot(Normal, cNormal)) * min2c(distFac, .65f);
        ao += saturate(max(aFactor, distFac));
    }

    return float4(color, ao) * NumSamplesRcp; // * ao * NumSamplesRcp * NumSamplesRcp;
}

[numthreads(1024, 1, 1)]
void main(uint3 groupThreadId : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint2 CurPixel = uint2(dispatchThreadId.x % _Res.x, dispatchThreadId.x / _Res.x);

	_SharedDepth[groupThreadId.x] = _DepthNDS[dispatchThreadId.x].x;

    // Sync threads
	GroupMemoryBarrierWithGroupSync();

	// Skip out of bound pixels
	if( CurPixel.y < _Res.y ) {
		// Find the XY clip space position for the current pixel
		// Y has to be inverted
		float2 centerClipPos = 2.f * float2(CurPixel) * _ResRcp;
		centerClipPos = float2(centerClipPos.x - 1.f, 1.f - centerClipPos.y);

		float4 CAO = ComputeAO(CurPixel, centerClipPos);
		CAO.a = pow(CAO.a, _SSAOPower);
		_AO[CurPixel].rgba = float4(lerp(CAO.rgb, 0.f, CAO.a), CAO.a);
	}
}
