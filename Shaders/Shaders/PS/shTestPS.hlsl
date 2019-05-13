struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

float4 main(PS In) : SV_Target0 {
    float l = max(.3, dot(normalize(float3(200., 50., 200.)), In.Normal));
    return float4(.7, .9, 0., 1.) * float4(l.xxx, 1.);
}
