struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
    float4 Color    : COLOR0;
    uint   States   : TEXCOORD1;
};

Texture2D _Atlas : register(t0);
SamplerState _Sm : register(s0);

float4 main(PS In) : SV_Target0 {
    float4 Out = In.Color;
    [flatten] if( In.States & 0x1 ) Out *= _Atlas.Sample(_Sm, In.Texcoord);
    //[flatten] if( In.States & 0x1 ) Out = float4(In.Texcoord, 0.f, 1.f);
    
    [flatten] if( Out.a <= .01f ) discard;
    return Out;
}
