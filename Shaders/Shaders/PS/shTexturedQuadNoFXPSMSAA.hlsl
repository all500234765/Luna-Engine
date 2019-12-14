static const float2 MSAAOffsets8[8] =
{
    float2( .0625f, -.1875f), float2(-.0625f,  .1875f),
    float2( .3125f,  .0625f), float2(-.1875f, -.3125f),
    float2(-.3125f,  .3125f), float2(-.4375f, -.0625f),
    float2( .1875f,  .4375f), float2( .4375f, -.4375f)
};

struct PS {
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

Texture2D _Texture    : register(t0);
SamplerState _Sampler : register(s0);

float4 main(PS In, out uint coverage : SV_Coverage) : SV_Target0 {
    coverage = 0;

    //[branch]
    //if( AlphaTest.x <= 1 / 255.0 )
    //    coverage = 255;
    //else
    {
        float2 tc_ddx = ddx(In.Texcoord);
        float2 tc_ddy = ddy(In.Texcoord);

        [unroll]
        for( int i = 0; i < 8; i++ )
        {
            float2 texelOffset = MSAAOffsets8[i].x * tc_ddx + MSAAOffsets8[i].y * tc_ddy;
            float temp = _Texture.Sample(_Sampler, In.Texcoord + texelOffset).a;

            if( temp >= .5f ) coverage |= 1 << i;
        }
    }

    float4 s = _Texture.Sample(_Sampler, In.Texcoord);
    return s;
}
