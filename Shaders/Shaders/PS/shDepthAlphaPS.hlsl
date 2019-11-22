struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

Texture2D _DiffuseTexture    : register(t0);
SamplerState _DiffuseSampler : register(s0);

void main(PS In) {
    [flatten] if( _DiffuseTexture.Sample(_DiffuseSampler, In.Texcoord).a < .1 ) { discard; }
}
