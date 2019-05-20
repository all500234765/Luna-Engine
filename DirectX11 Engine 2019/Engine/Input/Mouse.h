#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>

static const std::vector<int> MouseButtons = {
    1, 2, 10, 20, 40
};

typedef enum {
    Left,
    Right,
    Middle,
    X1,
    X2,

    Count
} MouseButton;

typedef unsigned int uint32_t;

class Mouse {
private:
    struct State {
        bool Left : 1;
        bool Right : 1;
        bool Middle : 1;
        bool X1 : 1;
        bool X2 : 1;

        bool __cdecl IsKeyDown(MouseButton key) const {
            if( key >= 0 && key <= 0xfe ) {
                auto ptr = reinterpret_cast<const bool*>(this);
                unsigned int bf = 1u << (key & 0x1f);
                return (ptr[(key >> 5)] & bf) != 0;
            }
            return false;
        }

        bool __cdecl IsKeyUp(MouseButton key) const {
            if( key >= 0 && key <= 0xfe ) {
                auto ptr = reinterpret_cast<const bool*>(this);
                unsigned int bf = 1u << (key & 0x1f);
                return (ptr[(key >> 5)] & bf) == 0;
            }
            return false;
        }
    };

    State mState;
    State mPressed;
    State mReleased;
    State mLastState;

    bool bIsFocused;
    int x = 0, y = 0;
    HWND m_hwnd;
public:
    Mouse();
    Mouse(HWND q);

    void Reset();
    void Refresh();

    // Set mouse position in class
    void SetMouse(int X, int Y, bool rel=false);

    // Set mouse position in window
    void SetAt(int X, int Y);

    int GetX();
    int GetY();

    bool IsPressed(MouseButton mkey);
    bool IsDown(MouseButton mkey);
    bool IsReleased(MouseButton mkey);

    void SetState(WPARAM w, bool Down);
    void SetState(ULONG flags);
    void SetFocus(bool isFocused);
    void SetHWND(HWND handle);
};
