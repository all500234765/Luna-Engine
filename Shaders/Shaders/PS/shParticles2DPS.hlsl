struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
    float3 Color    : COLOR0;
};

float4 main(PS In) : SV_Target0 {


    return float4(normalize(In.Color), 1.f);
}
