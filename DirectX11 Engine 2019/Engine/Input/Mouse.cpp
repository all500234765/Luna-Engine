#include "pc.h"
#include "Mouse.h"

Mouse::Mouse() {
    Reset();
}

Mouse::Mouse(HWND q): m_hwnd(q) {
    Reset();

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

#ifndef UPDATE_BUTTON_STATE
#define UPDATE_BUTTON_STATE(field) _mStates.field = static_cast<ButtonState>( ( !!mState.field ) | ( ( !!mState.field ^ !!mLastState.field ) << 1 ) );
#endif

void Mouse::Refresh() {
    UPDATE_BUTTON_STATE(mLeftButton);
    UPDATE_BUTTON_STATE(mRightButton);
    UPDATE_BUTTON_STATE(mMiddleButton);
    UPDATE_BUTTON_STATE(mX1Button);
    UPDATE_BUTTON_STATE(mX2Button);

    mLastState = mState;
}

void Mouse::SetMouse(float X, float Y, bool rel) {
    
    if( rel ) {
        x += X;
        y += Y;

        dx = X;
        dy = Y;
    } else {
        float ox = x, oy = y;

        x = X;
        y = Y;

        dx = x - ox;
        dy = y - oy;
    }

    /*if( mark_count > 0 && (abs(dx) > 0.f || abs(dy) > 0.f) ) {
        mark_count--;
        dx = 0;
        dy = 0; printf_s("%u\n", mark_count);
    }*/
}

void Mouse::SetAt(float X, float Y, bool abs) {
    // If window isn't focused, then do nothing
    if( GetFocus() != m_hwnd ) { return; }

    // Create vector
    POINT pt = { X, Y };

    // Store new mouse pos
    if( !abs ) {
        x = X;
        y = Y;
    } else {
        POINT pt2 = { 0, 0 };
        ClientToScreen(m_hwnd, &pt2);
        x = X - pt2.x;
        y = Y - pt2.y;
    }

    // TODO: 
    dx = 0;
    dy = 0;

    // Map point to the screen
    if( !abs ) ClientToScreen(m_hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
}

bool Mouse::IsPressed(MouseButton mkey) {
    return _mStates.KeyState(mkey) == ButtonState::PRESSED;
}

bool Mouse::IsDown(MouseButton mkey) {
    return _mStates.KeyState(mkey) == ButtonState::HELD;
}

bool Mouse::IsReleased(MouseButton mkey) {
    return _mStates.KeyState(mkey) == ButtonState::RELEASED;
}

void Mouse::SetState(WPARAM w, bool Down) {
    if( w < 0 || w > Count ) return;
    auto ptr = reinterpret_cast<bool*>(&mState);
    unsigned int bf = 1u << (w & 0x1f); // 1u << (w & 0x1f);

    if( Down ) {
        //mState.val[w] |= bf;
        mState.val |= bf;
    } else {
        //mState.val[w] &= ~bf;
        mState.val &= ~bf;
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
    if( flags & RI_MOUSE_BUTTON_1_DOWN ) SetState(X1, true);
    if( flags & RI_MOUSE_BUTTON_1_UP   ) SetState(X1, false);

    if( flags & RI_MOUSE_BUTTON_2_DOWN ) SetState(X2, true);
    if( flags & RI_MOUSE_BUTTON_2_UP   ) SetState(X2, false);
}
