cbuffer Matrices : register(b0) {
    float4x4 mLightProj[4];
    float4x4 mLightInvProj[4];
    float4x4 mLight2World;
    float4x4 mViewProj;
    float3 _EyePosition;
    float  _Bias;
};

Texture2DArray<float> _Depth : register(t0);
SamplerState _DepthSampler   : register(s0);

float SampleDepth(float2 uv, int Cascade) {
    return _Depth.Sample(_DepthSampler, float3(uv, asfloat(Cascade)));
}

struct HS_POINT {
    float4 ClipPos  : SV_Position;
    float4 WorldPos : TEXCOORD0;
};

struct PS {
    float4 Position : SV_Position;
    float4 WorldPos : TEXCOORD0;
};

[domain("quad")]
PS main(HS In, float2 uv : SV_DomainLocation, const OutputPatch<HS_POINT, 4> Patch) {
    PS Out;
    
    float3 ClipIn1 = lerp(Patch[0].ClipPos.xyz, Patch[1].ClipPos.xyz, uv.x);
    float3 ClipIn2 = lerp(Patch[3].ClipPos.xyz, Patch[2].ClipPos.xyz, uv.x);
    float3 ClipIn = lerp(ClipIn1, ClipIn2, uv.y);

    float4 Pos1 = lerp(Patch[0].WorldPos, Patch[1].WorldPos, uv.x);
    float4 Pos2 = lerp(Patch[3].WorldPos, Patch[2].WorldPos, uv.x);
    float4 WorldPos = lerp(Pos1, Pos2, uv.y);

    const float EDGE_FACTOR = .5f;
    if( all(abs(ClipIn.xy) < EDGE_FACTOR) ) {
        int Cascade = -1;
        float4 ClipPos = float4(0.f, 0.f, 0.f, 1.f);

        const int COARSE_CASCADE = 3;
        [unroll]
        for( int i = COARSE_CASCADE; i >= 0; i-- ) {
            // Try to refresh from finer cascade
            float4 ClipPosCascade = mul(mLightProj[i], WorldPos);
            ClipPosCascade *= rcp(ClipPosCascade.w);

            if( all(abs(ClipPosCascade.xy) < 1.f) ) {
                float2 Tex = ClipPosCascade.xy * float2(.5f, -.5f) + .5f;

                float depth = SampleDepth(Tex, i);
                if( depth > 0.f ) {
                    // Isn't far plane
                    ClipPos.xyz = float3(ClipPosCascade.xy, depth);
                    Cascade = i;
                }
            }
        }

        if( Cascade >= 0 ) {
            WorldPos = mul(mLightInvProj[Cascade], float4(ClipPos.xyz, 1.f));
            WorldPos *= rcp(WorldPos.w);

            // lerp(x, y, z) = x + z * (y - x)
            WorldPos.xyz = lerp(_EyePosition, WorldPos.xyz, 1.f - _Bias);
        }
    } else {
        WorldPos = mul(mLight2World, float4(ClipIn.xy, 1.f, 1.f));
        WorldPos *= rcp(WorldPos.w);
    }

    Out.WorldPos = WorldPos;
    Out.Position = mul(mViewProj, WorldPos);

    return Out;
}
