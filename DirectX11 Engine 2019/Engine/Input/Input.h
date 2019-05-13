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

class Input {
private:
    Gamepad *iGamepads[NUM_GAMEPAD];
    Keyboard iKeyboard;
    //Mouse iMouse;

public:
    Input();

    Gamepad* GetGamepad(int i);
    Keyboard* GetKeyboard();
    //Mouse* GetMouse();

    void PushKeyboardState(WPARAM w, bool Down);
};
