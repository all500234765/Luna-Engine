#include "ShaderDefines.h"

_ShDefault_(float3, _LightColor, float3())
_ShDefault_(float1, _LightPower, 0.f)

_ShDefault_(float3, _LightPosition, float3())
_ShDefault_(float1, _LightRadius, 0.f)

_ShDefault_(uint, _LightDynamic, 0)
uint3 _LightPadding;
