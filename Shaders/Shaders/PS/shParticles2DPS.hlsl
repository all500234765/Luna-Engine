struct PS {
    float4 Position : SV_Position;
    float3 Color    : COLOR0;
};

float4 main(PS In) : SV_Target0 {


    return float4(In.Color, 1.f);
}
