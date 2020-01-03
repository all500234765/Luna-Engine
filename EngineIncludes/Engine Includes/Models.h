#pragma once

#ifdef _WIN64
#pragma comment(lib, "Assimp/x64/assimp-x64.lib")
#else
#pragma comment(lib, "Assimp/x86/assimp-x86.lib")
#endif

// Models
#include "Engine/Models/Mesh.h"
#include "Engine/Models/Model.h"
#include "Engine/Models/ModelInstance.h"

// Cameras, lights
#include "Engine/Camera/Camera.h"

// Materials
#include "Engine/Scene/Sampler.h"
#include "Engine/Materials/Texture.h"
#include "Engine/Scene/Texture2.h"

