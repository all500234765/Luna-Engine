#include "Mouse.h"

#include <iostream>

Mouse::Mouse() {
    Reset();
}

Mouse::Mouse(HWND q): m_hwnd(q) {
    Mouse();

    RAWINPUTDEVICE rid[1];
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = q;
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;
    
    // Register RID
    if( !RegisterRawInputDevices(rid, 1, sizeof(rid[0])) ) {
        std::cout << "Can't register Mouse RID: " << GetLastError() << std::endl;
    }
}

void Mouse::Reset() {
    memset(&mState, 0, sizeof(State));
}

void Mouse::Refresh() {
    auto currPtr = reinterpret_cast<const bool*>(&mState);
    auto prevPtr = reinterpret_cast<const bool*>(&mLastState);
    auto releasedPtr = reinterpret_cast<bool*>(&mReleased);
    auto pressedPtr = reinterpret_cast<bool*>(&mPressed);
    for( size_t j = 0; j < (Count / 1); ++j ) {
        *pressedPtr = *currPtr & ~(*prevPtr);
        *releasedPtr = ~(*currPtr) & *prevPtr;

        ++currPtr;
        ++prevPtr;
        ++releasedPtr;
        ++pressedPtr;
    }

    mLastState = mState;
}

void Mouse::SetMouse(int X, int Y, bool rel) {
    if( rel ) {
        this->x += X;
        this->y += Y;
    } else {
        this->x = X;
        this->y = Y;
    }
}

void Mouse::SetAt(int X, int Y) {
    // If window isn't focused, then do nothing
    if( GetFocus() != m_hwnd ) { return; }

    // Create vector
    POINT pt = {X, Y};

    // Store new mouse pos
    this->x = X;
    this->y = Y;

    // Map point to the screen
    ClientToScreen(m_hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
}

int Mouse::GetX() {
    return this->x;
}

int Mouse::GetY() {
    return this->y;
}

bool Mouse::IsPressed(MouseButton mkey) {
    return mPressed.IsKeyDown(mkey);
}

bool Mouse::IsDown(MouseButton mkey) {
    return mState.IsKeyDown(mkey);
}

bool Mouse::IsReleased(MouseButton mkey) {
    // TODO: Somewhy not being registered
    return mReleased.IsKeyDown(mkey);
}

void Mouse::SetState(WPARAM w, bool Down) {
    if( w < 0 || w > Count ) return;
    auto ptr = reinterpret_cast<bool*>(&mState);
    unsigned int bf = 1u << (w & 0x1f);

    if( Down ) {
        ptr[(w >> 5)] |= bf;
    } else {
        ptr[(w >> 5)] &= ~bf;
    }
}

void Mouse::SetState(ULONG flags) {
    // https://docs.microsoft.com/en-us/windows/desktop/api/winuser/ns-winuser-tagrawmouse
    if( flags & RI_MOUSE_LEFT_BUTTON_DOWN ) SetState(Left, true);
    if( flags & RI_MOUSE_LEFT_BUTTON_UP   ) SetState(Left, false);

    if( flags & RI_MOUSE_RIGHT_BUTTON_DOWN ) SetState(Right, true);
    if( flags & RI_MOUSE_RIGHT_BUTTON_UP   ) SetState(Right, false);

    if( flags & RI_MOUSE_MIDDLE_BUTTON_DOWN ) SetState(Middle, true);
    if( flags & RI_MOUSE_MIDDLE_BUTTON_UP   ) SetState(Middle, false);

    // Somewhy it's being called after LMB
    // TODO: Fix it
    //if( flags & RI_MOUSE_BUTTON_1_DOWN ) SetState(X1, true);
    //if( flags & RI_MOUSE_BUTTON_1_UP   ) SetState(X1, false);

    if( flags & RI_MOUSE_BUTTON_2_DOWN ) SetState(X2, true);
    if( flags & RI_MOUSE_BUTTON_2_UP   ) SetState(X2, false);
}
