#pragma once

// External
//#include "Engine/External/Ansel.h"

// Ansel support
#if USE_ANSEL
#include "Vendor/Ansel/AnselSDK.h"

#ifdef _WIN64
#pragma comment(lib, "Ansel/AnselSDK64.lib")
#else
#pragma comment(lib, "Ansel/AnselSDK32.lib")
#endif
#endif
