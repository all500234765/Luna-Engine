struct GS {
    float4 Position : SV_Position0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

struct PS {
    float4 Position : SV_Position0;
    float3 Normal   : NORMAL0;
    float2 Texcoord : TEXCOORD0;
};

void NewShell(inout LineStream<GS> TriStream, GS In, float step) {
    PS Shell = In;
        Shell.Position = float4(In.Position.xyz + In.Normal * step, In.Position.w);
    TriStream.Append(Shell);
}

[maxvertexcount(6)]
void main(triangle GS In[3], inout LineStream<GS> TriStream) {
    for( int i = 0; i < 3; i++ ) {
        NewShell(TriStream, In[i], 0);
        NewShell(TriStream, In[i], 1);

        TriStream.RestartStrip();
    }

    TriStream.RestartStrip();

    return;
    for( i = 0; i < 3; i++ ) {
        NewShell(TriStream, In[i], 0.);
        NewShell(TriStream, In[i], 8.);
        NewShell(TriStream, In[i], .01);
        //NewShell(TriStream, In[i], 1.);
        //NewShell(TriStream, In[i], 2.);
        //NewShell(TriStream, In[i], .1);

        TriStream.RestartStrip();
    }

    //TriStream.RestartStrip();
}
