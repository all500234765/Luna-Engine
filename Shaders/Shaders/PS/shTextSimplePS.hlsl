#include "shTextInclude.hlsli"

float4 main(PS In) : SV_Target0 {
    return _FontAtlas.Sample(_FontSampler, In.Texcoord);
}
