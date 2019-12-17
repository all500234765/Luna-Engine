#include "ShaderDefines.h"

Texture2D _AlbedoTex           TEXSLOT(0);
Texture2D _NormalTex           TEXSLOT(1);
Texture2D _MetallicTex         TEXSLOT(2);
Texture2D _RougnessTex         TEXSLOT(3);
Texture2D _EmissionTex         TEXSLOT(4);
Texture2D _AmbientOcclusionTex TEXSLOT(5);

SamplerState _AlbedoSampl           SAMPLSLOT(0);
SamplerState _NormalSampl           SAMPLSLOT(1);
SamplerState _MetallicSampl         SAMPLSLOT(2);
SamplerState _RougnessSampl         SAMPLSLOT(3);
SamplerState _EmissionSampl         SAMPLSLOT(4);
SamplerState _AmbientOcclusionSampl SAMPLSLOT(5);
