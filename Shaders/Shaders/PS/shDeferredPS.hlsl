cbuffer bGlobal : register(b0) {
    float2 _TanAspect; // dtan(fov * .5) * aspect, - dtan(fov / 2)
    float2 _Texel;     // 1 / texture width, 1 / texture height
    float1 _Far;
    float3 PADDING0;
    float4x4 _mInvView;
}

cbuffer bLightData : register(b1) {
    float3 _LightDiffuse;
    float1 PADDING1;
    float2 _LightData; // Empty, Intensity
    float2 PADDING2;
};

Texture2D<half2> _NormalTexture : register(t0);
SamplerState _NormalSampler     : register(s0);

Texture2D _DepthTexture    : register(t1);
SamplerState _DepthSampler : register(s1);

struct PS {
    float4 Position : SV_Position;
    float4 LightPos : TEXCOORD0;
    float3 vLightPs : TEXCOORD1; // View
    float4 WorldPos : TEXCOORD2;
};

// https://aras-p.info/texts/CompactNormalStorage.html#method03spherical
// Spherical Coordinates
#define kPI 3.1415926536f
half3 NormalDecode(half2 enc) {
    half2 scth, ang = enc * 2.f - 1.f;
    sincos(ang.x * kPI, scth.x, scth.y);

    half2 scphi = half2(sqrt(1.f - ang.y * ang.y), ang.y);
    return half3(scth.y * scphi.x, scth.x * scphi.x, scphi.y);
}

#define fsqrt(x) /*sqrt(dot(x, x)) /*/dot(length(x), length(x))

half4 main(PS In): SV_Target0 {

    // Calculate screen space coordinates and world space vertex
    float2 ScreenPos = In.WorldPos.xy / In.WorldPos.w;
    float2 ScreenUV  = float2(ScreenPos.x, -ScreenPos.y) * .5 + .5 + _Texel * .5;

    // Sample depth
    float Depth = _DepthTexture.Sample(_DepthSampler, ScreenUV).r;

    // Calculate world vertex
    float3 tVertex = float3(Depth * _TanAspect * (ScreenUV * 2. - 1.), Depth);
    float4 Vertex = mul(_mInvView, float4(tVertex.xzy, 1.));
    Vertex.xyz /= Vertex.w;

    // Light direction
    float3 lDir = In.vLightPs.xyz - Vertex.xyz;

    // Light properties, but doubled
    float dist = length(lDir); // Faster than length
    float range = In.LightPos.w;// *In.LightPos.w;
    
    float3 Diffuse = 0.f, Specular = 0.f;
    float attenuation = 0.f;
    
    // Debug
    //printf("Dist: %f, Range: %f", dist, range);
    //return half4(dist.xxx / range, 1.);

    // Light calculation
    [branch] if( dist <= range ) {
        // Sample and convert normal
        half3 Normal = NormalDecode(_NormalTexture.Sample(_NormalSampler, ScreenUV));
        
        // Calculate lambert
        half3 L = lDir / dist;
        float lambert = dot(L, Normal);

        //[branch] if( lambert > 0. ) 
        {
            // Calculate attenuation
            float d = min(dist / range, 1.);
            attenuation = 1. - d;

            // Apply point light diffuse
            Diffuse = _LightDiffuse * attenuation * lambert * _LightData.y;

            // Calc specular
            float3 R = normalize(reflect(L, Normal));
            float3 V = normalize(-Vertex.xyz);

            float spec = pow(max(dot(R, V), 0.f), 32.f);

            // Apply point light specular
            Specular = _LightDiffuse * attenuation * spec * _LightData.y;
        }

        //return half4((1.).xxx, 1.f);
    }

    // 
    return half4(Specular + Diffuse, 1.f);
}

//return half4(Vertex, 1.); // dist / 100000;
