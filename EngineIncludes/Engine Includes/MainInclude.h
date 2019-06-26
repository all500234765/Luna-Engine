#pragma once

#define _LUNA_ENGINE_DX11_

// Core Engine
#include "Engine/Input/Input.h"
#include "Engine/Input/Gamepad.h"
#include "Engine/Window/Window.h"
#include "Engine/DirectX/DirectX.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/DirectX/PolygonLayout.h"
#include "Engine/DirectX/Buffer.h"
#include "Engine/DirectX/IndexBuffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/Query.h"

// Models
#include "Engine/Models/Mesh.h"
#include "Engine/Models/Model.h"
#include "Engine/Models/ModelInstance.h"

// Cameras, lights
#include "Engine/Camera/Camera.h"

// Materials
#include "Engine/Materials/Sampler.h"
#include "Engine/Materials/Texture.h"
#include "Engine/Materials/Material.h"

// Render Buffers
#include "Engine/RenderBuffer/RenderBufferBase.h"
#include "Engine/RenderBuffer/RenderBufferDepth2D.h"
#include "Engine/RenderBuffer/RenderBufferColor1.h"
#include "Engine/RenderBuffer/RenderBufferColor1Depth.h"
#include "Engine/RenderBuffer/RenderBufferColor2Depth.h"
#include "Engine/RenderBuffer/RenderBufferColor3Depth.h"

// Textures
#include "Engine/Textures/CubemapTexture.h"
#include "Engine/Textures/CustomTexture.h"

// States
#include "Engine/States/BlendState.h"

// External
//#include "Engine/External/Ansel.h"

// Audio engine using XAudio2
#include "Audio/AudioDevice.h"
#include "Audio/AudioDeviceChild.h"
#include "Audio/Sound.h"
#include "Audio/SoundEffect.h"
#include "Audio/Music.h"

// Physics Engine
#include "Physics/Collision/CollisionData.h"
#include "Physics/Generics/PhysicsObject.h"
#include "Physics/Generics/PhysicsObjectSphere.h"
#include "Physics/Generics/PhysicsObjectPlane.h"
#include "Physics/Generics/PhysicsObjectAABB.h"
#include "Physics/Main/PhysicsEngine.h"
#include "Physics/Main/PhysicsEngineDX11.h"
#include "Physics/Main/PhysicsEngineSIMD.h"

// Ansel support
#if USE_ANSEL
#include "Vendor/Ansel/AnselSDK.h"

#ifdef _WIN64
#pragma comment(lib, "Ansel/AnselSDK64.lib")
#else
#pragma comment(lib, "Ansel/AnselSDK32.lib")
#endif
#endif

// HBAO+
#if USE_HBAO_PLUS
#include "Vendor/HBAOPlus/GFSDK_SSAO.h"

#ifdef _WIN64
#pragma comment(lib, "HBAOPlus/GFSDK_SSAO_D3D11.win64.lib")
#else
#pragma comment(lib, "HBAOPlus/GFSDK_SSAO_D3D11.win32.lib")
#endif
#endif

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

// Global game instances
static _DirectX      *gDirectX       = 0;
static Window        *gWindow        = 0;
static Input         *gInput         = 0;
static Mouse         *gMouse         = 0;
static Keyboard      *gKeyboard      = 0;
static AudioDevice   *gAudioDevice   = 0;
static PhysicsEngine *gPhysicsEngine = 0;

#if USE_GAMEPADS
static Gamepad* gGamepad[NUM_GAMEPAD] = {};
#endif

