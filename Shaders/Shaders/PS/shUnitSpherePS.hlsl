struct PS {
    float4 Position : SV_Position0;
    float3 Normal   : NORMAL0;

};

#define PI 3.1415
#define TAU (2. * PI)

half4 main(PS In) : SV_Target0 {
    float3 d = In.Normal;
    //float2 uv = float2(.5 + atan2(d.z, d.x) / TAU, .5 - asin(d.y) / PI);

    float3 q = floor(d);

    return half4((fmod(q.x + q.y + q.z, 2.) * .5 + .5).xxx, 1.);
};
