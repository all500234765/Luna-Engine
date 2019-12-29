#include "pc.h"
#include "Input.h"

Input::Input() {
    for( int i = 0; i < NUM_GAMEPAD; i++ ) {
        iGamepads[i] = new Gamepad(i);
    }
}

Input::Input(HWND q) {
    for( int i = 0; i < NUM_GAMEPAD; i++ ) {
        iGamepads[i] = new Gamepad(i);
    }

    iMouse = q;
    iKeyboard = q;
}

Gamepad* Input::GetGamepad(int i) {
    return iGamepads[i];
}

Keyboard* Input::GetKeyboard() {
    return &iKeyboard;
}

Mouse* Input::GetMouse() {
    return &iMouse;
}

void Input::PushKeyboardState(WPARAM w, bool Down) {
    iKeyboard.SetState(w, Down);
}
