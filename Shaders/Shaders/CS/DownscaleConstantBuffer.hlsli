cbuffer _DownScale : register(b0) {
    // Res of downscaled target: x - width, y - height
    uint2 _Res; // Backbuffer / 4

    // Total pixels in the downscaled img
    uint _Domain; // Res.x * Res.y

    // Number of groups dispatched on 1st pass
    uint _GroupSize; // Domain / 1024

    // Adaptation factor
    float _Adaptation;

    // Bloom threshold
    float _BloomThreshold;

    // Padding
    float2 _Alignment;
};

static const float4 _LumFactor = { .299f, .587f, .114f, 0.f };
