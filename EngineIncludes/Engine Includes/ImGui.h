#pragma once

// ImGUI
#if _DEBUG_BUILD
#include "Vendor/ImGUI/imgui.h"
#include "Vendor/ImGUI/imgui_impl_win32.h"
#include "Vendor/ImGUI/imgui_impl_dx11.h"

#ifdef _WIN64
#pragma comment(lib, "FreeType/win64/freetype.lib")
#else
#pragma comment(lib, "FreeType/win32/freetype.lib")
#endif
#endif
