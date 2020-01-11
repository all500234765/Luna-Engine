#define _CRT_SECURE_NO_WARNINGS

// Config
#include "Defines.h"

// DirectX
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3d11shadertracing.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

// WINAIP
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Windowsx.h>
#include <shlwapi.h>
#undef min
#undef max

// STL
#include <string>
#include <iostream>
#include <fstream>
#include <variant>
#include <algorithm>
#include <array>
#include <vector>
#include <locale>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <initializer_list>
#include <thread>
#include <chrono>
#include <tuple>
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <limits.h>
#include <intrin.h>

// Audio / Input
#include <xaudio2.h>
#include <x3daudio.h>
#include <Xinput.h>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "Xinput.lib")

// Vendor

// STB
#define STB_VORBIS_HEADER_ONLY
#include "STB/stb_vorbis.h"
#include "STB/stbi_image.h"

// DDS
#include "TinyDDSLoader/TinyDDSLoader.h"
#include "DirectXTex/DDSTextureLoader.h"

// DirectXMesh
#include "DirectXMesh/DirectXMeshP.h"
#include "DirectXMesh/DirectXMesh.h"

// Ansel
#ifdef USE_ANSEL
    /*#include "Ansel/ansel/Camera.h"
    #include "Ansel/ansel/Configuration.h"
    #include "Ansel/ansel/Defines.h"
    #include "Ansel/ansel/Hints.h"
    #include "Ansel/ansel/Session.h"
    #include "Ansel/ansel/UserControls.h"
    #include "Ansel/ansel/Version.h"
    #include "Ansel/nv/Quat.h"
    #include "Ansel/nv/Vec3.h"*/
    #include "Ansel/AnselSDK.h"
#endif

// HBAO+
#ifdef USE_HBAO
    #include "HBAOPlus/GFSDK_SSAO.h"
#endif

// Assimp
#include "Assimp/Importer.hpp"
#include "Assimp/scene.h"
#include "Assimp/postprocess.h"
#include "Assimp/pbrmaterial.h"

#ifdef _WIN64
    #pragma comment(lib, "Assimp/x64/assimp-vc141-mt.lib")
#else
    #pragma comment(lib, "Assimp/x86/assimp-vc141-mt.lib")
#endif

// ImGui
#include "ImGUI/imgui.h"

// Hate this thing
#undef _____________TYPE_FLOAT_OPERATORS22____3333
#include "Engine Includes/Types.h"
#include "Other/FloatTypeMath.h"

// Utilities w/o dependencies
#include "Engine/RendererFlags.h"

