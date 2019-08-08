Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0);

struct PS {
    float4 Position : SV_Position0;
    float2 Texcoord : TEXCOORD0;
    float3 Velocity : TEXCOORD1;
};

float4 main(PS In) : SV_Target0 {
    float l = length(In.Velocity) * 1.f;
    //if( l <= 0.f ) { discard; }

    return _Texture.Sample(_Sampler, float2(1.f - saturate(l), 0.f)) * .3f;
}
