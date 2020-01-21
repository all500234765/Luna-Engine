#include "ShaderDefines.h"

_ShDefault_(float3, _LightColor, float3(0.f, 0.f, 0.f));
_ShDefault_(float1, _LightPower, 0.f);

_ShDefault_(float3, _LightPosition, float3(0.f, 0.f, 0.f));
_ShDefault_(float1, _LightTheta, 0.f);

_ShDefault_(float3, _LightDirection, float3(0.f, 0.f, 0.f));
_ShDefault_(float1, _LightDistance, 0.f);

_ShDefault_(float1, _LightSinA, 0.f);
_ShDefault_(float1, _LightCosA, 0.f);
float2 _LightPadding;
