struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0);

float4 main(PS In): SV_Target0 {
    float4 s = _Texture.Sample(_Sampler, In.Texcoord);
    [branch] if( s.a < .5 ) { discard; } // Alpha test

    return s;
}
