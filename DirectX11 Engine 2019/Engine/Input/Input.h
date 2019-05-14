#pragma once

#include "Engine/Extensions/Default.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if USE_GAMEPADS
#pragma comment(lib, "Xinput.lib")
#include <Xinput.h>

#include "Gamepad.h"
#endif

#include "Keyboard.h"
#include "Mouse.h"

class Input {
private:
#if USE_GAMEPADS > 0
    Gamepad *iGamepads[NUM_GAMEPAD];
#endif

    Keyboard iKeyboard;
    Mouse iMouse;

public:
    Input();
    Input(HWND q);

#if USE_GAMEPADS > 0
    Gamepad* GetGamepad(int i);
#endif

    Keyboard* GetKeyboard();
    Mouse* GetMouse();

    void PushKeyboardState(WPARAM w, bool Down);
};
