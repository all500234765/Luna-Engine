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

class Mouse {
private:
    bool PressState[MouseButton::Count],
         DownState [MouseButton::Count],
         UpState   [MouseButton::Count];

    std::vector<WPARAM> Update;

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
    void SetState(USHORT flags);
};
