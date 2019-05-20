#include "Keyboard.h"

#include <iostream>

Keyboard::Keyboard() {
    memset(&mState, 0, sizeof(State));
    
    // Bug fix...
    /*SetState(VK_W, true);
    SetState(VK_S, true);*/
}

void Keyboard::Refresh() {
    auto currPtr = reinterpret_cast<const uint32_t*>(&mState);
    auto prevPtr = reinterpret_cast<const uint32_t*>(&mLastState);
    auto releasedPtr = reinterpret_cast<uint32_t*>(&mReleased);
    auto pressedPtr = reinterpret_cast<uint32_t*>(&mPressed);
    for( size_t j = 0; j < (256 / 32); ++j ) {
        *pressedPtr = *currPtr & ~(*prevPtr);
        *releasedPtr = ~(*currPtr) & *prevPtr;

        ++currPtr;
        ++prevPtr;
        ++releasedPtr;
        ++pressedPtr;
    }

    mLastState = mState;

}

Keyboard::Keyboard(HWND q) {
    Keyboard();

    RAWINPUTDEVICE rid[1];
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = q;
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x06;

    // Register RID
    if( !RegisterRawInputDevices(rid, 1, sizeof(rid[0])) ) {
        std::cout << "Can't register Keyboard RID: " << GetLastError() << std::endl;
    }
}

void Keyboard::SetState(WPARAM w, bool Down) {
    if( w < 0 || w > 0xfe ) return;
    auto ptr = reinterpret_cast<uint32_t*>(&mState);
    unsigned int bf = 1u << (w & 0x1f);

    if( Down ) {
        ptr[(w >> 5)] |= bf;
    } else {
        ptr[(w >> 5)] &= ~bf;
    }
}

bool Keyboard::IsDown(WPARAM key) {
    return mState.IsKeyDown((KeyboardUtils::Keys)key); // DownState[key];
}

bool Keyboard::IsPressed(WPARAM key) {
    // TODO: Release compilation: Fix somewhy true value by default or press issues
    return mPressed.IsKeyDown((KeyboardUtils::Keys)key); // ((reinterpret_cast<uint32_t>(&mState) & key) == key);
}

bool Keyboard::IsReleased(WPARAM key) {
    return mReleased.IsKeyDown((KeyboardUtils::Keys)key); // mState.IsKeyUp((KeyboardUtils::Keys)key); // UpState[key];
}
