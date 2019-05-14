#include "Keyboard.h"

#include <iostream>

Keyboard::Keyboard() {
    memset(&UpState, false, sizeof(UpState));
    memset(&DownState, false, sizeof(DownState));
    memset(&PressState, false, sizeof(PressState));
}

void Keyboard::Refresh() {
    for( int i = 0; i < 256; i++ ) {
        if( UpState[i] ) {
            // Release button
            DownState[i] = false;
            UpState[i] = false;
            PressState[i] = false;
        }
    }
}

Keyboard::Keyboard(HWND q) {
    Keyboard();

    RAWINPUTDEVICE rid[1];
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = q;
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x06;

    // Register RID
    if( !RegisterRawInputDevices(rid, 1, sizeof(rid[0])) ) 
    {
        std::cout << "Can't register Keyboard RID: " << GetLastError() << std::endl;
    }
}

void Keyboard::SetState(WPARAM w, bool Down) {
    // Down state
    if( Down ) {
        std::cout << "Pressed" << std::endl;

        // Was pressed => Press = false
        PressState[w] = !DownState[w];

        //  
        DownState[w] = true;
    } else {
        // Up state
        UpState[w] = true;
        DownState[w] = false;
        std::cout << "Released" << std::endl;
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
