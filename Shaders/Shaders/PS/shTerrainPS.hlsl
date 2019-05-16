struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

static const float3 _Color0 = float3(248.f, 212.f, 186.f) / 255.f; // Light skin
static const float3 _Color1 = float3( 72.f,  46.f,  47.f) / 255.f; // Dark brown

float4 main(PS In) : SV_Target0 {
    float3 N = normalize(In.Normal);

    float l = saturate(max(.3, dot(normalize(float3(0.f, 25.f, 0.f)), N)));
    return float4(lerp(_Color1, _Color0, l), 1.f);
}
