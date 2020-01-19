cbuffer LightCameraBuffer : register(b0) {
    #include "Transform.h"
}

cbuffer LightMatrixBuffer : register(b1) {
    #include "Camera.h"
};

cbuffer PlayerMatrixBuffer : register(b2) {
    #include "Camera.h"
};

cbuffer WorldLight : register(b3) {
    #include "WorldLight.h"
};

cbuffer Settings : register(b4) {
    //                              fQ = fFar / (fNear - fFar);
    float4 _ProjValues;  // fNear * fQ, fQ, 1 / m[0][0], 1 / m[1][1] // Player
    float4 _ProjValues2; //                                          // Light
    float2 _Scaling;     // Width, Height / Downscaling Factor
    float _GScattering;  // [-1; 1]
    float _MaxDistance;  // 0 - Light Far?
    uint _FrameIndex;    // 0 -> Interleaved; based on frame index
    uint _Interleaved;   // pow(2, n)
    float _Exposure;     // Default: 10.f
    uint _Padding;
};

Texture2D<float> _LightDepth : register(t0);
Texture2D<float> _Depth      : register(t1);

RWTexture2D<float3> _Accumulation : register(u0);

float3 GetWorldPos0(float2 ClipSpace, float z) {
	return mul(mInvView0, float4(ClipSpace * _ProjValues2.zw * z, z, 1.)).xyz;
}

float3 GetWorldPos1(float2 ClipSpace, float z) {
    return mul(mInvView1, float4(ClipSpace * _ProjValues.zw * z, z, 1.)).xyz;
}

float Depth2Linear(float z) {
    return _ProjValues.x / (z + _ProjValues.y);
}

float Depth2Linear2(float z) {
    return _ProjValues2.x / (z + _ProjValues2.y);
}

#define PI 3.1415926536f
float Scattering(float LdotV) {
	float g = _GScattering; //.3f;
    float a = g * g; //_GScattering * _GScattering;
    float r = 1.f - a;
        r /= 4.f * PI * pow(1.f + a - 2.f * g * LdotV, 1.5f);
    return r;
}

#define STEPS (16*4)
#define DIVS (1)

static const float DitherPattern[4][4] = { { .0f   , .5f   , .125f , .625f },
                                           { .75f  , .22f  , .875f , .375f },
                                           { .1875f, .6875f, .0625f, .5625f},
                                           { .9375f, .4375f, .8125f, .3125f} };

[numthreads(8, 4, 1+0*STEPS / DIVS)]
void main(uint3 dtid : SV_DispatchThreadID) {
    //uint StepID = dtid.z * DIVS;
    
    // Interleaved rendering
    [flatten] if( _Interleaved > 1u )
        [flatten] if( (fmod(dtid.x + dtid.y, _Interleaved) == _FrameIndex) ) { return; }
    
    // Player camera position
    const float3 Camera = mInvView1._m03_m13_m23;       // The right one
	//const float3 Direction = mInvView0._m02_m12_m22;  // The right one
    
    // 
    uint2 iuv        = dtid.xy * _Scaling; // In texture resolution
	float2 uv        = dtid.xy / float2(fWidth1, fHeight1) * _Scaling;
    float  LinDepth  = Depth2Linear(1.f - _Depth[iuv]);
    float3 WorldPos1 = GetWorldPos1(float2(uv.x * 2.f - 1.f, (1.f - uv.y) * 2.f - 1.f), LinDepth);
	
    // 
    float3 Start = Camera;
	float3 Vec   = WorldPos1 - Start;
    float rLen   = length(Vec); //clamp(length(Vec), 1.f, 130000.f);
    float3 rDir  = Vec / rLen;
    float StepL  = rLen / STEPS;
    
    // Ray marching settings
    float3 Step    = rDir * StepL;
    float3 Current = Start;
    float3 Accum   = 0.f; // Final accumulation
    
    // Ray march
    [unroll(STEPS)]
    for( uint i = 1; i < STEPS; i++ ) {
        float4 LightSpace = mul(mProj0, mul(mView0, float4(Current, 1.f)));
        LightSpace /= LightSpace.w;
        
        // Apply bias
        //LightSpace.z -= 1.f / 2048.f;
        //LightSpace.z -= 1.f / 65536.f;
		
        Current += Step;
        //Current.xz += DitherPattern[(i / 4u) % 4u][i % 4u];
		
		float2 proj_uv = float2(+LightSpace.x * .5f + .5f, 
							    -LightSpace.y * .5f + .5f);// + (DitherPattern[(i / 4u) % 4u][i % 4u] * Texel);

        [flatten] if( saturate(proj_uv.x) != proj_uv.x || saturate(proj_uv.y) != proj_uv.y ) { continue; }
        
		float Shadow = _LightDepth[proj_uv * float2(fWidth0, fHeight0)];
        float delta  = Shadow - LightSpace.z;
        if( delta < 0.f ) {
			Accum += _Exposure * Scattering(dot(rDir, -_WorldLightDirection)) * _WorldLightColor;
		}
    }
    
    _Accumulation[dtid.xy] = pow(Accum / STEPS, 1.f / 2.2f);
}
