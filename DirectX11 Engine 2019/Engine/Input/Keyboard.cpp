#include "Keyboard.h"

Keyboard::Keyboard() {
    memset(&UpState, false, sizeof(UpState));
    memset(&DownState, false, sizeof(DownState));
    memset(&PressState, false, sizeof(PressState));
}

void Keyboard::SetState(WPARAM w, bool Down) {
    // Down state
    if( Down ) {
        // Was pressed => Press = false
        PressState[w] = !DownState[w];
        
        // Up state = false
        //UpState[w] = !DownState[w];

        //  
        DownState[w] = true;
    } else {
        // Up state
        UpState[w] = true;
        DownState[w] = false;
    }
}

bool Keyboard::IsDown(WPARAM key) {
    return DownState[key];
}

bool Keyboard::IsPressed(WPARAM key) {
    return PressState[key];
}

bool Keyboard::IsReleased(WPARAM key) {
    return UpState[key];
}
