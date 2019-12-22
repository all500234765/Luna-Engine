#pragma once

// ImGUI
#if _DEBUG_BUILD
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_win32.h"
#include "ImGUI/imgui_impl_dx11.h"

#ifdef _WIN64
#pragma comment(lib, "FreeType/win64/freetype.lib")
#else
#pragma comment(lib, "FreeType/win32/freetype.lib")
#endif
#endif
