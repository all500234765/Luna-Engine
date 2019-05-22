TextureCube _CubemapTexture  : register(t0);
SamplerState _CubemapSampler : register(s0);

struct PS {
    float4 Position : SV_Position;
    float3 InputPos : TEXCOORD0;
};

float4 main(PS In) : SV_Target0 {
    return _CubemapTexture.Sample(_CubemapSampler, normalize(In.InputPos));
}
