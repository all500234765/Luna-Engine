#include "ShaderDefines.h"

_ShDefault_(float3, _LightColor, float3(0.f, 0.f, 0.f))
_ShDefault_(float1, _LightPower, 0.f)

_ShDefault_(float3, _LightPosition, float3(0.f, 0.f, 0.f))
_ShDefault_(float1, _LightTheta, 0.f)

_ShDefault_(float3, _LightDirection, float3(0.f, 0.f, 1.f))
_ShDefault_(float1, _LightDistance, 0.f)

_ShDefault_(float1, _LightCutOff   , 0.f)
_ShDefault_(float1, _LightOutCutOff, 0.f)
float2 _LightPadding0;

// TODO: Replace with quat
_ShDefault_(float3x3, _LightRotMat, float3x3(1.f, 0.f, 0.f, 
                                             0.f, 1.f, 0.f, 
                                             0.f, 0.f, 1.f))
float3 _LightPadding1;
