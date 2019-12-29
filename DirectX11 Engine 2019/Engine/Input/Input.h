#pragma once

#include "pc.h"


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
