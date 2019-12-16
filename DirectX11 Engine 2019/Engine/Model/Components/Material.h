#include "ShaderDefines.h"

bool _IsTransparent;
bool _UseVertexColor;
/*bool _Alb;
bool _Norm;
bool _Metal;
bool _Rough;
bool _AO;
bool _Emis;*/
bool _MatPadding[24 + 6];

float _Alpha;

float _AlbedoMul;
float _NormalMul;
float _MetallnessMul;
float _RoughnessMul;
float _AmbientOcclusionMul;
float _EmissionMul;

float _MatPadding1;
float3 _EmissionColor;
float _MatPadding2;

