#pragma once

#include "pc.h"

#define _____LUNA___ENGINE___DISCARD____ 1
#include "HighLevel/DirectX/Utlities.h"

#include "Engine/Extensions/Default.h"

// Core engine includes
#include "Core.h"

// Audio engine using XAudio2
#include "Audio.h"

// Physics Engine
#define _LUNA_ENGINE_DX11_
#include "Physics.h"

// Global game instances
extern _DirectX       *gDirectX      ;
extern Window         *gWindow       ;
extern Input          *gInput        ;
extern Mouse          *gMouse        ;
extern Keyboard       *gKeyboard     ;
extern AudioDevice    *gAudioDevice  ;
extern PhysicsEngine  *gPhysicsEngine;
extern Gamepad* gGamepad[NUM_GAMEPAD];

#include "Models.h"
#include "RenderBuffers.h"
#include "Textures.h"

// Text engine
#include "Text.h"

// External
#include "Ansel.h"
#include "HBAOPlus.h"
#include "ImGui.h"

//#include "Profiling.h"
#include "Engine/Profiler/RangeProfiler.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"

#include "Types.h"

#include "Engine/Model/Scene.h"

#ifndef WINMAIN
    #ifdef UNICODE
        #define WINMAIN wWinMain
        #define LPCMDLINE PWSTR 
    #else
        #define WINMAIN WinMain
        #define LPCMDLINE PSTR  
    #endif
#endif
