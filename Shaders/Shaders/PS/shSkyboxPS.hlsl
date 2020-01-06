TextureCube<float3> _CubemapTexture : register(t10);
SamplerState        _CubemapSampler : register(s10);

struct PS {
    float4 Position : SV_Position;
    float3 InputPos : TEXCOORD0;
};

float4 main(PS In) : SV_Target0 {
    return float4(_CubemapTexture.SampleLevel(_CubemapSampler, normalize(In.InputPos), 0), 1.f);
}
