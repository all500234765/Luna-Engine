#include "Input.h"

Input::Input() {
#if USE_GAMEPADS
    for( int i = 0; i < NUM_GAMEPAD; i++ ) {
        iGamepads[i] = i;
    }
#endif
}

Input::Input(HWND q) {
    Input();
    iMouse = q;
    iKeyboard = q;
}

#if USE_GAMEPADS
Gamepad* Input::GetGamepad(int i) {
    return &iGamepads[i];
}
#endif

Keyboard* Input::GetKeyboard() {
    return &iKeyboard;
}

Mouse* Input::GetMouse() {
    return &iMouse;
}

void Input::PushKeyboardState(WPARAM w, bool Down) {
    iKeyboard.SetState(w, Down);
}
