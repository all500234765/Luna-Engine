#include "MainInclude.h"

_DirectX      *gDirectX       = 0;
Window        *gWindow        = 0;
Input         *gInput         = 0;
Mouse         *gMouse         = 0;
Keyboard      *gKeyboard      = 0;
AudioDevice   *gAudioDevice   = 0;
PhysicsEngine *gPhysicsEngine = 0;

#if USE_GAMEPADS
Gamepad* gGamepad[NUM_GAMEPAD] = {};
#endif
