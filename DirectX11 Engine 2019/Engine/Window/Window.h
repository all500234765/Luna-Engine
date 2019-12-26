#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Input/Input.h"
#include "Engine/DirectX/DirectX.h"
#include "Engine/States/PipelineState.h"

struct WindowConfig {
    // On create only
    const wchar_t* Title;
    const wchar_t* Icon;

    int Width;
    int Height;

    // Can be modified
    bool Borderless;
    bool Windowed;
    bool ShowConsole;

    int CurrentWidth, CurrentHeight;
    int CurrentHeight2; // Window height w/o top bar
    bool Resized; // Modification implemented
};

class Window: public PipelineState<Window> {
private:
    bool (_DirectX::*FrameFunction)();
    int screenWidth, screenHeight;
    WindowConfig cfg;
    HWND m_hwnd;
    HINSTANCE m_hinstance;
    _DirectX *gDirectX;
    Input *gInput;

    RECT mRect;

    HICON hIcon;

public:
    void Create(const WindowConfig& config);
    void Loop();
    void SetFrameFunction(bool(_DirectX::*ff)());
    void SetDirectX(_DirectX* inst);
    void Destroy();
    
    HWND GetHWND();
    const WindowConfig& GetCFG();
    Input *GetInputDevice();
    inline RECT GetRect() const { return mRect; };

    friend static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    friend static LRESULT CALLBACK InputWndProc(HWND, UINT, WPARAM, LPARAM);
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK InputWndProc(HWND, UINT, WPARAM, LPARAM);
static Window* ApplicationHandle = 0;
static float fTimeDelay = 1000.f / 60.f;
