struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float3 Texcoord : TEXCOORD0;
};

float4 main(PS In) : SV_Target0 {
    return float4(dot(In.Normal, normalize(float3(1.f, 1.f, 1.f))) * float3(.7f, .9f, 0.f) + float3(0.f, .1f, .2f), 1.f);
}
