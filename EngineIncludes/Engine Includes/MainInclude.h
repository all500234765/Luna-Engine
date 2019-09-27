#pragma once

#include "Engine/Extensions/Default.h"

// Core engine includes
#include "Core.h"
#include "Models.h"
#include "RenderBuffers.h"
#include "Textures.h"

// Audio engine using XAudio2
#include "Audio.h"

// Physics Engine
#define _LUNA_ENGINE_DX11_
#include "Physics.h"

// Text engine
#include "Text.h"

// External
#include "Ansel.h"
#include "HBAOPlus.h"
#include "ImGui.h"

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

typedef float               float1;
typedef DirectX::XMFLOAT2   float2;
typedef DirectX::XMFLOAT3   float3;
typedef DirectX::XMFLOAT4   float4;
typedef DirectX::XMVECTOR   vfloat;
typedef DirectX::XMFLOAT4X4 float4x4;
typedef DirectX::XMMATRIX   mfloat4x4;
typedef UINT                uint;
typedef UINT                uint1;
typedef DirectX::XMUINT2    uint2;
typedef DirectX::XMUINT3    uint3;
typedef DirectX::XMUINT4    uint4;
typedef INT                  int1;
typedef DirectX::XMINT2      int2;
typedef DirectX::XMINT3      int3;
typedef DirectX::XMINT4      int4;
typedef double               double1;

#include "HighLevel/DirectX/Utlities.h"

