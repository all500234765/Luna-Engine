#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Defines.h"

#pragma comment(lib, "Xinput.lib")

#include "Gamepad.h"

#include "Keyboard.h"
#include "Mouse.h"

class Input {
private:
    Gamepad* iGamepads[NUM_GAMEPAD];
    Keyboard iKeyboard;
    Mouse iMouse;

public:
    Input();
    Input(HWND q);

    Gamepad* GetGamepad(int i);
    Keyboard* GetKeyboard();
    Mouse* GetMouse();

    void PushKeyboardState(WPARAM w, bool Down);
};
