cbuffer cbMaterial : register(b0) {
    #include "Material.h"
};

cbuffer cbAmbientLight : register(b1) {
    #include "AmbientLight.h"
};

cbuffer cbBasicFog : register(b2) {
    #include "BasicFog.h"    
}

#include "MaterialTextures.h"

Texture2D<float1> _DepthTexture       : register(t6);
SamplerComparisonState  _DepthSampler : register(s6) {
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = GREATER;
};

Texture2D<float2> _NoiseTexture : register(t7);
SamplerState _NoiseSampler      : register(s7);

TextureCube<float3> _CubemapTexture : register(t8);
SamplerState        _CubemapSampler : register(s8);

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

static const float3 _Color0 = float3(248.f, 212.f, 186.f) / 255.f; // Light skin
static const float3 _Color1 = float3( 72.f,  46.f,  47.f) / 255.f; // Dark brown

float4 main(PS In) : SV_Target0 {
    float3 N = normalize(In.Normal);

    float l = saturate(max(.3, dot(normalize(float3(0.f, 25.f, 0.f)), N)));
    return float4(0.f + _AlbedoTex.Sample(_AlbedoSampl, In.Texcoord).rgb * l, 1.f);
    //return float4(lerp(_Color1, _Color0, l), 1.f);
}
