#pragma once

#include <Windows.h>
#include <algorithm>

static LRESULT CALLBACK SplashScreenWndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

class SplashScreen {
protected:
    static HBITMAP image;
    static HINSTANCE hinstance;
    static UINT timer_time;
    static HWND m_hwnd;

public:
    static void Launch(const wchar_t* fname, UINT time);

    static UINT GetTimer() { return timer_time; }
    static HBITMAP GetImage() { return image; }
    static HWND GetHWND() { return m_hwnd; }
};
