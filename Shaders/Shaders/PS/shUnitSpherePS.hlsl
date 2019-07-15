struct PS {
    float4 Position : SV_Position0;
    float3 Normal   : NORMAL0;

};

#define PI 3.1415
#define TAU (2. * PI)

static const float3 _Ambient = { .4, .6, .7 };
static const float3 _LightPos = { 0., 100., 0. };
static const float3 _LightDir = normalize(_LightPos);

half4 main(PS In) : SV_Target0 {
    float3 d = In.Normal;
    //float2 uv = float2(.5 + atan2(d.z, d.x) / TAU, .5 - asin(d.y) / PI);

    float3 q = floor(d);
    float3 col = (fmod(q.x + q.y + q.z, 2.) * .5 + .5).xxx;

    return half4(col * dot(d, _LightDir).xxx * .5 + .5, 1.);
    return half4(lerp(_Ambient, col, max(dot(d, _LightDir), .2)), 1.);
};
