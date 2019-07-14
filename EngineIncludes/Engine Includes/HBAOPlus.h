#pragma once

// HBAO+
#if USE_HBAO_PLUS
#include "Vendor/HBAOPlus/GFSDK_SSAO.h"

#ifdef _WIN64
#pragma comment(lib, "HBAOPlus/GFSDK_SSAO_D3D11.win64.lib")
#else
#pragma comment(lib, "HBAOPlus/GFSDK_SSAO_D3D11.win32.lib")
#endif
#endif
