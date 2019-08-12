struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

float4 main(PS In) : SV_Target0 {


    return float4(normalize(In.Color.rgb), 1.f - In.Color.a / .2f);
}
