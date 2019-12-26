#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>

#include <DirectXMath.h>
#include "Engine Includes/Types.h"

static const std::vector<int> MouseButtons = {
    1, 2, 10, 20, 40
};

typedef enum {
    Left,
    Right,
    Middle,
    X1,
    X2,

    Count,

    Mask = 7, 

    AxisX = 8, 
    AxisY = 16, 
    AxisXY = AxisX | AxisY
} MouseButton;

#include "ButtonStateEnum.h"

typedef unsigned int uint32_t;

class Mouse {
private:
    struct State {
        bool mLeftButton   : 1;
        bool mRightButton  : 1;
        bool mMiddleButton : 1;
        bool mX1Button     : 1;
        bool mX2Button     : 1;
    };

    State mState;
    State mLastState;

    bool bIsFocused;
    float x = 0.f, y = 0.f;
    float dx = 0.f, dy = 0.f;
    HWND m_hwnd;

    struct {
        ButtonState mLeftButton;
        ButtonState mRightButton;
        ButtonState mMiddleButton;
        ButtonState mX1Button;
        ButtonState mX2Button;

        ButtonState KeyState(MouseButton key) const {
            switch( key ) {
                case MouseButton::Left  : return mLeftButton;
                case MouseButton::Right : return mRightButton;
                case MouseButton::Middle: return mMiddleButton;
                case MouseButton::X1    : return mX1Button;
                case MouseButton::X2    : return mX2Button;
            }

            return ButtonState::UP;
        }
    } _mStates;

public:
    Mouse();
    Mouse(HWND q);

    void Reset();
    void Refresh();

    // Set mouse position in class
    void SetMouse(float X, float Y, bool rel=false);

    // Set mouse position in window
    void SetAt(float X, float Y);

    float GetDX() const { return dx; }
    float GetDY() const { return dy; }
    float GetX() const { return x; }
    float GetY() const { return y; }
    float2 GetXY() const { return float2((float)x, (float)y); }

    bool IsPressed(MouseButton mkey);
    bool IsDown(MouseButton mkey);
    bool IsReleased(MouseButton mkey);

    void SetState(WPARAM w, bool Down);
    void SetState(ULONG flags);
    void SetFocus(bool isFocused);
    void SetHWND(HWND handle);
};
