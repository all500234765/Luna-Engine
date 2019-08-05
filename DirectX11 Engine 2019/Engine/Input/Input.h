#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Defines.h"

#if USE_GAMEPADS
#pragma comment(lib, "Xinput.lib")

#include "Gamepad.h"
#endif

#include "Keyboard.h"
#include "Mouse.h"

class Input {
private:
#if USE_GAMEPADS
    Gamepad* iGamepads[NUM_GAMEPAD];
#endif

    Keyboard iKeyboard;
    Mouse iMouse;

public:
    Input();
    Input(HWND q);

#if USE_GAMEPADS
    Gamepad* GetGamepad(int i);
#endif

    Keyboard* GetKeyboard();
    Mouse* GetMouse();

    void PushKeyboardState(WPARAM w, bool Down);
};
