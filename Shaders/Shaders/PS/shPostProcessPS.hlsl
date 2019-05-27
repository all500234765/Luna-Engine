Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0);

Texture2D _SSLRTexture    : register(t1);
SamplerState _SSLRSampler : register(s1);

Texture2D<half2> _NormalTexture : register(t2);
SamplerState _NormalSampler     : register(s2);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

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
#include "fxaa.hlsli"

static const float fxaaSubpix = 0.75;
static const float fxaaEdgeThreshold = 0.166;
static const float fxaaEdgeThresholdMin = 0.0833;

static const float threshold = .9; // .9

#define fsqrt(x) sqrt(dot(x, x)) //dot(length(x), length(x))

half4 main(PS In): SV_Target0 {
    // Sobel operator
    const int2 texAddrOffsets[8] = {
            int2(-1, -1),
            int2(0, -1),
            int2(1, -1),
            int2(-1,  0),
            int2(1,  0),
            int2(-1,  1),
            int2(0,  1),
            int2(1,  1),
    };
    
    float lum[8];
    int i;
    float3 LuminanceConv = float3(.2125f, .7154f, .0721f);
    float2 fxaaFrame;
    _Texture.GetDimensions(fxaaFrame.x, fxaaFrame.y);
    float D;

    for( i = 0; i < 8; i++ ) {
        float3 Color = _Texture.Load(int3(int2(In.Texcoord * fxaaFrame) + texAddrOffsets[i], 0));
        lum[i] = (dot(Color, LuminanceConv)) * threshold;
    }

    float x = lum[0] + 2 * lum[3] + lum[5] - lum[2] - 2 * lum[4] - lum[7];
    float y = lum[0] + 2 * lum[1] + lum[2] - lum[5] - 2 * lum[6] - lum[7];

    D = 1.; //(1. - Color2Depth(_Depth.Sample(_DepthSamp, In.Texcoord))) / 1.;
    float edge = D * sqrt(x * x + y * y);

    // 
    half4 Diff = _Texture.Sample(_Sampler, In.Texcoord);

    if( Diff.a < .5f ) { discard; }

    // Add snow
    half3 Normal = NormalDecode(_NormalTexture.Sample(_NormalSampler, In.Texcoord));


    // Chromatic Abberation
    //float hdrLeft  = _Texture.Sample(_Sampler, In.Texcoord, int2(-1, -1)).b;
    //float hdrRight = _Texture.Sample(_Sampler, In.Texcoord, int2(+1, +1)).r;
    //Diff.rgb = half3(hdrRight, Diff.g, hdrLeft);

    // FXAA
    FxaaTex tex = {_Sampler, _Texture};
    Diff = FxaaPixelShader(In.Texcoord, 0, tex, tex, tex, 1 / fxaaFrame, 0, 0, 0, 
                           fxaaSubpix, fxaaEdgeThreshold, fxaaEdgeThresholdMin, 0, 0, 0, 0);

    // Tonemapping
    //Diff.rgb = _toneReinhard(Diff.rgb, 1.f, 1.f, 1.f);
    Diff.rgb = (1. - edge) * (Diff.rgb / (Diff.rgb + 1.f)) * 2.f;

    // 
    return Diff;
}
