#pragma once

// Engine
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
#include "Engine/Models/Mesh.h"
#include "Engine/Models/Model.h"
#include "Engine/Models/ModelInstance.h"
#include "Engine/Camera/Camera.h"
#include "Engine/Materials/Sampler.h"
#include "Engine/Materials/Texture.h"
#include "Engine/Materials/Material.h"
#include "Engine/RenderBuffer/RenderBufferBase.h"
#include "Engine/RenderBuffer/RenderBufferDepth2D.h"
#include "Engine/RenderBuffer/RenderBufferColor1.h"
#include "Engine/RenderBuffer/RenderBufferColor1Depth.h"
#include "Engine/RenderBuffer/RenderBufferColor2Depth.h"
#include "Engine/RenderBuffer/RenderBufferColor3Depth.h"
#include "Engine/Textures/CubemapTexture.h"
#include "Engine/Textures/CustomTexture.h"
#include "Engine/DirectX/Query.h"
#include "Engine/States/BlendState.h"

//#include "Engine/External/Ansel.h"

// Audio engine using XAudio2
#include "Audio/AudioDevice.h"
#include "Audio/AudioDeviceChild.h"
#include "Audio/Sound.h"
#include "Audio/SoundEffect.h"

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


// Global game instances
static _DirectX    *gDirectX     = 0;
static Window      *gWindow      = 0;
static Input       *gInput       = 0;
static Mouse       *gMouse       = 0;
static Keyboard    *gKeyboard    = 0;
static AudioDevice *gAudioDevice = 0;

#if USE_GAMEPADS
static Gamepad* gGamepad[NUM_GAMEPAD] = {};
#endif

