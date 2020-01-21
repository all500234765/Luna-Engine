#include "ShaderDefinesLocal.h"

mfloat4x4 _mInvView;
mfloat4x4 _mInvProj;

float2 _TanAspect; // dtan(fov * .5) * aspect, - dtan(fov / 2)
float2 _Texel;     // 1 / target width, 1 / target height
float _Far;        // Far clipping plane
uint _LightCount;
float2 _Padding;

float4 _ProjValues;
