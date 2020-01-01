Texture2D _DirectLightTex  : register(t0);
Texture2D _AmbientLightTex : register(t1);
Texture2D _SSAOTex         : register(t2);

SamplerState _Sampler : register(s0);


struct PS {
    float4 Position  : SV_Position;
    float2 Texcoord  : TEXCOORD0;
    float4 CameraPos : TEXCOORD1;
};

struct LBuffer {
    float4 Light : SV_Target0;
};

LBuffer main(PS In) {
    float3 Light = 0.f;
    
    // 
    float3 Ambient = _AmbientLightTex.Sample(_Sampler, In.Texcoord).rgb;
    float1 SSAO    = _SSAOTex.Sample(_Sampler, In.Texcoord).r;
    float3 Direct  = _DirectLightTex.Sample(_Sampler, In.Texcoord).rgb;
    
    // Light formula
    Light = (Ambient + Direct);
    
    //Light /= 1.f + Light;
    //Light = pow(Light, 1.f / 2.2f);
    
    // 
    LBuffer Out;
        Out.Light = float4(Light, 1.f);
    return Out;
}
