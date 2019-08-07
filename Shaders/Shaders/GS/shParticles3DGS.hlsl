struct PS {
    float4 Position : SV_Position0;
};

[maxvertexcount(1)]
void main(point PS In[1], inout PointStream<PS> Stream) {
    PS Out = In[0];

    Stream.Append(Out);
    Stream.RestartStrip();
}
