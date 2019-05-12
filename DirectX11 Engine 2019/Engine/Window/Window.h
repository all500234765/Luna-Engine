#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/DirectX/DirectX.h"

struct WindowConfig {
    // On create only
    const wchar_t* Title;
    const wchar_t* Icon;

    int Width;
    int Height;

    // Can be modified
    bool Borderless;
    bool Windowed;

    int CurrentWidth, CurrentHeight;
    bool Resized; // Modification implemented
};

class Window {
private:
    bool (_DirectX::*FrameFunction)();
    int screenWidth, screenHeight;
    WindowConfig cfg;
    HWND m_hwnd;
    HINSTANCE m_hinstance;
    _DirectX *gDirectX;

    HICON hIcon;

public:
    void Create(WindowConfig* config);
    void Loop();
    void SetFrameFunction(bool(_DirectX::*ff)());
    void SetDirectX(_DirectX* inst);
    void Destroy();
    
    HWND GetHWND();
    const WindowConfig& GetCFG();

    friend static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static Window* ApplicationHandle = 0;
