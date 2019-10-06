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

StructuredBuffer<float4> _DepthNDS : register(t0);
RWTexture2D<float> _AO : register(u0);

groupshared float _SharedDepth[1024];

static const float NumSamplesRcp = 1.0 / 8.0;
static const uint NumSamples = 8;
static const float2 SampleOffsets[NumSamples] = {
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

// c = Center
float ComputeAO(int2 cPixel, float2 cClip) {
    // Get depth
    float cDepth = GetDepth(cPixel);
    float IsNSky = (cDepth);

    [flatten] if( IsNSky == 0.f ) return 0.f;

    // Center in view space in pixels
    float3 cPos = float3(cClip * _ProjValues.xy * cDepth, cDepth);

    // Get view space normal
    float3 cNormal = GetNormal(cPixel);

    // Prepare random sampling
    float rAng = 0.f;
    float2 rSinCos; sincos(rAng, rSinCos.x, rSinCos.y);
    float2x2 mRot = float2x2(rSinCos.y, -rSinCos.x, 
                             rSinCos.x, +rSinCos.y);

    float ao = 0.f;
    [unroll(NumSamples)]
    for( uint i = 0; i < NumSamples; i++ ) {
        // Get offset and sample depth
        float2 sOffset = _OffsetRad * mul(mRot, SampleOffsets[i]);
        float curDepth = GetDepth(cPixel + sOffset * float2(1.f, -1.f));

        // Calc view space pos
        float3 curPos = float3((cClip + 2.f * sOffset * _ResRcp) * _ProjValues.xy * curDepth, curDepth);

        float3 c2CurP = curPos - cPos;
        float LenC2CP = length(c2CurP);
        float aFactor = 1.f - dot(c2CurP / LenC2CP, cNormal);
        float distFac = LenC2CP / _Radius;

        ao += saturate(max(aFactor, distFac));
    }

    return ao * NumSamplesRcp;
}

[numthreads(1024, 1, 1)]
void main(uint3 groupThreadId : SV_GroupThreadID, 
          uint3 dispatchThreadId : SV_DispatchThreadID)
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

		_AO[CurPixel] = pow(ComputeAO(CurPixel, centerClipPos), _SSAOPower);
	}
}
