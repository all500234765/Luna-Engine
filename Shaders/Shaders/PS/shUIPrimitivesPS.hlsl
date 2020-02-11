struct PS {
    float4 Position : SV_Position;
    float4 Color    : COLOR0;
};

float4 main(PS In) : SV_Target0 {
    return In.Color;
}