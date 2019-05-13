#include "Input.h"

Input::Input() {
    //iKeyboard = new Keyboard();

    //iGamepads = (Gamepad*)malloc(sizeof(Gamepad) * NUM_GAMEPAD);
    for( int i = 0; i < NUM_GAMEPAD; i++ ) {
        iGamepads[i] = new Gamepad(i);
    }
}

Gamepad* Input::GetGamepad(int i) {
    return iGamepads[i];
}

Keyboard* Input::GetKeyboard() {
    return &iKeyboard;
}

void Input::PushKeyboardState(WPARAM w, bool Down) {
    iKeyboard.SetState(w, Down);
}

