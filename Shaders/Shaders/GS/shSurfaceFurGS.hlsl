struct GS {
    float4   Position : SV_Position;
    float3x3 WorldTBN : TEXCOORD0;
    float2   Texcoord : TEXCOORD3;
    float3   WorldPos : TEXCOORD4;
    float4   LightPos : TEXCOORD5;
    float3   InputPos : TEXCOORD6;
    float3   ViewDir  : TEXCOORD7;
};

struct PS {
    float4   Position : SV_Position;
    float3x3 WorldTBN : TEXCOORD0;
    float2   Texcoord : TEXCOORD3;
    float3   WorldPos : TEXCOORD4;
    float4   LightPos : TEXCOORD5;
    float3   InputPos : TEXCOORD6;
    float3   ViewDir  : TEXCOORD7;
};

void NewShell(inout LineStream<GS> Stream, GS In, float step) {
    PS Shell = In;
        Shell.Position = float4(In.Position.xyz + In.WorldTBN._m20_m21_m22 * step, In.Position.w);
    Stream.Append(Shell);
}

[maxvertexcount(2)]
void main(point GS In[1], inout LineStream<GS> Stream) {
    int i = 0;
    //for( i = 0; i < 3; i++ ) 
    {
        NewShell(Stream, In[i], 0);
        NewShell(Stream, In[i], 1);

        Stream.RestartStrip();
    }

    //Stream.RestartStrip();
}
