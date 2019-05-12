struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

float4 main(PS In): SV_Target0 {
    return float4(In.Texcoord, 0., .25);
}
