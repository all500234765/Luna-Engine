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
    memset(&UpState, false, sizeof(UpState));
    memset(&DownState, false, sizeof(DownState));
    memset(&PressState, false, sizeof(PressState));
}

void Mouse::Refresh() {
    for( int i = 0; i < Update.size(); i++ ) {
        SetState(Update[i], true);
    }

    Update.clear();

    for( int i = 0; i < MouseButton::Count; i++ ) {
        if( UpState[i] ) {
            // Release button
            DownState[i]  = false;
            UpState[i]    = false;
            PressState[i] = false;
        }
    }
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

    // 
    POINT pt = {X, Y};
    x = X;
    y = Y;

    ClientToScreen(m_hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
}

int Mouse::GetX() {
    return x;
}

int Mouse::GetY() {
    return y;
}

bool Mouse::IsPressed(MouseButton mkey) {
    return PressState[mkey];
}

bool Mouse::IsDown(MouseButton mkey) {
    return DownState[mkey];
}

bool Mouse::IsReleased(MouseButton mkey) {
    return UpState[mkey];
}

void Mouse::SetState(WPARAM w, bool Down) {
    // Down state
    if( Down ) {
        // Was pressed => Press = false
        bool q = PressState[w];
        PressState[w] = !DownState[w];

        // Now button is on hold
        DownState[w] = true;

        // Call only if button wasn't pressed earlier
        if( q == false ) Update.push_back(w);
    } else {
        // Up state
        PressState[w] = false;
        UpState[w] = true;
        DownState[w] = false;
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
