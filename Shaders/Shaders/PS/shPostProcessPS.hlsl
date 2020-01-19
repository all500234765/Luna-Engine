Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0);

Texture2D _SSLRTexture    : register(t1);
SamplerState _SSLRSampler : register(s1);

Texture2D _ShadowTexture    : register(t2);
SamplerState _ShadowSampler : register(s2);

Texture2D _DeferredTexture    : register(t3);
SamplerState _DeferredSampler : register(s3);

cbuffer _FinalPass : register(b0) {
    // Eye adaptation / HDR
    float1 _MiddleGrey;
    float1 _LumWhiteSqr;

    // Bloom
    float1 _BloomScale;
    
    float1 _Alignment2;

    // DoF
    // _ProjValues.x = ;
    float2 _ProjValues; // _ProjValues.y / _ProjValues.x
    float2 _DoFFarValues;

    // Bokeh
    float1 _ColorScale;
    float1 _RadiusScale;
    float1 _BokehThreshold;

    uint1 _RenderFlags;
    //float1 _Alignment3;
};

StructuredBuffer<float> _AvgLum : register(t4);

Texture2D<float4> _BloomTexture     : register(t5);
Texture2D<float4> _BlurTexture      : register(t6);
Texture2D<float1> _DepthTexture     : register(t7);
Texture2D<float1> _AmbientOcclusion : register(t8);
Texture2D<float4> _OITransparencyCl : register(t9);
Texture2D<float4> _VolumetricLights : register(t10);

SamplerState _LinearSampler : register(s5);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

float Depth2Linear(float z) {
    return _ProjValues.x / (z + _ProjValues.y);
}

float3 DistDoF(float3 focus, float3 blur, float depth) {
    // Calculate CoC and lerp between colors based on CoC
    float CoC = saturate((depth - _DoFFarValues.x) * _DoFFarValues.y);
    return lerp(focus, blur, CoC);
}

float3 _toneExposure(float3 vColor, float average) {
    float T = -pow(average, -1.f);
    return 1. - exp(T * vColor);
}

float3 _toneReinhard(float3 vColor, float average, float exposure, float whitePoint) {
    // RGB -> XYZ conversion
    const float3x3 RGB2XYZ = { 0.5141364, 0.3238786,  0.16036376,
                               0.265068,  0.67023428, 0.06409157,
                               0.0241188, 0.1228178,  0.84442666 };
    float3 XYZ = mul(RGB2XYZ, vColor.rgb);
    
    // XYZ -> Yxy conversion
    float3 Yxy;
    Yxy.r = XYZ.g;                           // copy luminance Y
    Yxy.g = XYZ.r / (XYZ.r + XYZ.g + XYZ.b); // x = X / (X + Y + Z)
    Yxy.b = XYZ.g / (XYZ.r + XYZ.g + XYZ.b); // y = Y / (X + Y + Z)
    
    // (Lp) Map average luminance to the middlegrey zone by scaling pixel luminance
    float Lp = Yxy.r * exposure / average;
    
    // (Ld) Scale all luminance within a displayable range of 0 to 1
    Yxy.r = (Lp * (1.0f + Lp / (whitePoint * whitePoint))) / (1.0f + Lp);
    
    // Yxy -> XYZ conversion
    XYZ.r = Yxy.r * Yxy.g / Yxy.b;               // X = Y * x / y
    XYZ.g = Yxy.r;                                // copy luminance Y
    XYZ.b = Yxy.r * (1 - Yxy.g - Yxy.b) / Yxy.b;  // Z = Y * (1-x-y) / y
    
    // XYZ -> RGB conversion
    const float3x3 XYZ2RGB = { 2.5651, -1.1665, -0.3986,
                              -1.0217,  1.9777,  0.0439,
                               0.0753, -0.2543,  1.1892};
    
    return mul(XYZ2RGB, XYZ);
}

static const float3 _LumFactor = { .299f, .587f, .114f };
float3 EyeAdaptationNtoneMapping(float3 HDR) {
    //return _toneReinhard(HDR, _AvgLum[0], _MiddleGrey, _LumWhiteSqr);
    //return HDR;
    
    float3 LScale = dot(HDR, _LumFactor);

    LScale *= _MiddleGrey / _AvgLum[0];
    LScale = (LScale + LScale * LScale / _LumWhiteSqr) / (1.f + LScale);

    // Apply lum scale
    HDR *= LScale;
    
    // Reinhard & Gamma
    return HDR; //pow(HDR / (1.f + HDR), 2.2f);
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

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_GREEN_AS_LUMA 1
//#define FXAA_QUALITY__PRESET 12
//#define FXAA_QUALITY__PRESET 25
#define FXAA_QUALITY__PRESET 39
//#include "fxaa.hlsli"

static const float fxaaSubpix = 0.75;
static const float fxaaEdgeThreshold = 0.166;
static const float fxaaEdgeThresholdMin = 0.0833;

static const float threshold = .9; // .9

#define fsqrt(x) sqrt(dot(x, x)) //dot(length(x), length(x))

half4 main(PS In): SV_Target0 {
    //float2 fxaaFrame;
    //_Texture.GetDimensions(fxaaFrame.x, fxaaFrame.y);

    // 
    half4 Diff = 1.f;
    [flatten] if( _RenderFlags & 64 ) {
        Diff = _Texture.Sample(_Sampler, In.Texcoord);
    }
    
    //Diff.rgb = pow(Diff.rgb, 2.2f);
    [flatten] if( Diff.a < .5f ) { discard; }

    // Screen-Space Snow here
    //half3 Normal = NormalDecode(_NormalTexture.Sample(_NormalSampler, In.Texcoord));

    // FXAA
    //FxaaTex tex = {_Sampler, _Texture};
    //Diff = FxaaPixelShader(In.Texcoord, 0, tex, tex, tex, 1.f / fxaaFrame, 0, 0, 0, 
    //                       fxaaSubpix, fxaaEdgeThreshold, fxaaEdgeThresholdMin, 0, 0, 0, 0);

    //half3 Shadows  = _ShadowTexture.Sample(_ShadowSampler, In.Texcoord);
    //half3 Deferred = _DeferredTexture.Sample(_DeferredSampler, In.Texcoord);

    //Diff.rgb *= Shadows;

    // Chromatic Abberation
    //float hdrLeft = _Texture.Sample(_Sampler, In.Texcoord, int2(-1, -1)).b;
    //float hdrRight = _Texture.Sample(_Sampler, In.Texcoord, int2(+1, +1)).r;
    //Diff.rgb = half3(hdrRight, Diff.g, hdrLeft);

    // Tonemapping
    //Diff.rgb = _toneReinhard(Diff.rgb, 1.f, 1.f, 1.f);
    //Diff.rgb *= (1.f / (Diff.rgb + 1.f)) * 2.f; // 1.5f;
    
    // Ambient Occlusion
    /*float AO = 1.f;
    [flatten] if( _RenderFlags & 2 ) {
        AO = _AmbientOcclusion.Sample(_LinearSampler, In.Texcoord);
        Diff.rgb *= AO;
    }*/
    
    // Depth of Field
    [flatten] if( _RenderFlags & 8 ) {
        float depth = 1.f - _DepthTexture.Sample(_LinearSampler, In.Texcoord);

        // We don't wanna process far plane pixels
        [flatten] if( depth > 0.f ) {
            // Convert depth to linear space
            depth = Depth2Linear(depth);

            // Get blurred color
            float3 Blur = _BlurTexture.Sample(_LinearSampler, In.Texcoord).rgb;

            // Compute final color
            Diff.rgb = DistDoF(Diff.rgb, Blur, depth);
        }
    }

    // Volumetric
    Diff.rgb += pow(_VolumetricLights.Sample(_LinearSampler, In.Texcoord).rgb, 2.2f);
    
    // Order Independent Transparency
    //float4 OIT = _OITransparencyCl.Sample(_Sampler, In.Texcoord);
    //Diff.rgb += OIT.rgb;// * OIT.a;
    
    // Deferred lightning
    [flatten] if( _RenderFlags & 128 ) {
        //Diff.rgb = Deferred;
    }
    
    // Bloom
    [flatten] if( _RenderFlags & 16 ) {
        Diff.rgb += _BloomScale * _BloomTexture.Sample(_LinearSampler, In.Texcoord).rgb;
    }
    
    // Eye Adaptation And Tonemapping
    [flatten] if( _RenderFlags & 4 ) {
        Diff.rgb = EyeAdaptationNtoneMapping(Diff.rgb);
    }

    // 
    return Diff;
}
