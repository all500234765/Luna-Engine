struct VS {
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

struct PS {
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

PS main(VS In/*, uint Instance : SV_InstanceID*/) {
    /*In.Position.x += 2. * sin(fmod(Instance, 10) / 10.); //float(fmod(Instance, 10)) / 10. * 40.;
    In.Position.z += 3. * sin(float(Instance)    / 10.); //floor(float(Instance) / 10.) * 5.;
    In.Position.y += 4. * sin(Instance);*/
    
    PS Out;
        Out.Position = In.Position;
        Out.Texcoord = In.Texcoord;
        Out.Normal   = In.Normal;
    return Out;
}
