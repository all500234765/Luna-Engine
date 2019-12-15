#pragma once

#include "HighLevel/DirectX/Utlities.h"

#include "Engine/Extensions/Default.h"

// Core engine includes
#include "Core.h"
#include "Models.h"
#include "RenderBuffers.h"
#include "Textures.h"
#include "Engine/Model/Scene.h"

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

//#include "Profiling.h"
#include "Engine/Profiler/RangeProfiler.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"

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

#include "Types.h"

