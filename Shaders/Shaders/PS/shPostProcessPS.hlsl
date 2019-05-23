Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0);

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};


float3 _toneExposure(float3 vColor, float average) {
    float T = -pow(average, -1) * vColor;
    return 1. - exp(T);
}

float3 _toneReinhard(float3 vColor, float average, float exposure, float whitePoint) {
    // RGB -> XYZ conversion
    const float3x3 RGB2XYZ = {0.5141364, 0.3238786,  0.16036376,
                               0.265068,  0.67023428, 0.06409157,
                               0.0241188, 0.1228178,  0.84442666};
    float3 XYZ = mul(RGB2XYZ, vColor.rgb);
    
    // XYZ -> Yxy conversion
    float3 Yxy;
    Yxy.r = XYZ.g;                           // copy luminance Y
    Yxy.g = XYZ.r / (XYZ.r + XYZ.g + XYZ.b); // x = X / (X + Y + Z)
    Yxy.b = XYZ.g / (XYZ.r + XYZ.g + XYZ.b); // y = Y / (X + Y + Z)
    
    // (Lp) Map average luminance to the middlegrey zone by scaling pixel luminance
    float Lp = Yxy.r * exposure / average;
    
    // (Ld) Scale all luminance within a displayable range of 0 to 1
    Yxy.r = (Lp * (1.0f + Lp / (whitePoint * whitePoint))) / (1.0f + Lp);
    
    // Yxy -> XYZ conversion
    XYZ.r = Yxy.r * Yxy.g / Yxy.b;               // X = Y * x / y
    XYZ.g = Yxy.r;                                // copy luminance Y
    XYZ.b = Yxy.r * (1 - Yxy.g - Yxy.b) / Yxy.b;  // Z = Y * (1-x-y) / y
    
    // XYZ -> RGB conversion
    const float3x3 XYZ2RGB = { 2.5651, -1.1665, -0.3986,
                              -1.0217,  1.9777,  0.0439,
                               0.0753, -0.2543,  1.1892};
    
    return mul(XYZ2RGB, XYZ);
}

float4 main(PS In): SV_Target0 {
    // 
    float4 Diff = _Texture.Sample(_Sampler, In.Texcoord);

    // Tonemapping
    Diff.rgb = _toneReinhard(Diff.rgb, 1.f, 1.f, 1.f);

    // 
    return Diff;
}
