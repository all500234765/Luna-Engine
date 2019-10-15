cbuffer _SSLRSettings : register(b0) {
    float4x4 _mProj;
    float1 _ViewAngleThreshold;
    float1 _EdgeDistThreshold;
    float1 _DepthBias;
    float1 _ReflScale;
    float4 _ProjValues;
    float1 _PixelSize; // 2.f / Height
    uint1  _NumSteps;  // Width
    float2 _Alignment;
}

struct PS {
    float4 Position : SV_Position0;
    float4 ViewPos  : TEXCOORD0;
    float3 ClipSPos : TEXCOORD1; // Clip space position
    float3 ViewNorm : NORMAL0;
};

SamplerState _PointSampler      : register(s0);
Texture2D<float4> _Diffuse      : register(t0);
Texture2D<half2> _NormalTexture : register(t1);
Texture2D<float> _DepthTexture  : register(t2);

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

float3 CalcViewPos(float2 xy, float z) {
	return float3(xy * _ProjValues.zw * z, z);
}

float4 main(PS In) : SV_Target0 {
    // Pixel pos and normal in view space
    float3 VPos  = In.ViewPos.xyz;
    float3 VNorm = normalize(In.ViewNorm);

    // Camera to pixel direction
    float3 Cam2pixel = normalize(VPos);

    // Reflected view direction
    float3 VReflect = reflect(Cam2pixel, VNorm);

    // Reflection color
    float4 ReflCol = 0.f;

    // Threshold
    if( VReflect.z > _ViewAngleThreshold ) {
        // Fade reflection
        float InvViewAngleT = 1.f - _ViewAngleThreshold;
        float ViewAngleFade = (VReflect.z - _ViewAngleThreshold) / InvViewAngleT;

        // View space to clip space
        float3 VPosRefl = VPos + VReflect;
        float3 CPosRefl = mul(_mProj, float4(VPosRefl, 1.f)).xyz / VPosRefl.z;
        float3 CReflect = CPosRefl - In.ClipSPos;

        // Resize SSR to an appropriate length
        float reflScale = _PixelSize / length(CReflect.xy); // dot(x, x) ?
        CReflect *= reflScale;

        // Calculate offsets
        float3 LastOffset = In.ClipSPos;
        float3 CurrOffset = In.ClipSPos + CReflect;
        CurrOffset.xy = CurrOffset.xy * float2(.5f, -.5f) + .5f;

        // 
        CReflect.xy *= float2(.5f, -.5f);

        // Iteration
        [loop] for( uint i = 0; i < _NumSteps; i++ ) {
            //float Z = _DepthTexture.SampleBias(_PointSampler, CurrOffset.xy, _DepthBias);
            
            float Z = 1.f - _DepthTexture.SampleLevel(_PointSampler, CurrOffset.xy, 0.);
			
			float depth = Depth2Linear(Z);
			float3 CP = CalcViewPos(In.ClipSPos.xy + CReflect.xy * ((float)i + 1.f), depth);
			
            // TODO: Probably inverse this from < to >
            if( Z > CurrOffset.z ) {
                // Correct offset based on depth
                CurrOffset.xy = (CurrOffset.z - Z) * CReflect.xy + LastOffset.xy;

                // Get Diffuse color
                ReflCol.rgb = _Diffuse.Sample(_PointSampler, CurrOffset.xy).rgb;

                // Edge fade
                float EdgeFade = saturate(distance(CurrOffset, .5f)) * 2.f - _EdgeDistThreshold;

                // Find fade value
                ReflCol.a = min(ViewAngleFade, 1.f - EdgeFade * EdgeFade) * _ReflScale;
                
                // Advance past the final iteration
                i = _NumSteps;
            }

            // Advance to next sample
            LastOffset = CurrOffset;
            CurrOffset += CReflect;
        }
    }

    return ReflCol;
}
