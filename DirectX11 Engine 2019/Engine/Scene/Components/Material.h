#include "ShaderDefines.h"

_ShDefault_(float, _IsTransparent, 0);
_ShDefault_(float, _UseVertexColor, 0);
_ShDefault_(float, _FlipNormals, 0);
_ShDefault_(float, _Alb, 0);

_ShDefault_(float, _Norm, 0);
_ShDefault_(float, _Metal, 0);
_ShDefault_(float, _Rough, 0);
_ShDefault_(float, _Emis, 0);

_ShDefault_(float, _AO, 0);
_ShDefault_(float, _ShadowReceiver, 1);
_ShDefault_(float, _ShadowCaster, 1);
_ShDefault_(float, _MatPadding1, 0);

_ShDefault_(float, _Alpha, 1);
_ShDefault_(float, _AlbedoMul, 1);
_ShDefault_(float, _NormalMul, 1);
_ShDefault_(float, _MetallnessMul, 1);

_ShDefault_(float, _RoughnessMul, 1.f);
_ShDefault_(float, _AmbientOcclusionMul, 1.f);
_ShDefault_(float, _EmissionMul, 1.f);
_ShDefault_(uint32_t, _MaterialLayer, 0xFFFFFFFF);

_ShDefault_(float3, _EmissionColor, float3(0.f, 0.f, 0.f));
_ShDefault_(float, _MatPadding2, 0);

#ifdef _MATERIAL_EXT_

// C++ side only
_Shader_(_Shader);
_Shader_(_ShaderDepth);
_ShDefault_(uint32_t, _MatDrawCallType, DXDRAWINDEXED);
_ShDefault_(uint32_t, _MatBindingShader, 0);
Topology _MatTopology = (Topology)0;

#endif

