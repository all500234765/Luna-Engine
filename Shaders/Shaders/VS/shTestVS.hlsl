cbuffer MatrixBuffer : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

struct VS {
    float3 Position : POSITION0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

struct PS {
    float4 Position : SV_Position;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

PS main(VS In/*, uint Instance : SV_InstanceID*/) {
    //In.Position.x += 2. * sin(fmod(Instance, 10) / 10.); //float(fmod(Instance, 10)) / 10. * 40.;
    //In.Position.z += 3. * sin(float(Instance)    / 10.); //floor(float(Instance) / 10.) * 5.;
    //In.Position.y += 4. * sin(Instance);
    
    PS Out;
        Out.Position = mul(mProj, mul(mView, mul(mWorld, float4(In.Position, 1.))));
        Out.Texcoord = In.Texcoord;
        Out.Normal   = mul(mWorld, float4(In.Normal, 0.)).xyz;
    return Out;
}
