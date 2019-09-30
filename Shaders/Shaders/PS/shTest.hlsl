float4 main(uint b : SV_PrimitiveID, 
            uint c : SV_Coverage, 
            uint d : SV_SampleIndex) : SV_Target0 {
    float a = (c + b) * d;
    float e = c + b * d;
    float f = c * b + d;
    float g = mad(c, b, d);

    return float4(a, e, f, g);
}
