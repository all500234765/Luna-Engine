#include <Windowsx.h>
#include <iostream>
#include <chrono>

#include "Window.h"

void Window::Create(const WindowConfig& config) {
    WNDCLASSEX wc;
    int posX, posY;
    DEVMODE dmScreenSettings;

    // Save configuration
    cfg = config;

    // 
    ApplicationHandle = this;

    // Get the instance of this application.
    m_hinstance = GetModuleHandle(NULL);

    // Setup the windows class with default settings.
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = config.Title;
    wc.cbSize = sizeof(WNDCLASSEX);

    // Load icon
    hIcon = (HICON)LoadImage(NULL, config.Icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE | LR_SHARED);
    if( hIcon ) {
        wc.hIconSm = hIcon;
        wc.hIcon   = hIcon;
    } else {
        std::wstring w = config.Icon;
        std::cout << "Failed to load icon. (" << std::string(w.begin(), w.end()).c_str() << ")" << std::endl;
    }

    // Register the window class.
    RegisterClassEx(&wc);

    // Determine the resolution of the clients desktop screen.
    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Mode
    if( config.Windowed ) {
        // If windowed then set it to custom resolution
        if( !config.Width  ) cfg.CurrentWidth  = screenWidth ; else cfg.CurrentWidth  = config.Width;
        if( !config.Height ) cfg.CurrentHeight = screenHeight; else cfg.CurrentHeight = config.Height;

        // Place the window in the middle of the screen.
        posX = max((screenWidth  - cfg.CurrentWidth ) / 2, 0);
        posY = max((screenHeight - cfg.CurrentHeight) / 2, 0);
    } else {
        // If full screen set the screen to maximum size of the users desktop and 32bit.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if( !config.Borderless ) {
            // Change the display settings to full screen.
            ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
        }

        cfg.CurrentWidth  = screenWidth;
        cfg.CurrentHeight = screenHeight;

        // Set the position of the window to the top left corner.
        posX = posY = 0;
    }

    // Window flags
    unsigned int flags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    // Borderless flag
    if( config.Borderless ) flags |= WS_POPUP;
    else                    flags |= WS_OVERLAPPEDWINDOW;

    // Adjust window size and position
    cfg.CurrentHeight2 = cfg.CurrentHeight;// -1;

    RECT rect = {posX, posY, posX + cfg.CurrentWidth, posY + cfg.CurrentHeight};
    AdjustWindowRectEx(&rect, flags, false, WS_EX_APPWINDOW);
    
    posX = rect.left;
    posY = rect.top;
    cfg.CurrentWidth   = rect.right  - posX;
    cfg.CurrentHeight  = rect.bottom - posY;

    // Create the window with the screen settings and get the handle to it.
    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, config.Title, config.Title, flags,
                            posX, posY, cfg.CurrentWidth, cfg.CurrentHeight,
                            NULL, NULL, m_hinstance, NULL);

    // Bring the window up on the screen and set it as main focus.
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    // Create message only window
    /*HWND input_hwnd = 0;
    WNDCLASSEX wx = {};
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = InputWndProc;        // function which will handle messages
    wx.hInstance = GetModuleHandle(NULL);
    wx.lpszClassName = L"Input";
    if( RegisterClassEx(&wx) ) {
        input_hwnd = CreateWindowEx(0, wx.lpszClassName, L"Input", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    }*/

    // Setup input
    gInput = new Input(m_hwnd);

    // 
    std::cout << "New window(x=" << posX << ", y=" << posY << ", w=" << cfg.CurrentWidth << ", h=" << cfg.CurrentHeight << ")" << std::endl;

    // Hide the mouse cursor.
    //ShowCursor(false);

    // Show / Hide console
    ShowWindow(GetConsoleWindow(), config.ShowConsole ? SW_SHOW : SW_HIDE);
}

void Window::Loop() {
    if( FrameFunction == nullptr ) {
        MessageBox(NULL, L"No frame function defined!", L"Error", MB_OK);
    }

    MSG msg;

    // Initialize the message structure.
    ZeroMemory(&msg, sizeof(MSG));

    // Create new clock
    std::chrono::high_resolution_clock Clock;
    fTimeDelay = 1000.f / (float)gDirectX->GetConfig().RefreshRate;
    std::chrono::time_point<std::chrono::steady_clock> tLast;
    auto tp1 = Clock.now();
    auto tp2 = tp1;

    // Loop until there is a quit message from the window or the user.
    while( true ) {
        // Reset states
        cfg.Resized = false;

        // Handle the windows messages.
        while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // If windows signals to end the application then exit out.
            if( msg.message == WM_QUIT ) {
                return;
            }
        }

        // Reset states
        gInput->GetMouse()->Refresh();
        gInput->GetKeyboard()->Refresh();

#if USE_GAMEPADS
        for( int i = 0; i < NUM_GAMEPAD; i++ ) gInput->GetGamepad(i)->Update();
#endif

        // Calculate dt and fps
        tp2 = Clock.now();
        std::chrono::duration<float> dt = tp2 - tp1;
        tp1 = tp2;
        float fDeltaTime = dt.count();
        
        //float fFPS = 1.f / fDeltaTime;

        // Tick function
        // Collisions, AI, Input etc...
        gDirectX->Tick(fDeltaTime);

#if USE_GAMEPADS
        for( int i = 0; i < NUM_GAMEPAD; i++ ) gInput->GetGamepad(i)->Refresh();
#endif

        // Otherwise do the frame processing.
        if( gDirectX->FrameFunction() ) { break; }
    }
}

void Window::SetFrameFunction(bool(_DirectX::*ff)()) {
    FrameFunction = ff;
}

void Window::SetDirectX(_DirectX *inst) {
    gDirectX = inst;
}

void Window::Destroy() {
    // Show the mouse cursor.
    ShowCursor(true);

    // Fix the display settings if leaving full screen mode.
    if( !cfg.Windowed ) {
        ChangeDisplaySettings(NULL, CDS_RESET);
    }

    // Remove the window.
    DestroyWindow(m_hwnd);
    m_hwnd = NULL;

    // Remove the application instance.
    UnregisterClass(cfg.Title, m_hinstance);
    m_hinstance = NULL;

    // Release the pointer to this class.
    ApplicationHandle = NULL;

    // 
    DestroyIcon(hIcon);
}

HWND Window::GetHWND() {
    return m_hwnd;
}

const WindowConfig& Window::GetCFG() {
    return cfg;
}

Input* Window::GetInputDevice() {
    return gInput;
}

LRESULT CALLBACK InputWndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    /*switch( umessage ) {
        // Keyboard
        /*case WM_KEYUP:
            ApplicationHandle->gInput->PushKeyboardState(wparam, false);
            return 0;

        case WM_KEYDOWN:
            //if( (lparam & 0x40000000) == 0 ) 
            ApplicationHandle->gInput->PushKeyboardState(wparam, true);
            return 0;* /
    }*/

    return DefWindowProc(hwnd, umessage, wparam, lparam);
}

// ImGUI
#define _DEBUG_BUILD 1
#if _DEBUG_BUILD
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
#endif

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
#if _DEBUG_BUILD
    if( ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam) ) {
        return true;
    }
#endif

    MINMAXINFO* info = (MINMAXINFO*)(lparam);

    switch( umessage ) {
        // Check if the window is being destroyed.
        case WM_DESTROY:
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        case WM_ERASEBKGND:
            return 0;

        case WM_GETMINMAXINFO:
            // You should set a minimize window size that is reasonable for your app
            info->ptMinTrackSize.x = 380;
            info->ptMinTrackSize.y = 200;
            return 0;

        case WM_SIZE:
            // Window was resized
            if( ApplicationHandle == nullptr ) return 0;
            ApplicationHandle->cfg.CurrentWidth   = LOWORD(lparam);
            ApplicationHandle->cfg.CurrentHeight  = HIWORD(lparam);
            ApplicationHandle->cfg.CurrentHeight2 = HIWORD(lparam) - 1;// +(ApplicationHandle->cfg.CurrentHeight2 - ApplicationHandle->cfg.CurrentHeight);
            ApplicationHandle->cfg.Resized = true;
            return 0;

            // Mouse
        case WM_INPUT:
        {
            UINT dwSize;
            GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
            LPBYTE lpb = new BYTE[dwSize];
            if( lpb == NULL ) {
                return 0;
            }

            if( GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize )
                OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

            RAWINPUT* raw = (RAWINPUT*)lpb;

            if( raw->header.dwType == RIM_TYPEMOUSE ) {
                // Update mouse position
                POINT lp; GetCursorPos(&lp);
                ScreenToClient(hwnd, &lp);

                ApplicationHandle->gInput->GetMouse()->SetMouse(lp.x, lp.y);
                
                // Update buttons
                if( raw->data.mouse.ulButtons > (ULONG)0 ) 
                    ApplicationHandle->gInput->GetMouse()->SetState(raw->data.mouse.ulButtons);
            }

            if( raw->header.dwType == RIM_TYPEKEYBOARD ) {
                ApplicationHandle->gInput->GetKeyboard()->SetState(raw->data.keyboard.VKey, 
                                                                   !(raw->data.keyboard.Flags & RI_KEY_BREAK));
            }

            return DefWindowProc(hwnd, umessage, wparam, lparam);
        }
            
        /*case WM_LBUTTONDOWN:
            //ApplicationHandle->gInput->GetMouse()->SetState(MK_LBUTTON, true);
            //ApplicationHandle->gInput->GetMouse()->SetMouse(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;

        case WM_LBUTTONUP:
            //ApplicationHandle->gInput->GetMouse()->SetState(MK_LBUTTON, false);
            //ApplicationHandle->gInput->GetMouse()->SetMouse(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;

        case WM_RBUTTONDOWN:
            ApplicationHandle->gInput->GetMouse()->SetState(MK_RBUTTON, true);
            ApplicationHandle->gInput->GetMouse()->SetMouse(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;

        case WM_RBUTTONUP:
            ApplicationHandle->gInput->GetMouse()->SetState(MK_RBUTTON, false);
            ApplicationHandle->gInput->GetMouse()->SetMouse(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;

        case WM_MBUTTONDOWN:
            ApplicationHandle->gInput->GetMouse()->SetState(MK_MBUTTON, true);
            ApplicationHandle->gInput->GetMouse()->SetMouse(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;

        case WM_MBUTTONUP:
            ApplicationHandle->gInput->GetMouse()->SetState(MK_MBUTTON, false);
            ApplicationHandle->gInput->GetMouse()->SetMouse(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;

        case WM_MOUSEMOVE:
            ApplicationHandle->gInput->GetMouse()->SetMouse(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            return 0;*/

        // All other messages pass to the message handler in the system class.
        default:
        {
            return /*ApplicationHandle->MessageHandler*/DefWindowProc(hwnd, umessage, wparam, lparam);
        }
    }
}
