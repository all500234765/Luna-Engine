struct PS {
    float4 Position : SV_Position;

};

PS main() {
    PS Out;
        Out.Position = float4(0.f, 0.f, 0.f, 1.f);
    return Out;
}
