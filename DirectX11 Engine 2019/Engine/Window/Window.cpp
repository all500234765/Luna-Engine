#include "Window.h"

#include <iostream>

void Window::Create(WindowConfig* config) {
    WNDCLASSEX wc;
    int posX, posY;
    DEVMODE dmScreenSettings;

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
    wc.lpszClassName = config->Title;
    wc.cbSize = sizeof(WNDCLASSEX);

    // Load icon
    hIcon = (HICON)LoadImage(NULL, config->Icon, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE | LR_SHARED);
    if( hIcon ) {
        wc.hIconSm = hIcon;
        wc.hIcon   = hIcon;
    } else {
        std::wstring w = config->Icon;
        std::cout << "Failed to load icon. (" << std::string(w.begin(), w.end()).c_str() << ")" << std::endl;
    }

    // Register the window class.
    RegisterClassEx(&wc);

    // Determine the resolution of the clients desktop screen.
    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Mode
    if( config->Windowed ) {
        // If windowed then set it to custom resolution
        if( !config->Width  ) config->CurrentWidth  = screenWidth ; else config->CurrentWidth  = config->Width;
        if( !config->Height ) config->CurrentHeight = screenHeight; else config->CurrentHeight = config->Height;

        // Place the window in the middle of the screen.
        posX = max((GetSystemMetrics(SM_CXSCREEN) - config->CurrentWidth) / 2, 0);
        posY = max((GetSystemMetrics(SM_CYSCREEN) - config->CurrentHeight) / 2, 0);
    } else {
        // If full screen set the screen to maximum size of the users desktop and 32bit.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if( !config->Borderless ) {
            // Change the display settings to full screen.
            ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
        }

        config->CurrentWidth  = screenWidth;
        config->CurrentHeight = screenHeight;

        // Set the position of the window to the top left corner.
        posX = posY = 0;
    }

    // Window flags
    unsigned int flags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    // Borderless flag
    if( config->Borderless ) flags |= WS_POPUP;
    else                     flags |= WS_OVERLAPPEDWINDOW;

    // Adjust window size and position
    //RECT rect = {posX, posY, posX + config->CurrentWidth, posY + config->CurrentHeight};
    //AdjustWindowRectEx(&rect, flags, false, WS_EX_APPWINDOW);

    //posX = rect.left;
    //posY = rect.top;
    //config->CurrentWidth  = rect.right  - posX;
    //config->CurrentHeight = rect.bottom - posY;

    // Create the window with the screen settings and get the handle to it.
    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, config->Title, config->Title, flags,
                            posX, posY, config->CurrentWidth, config->CurrentHeight,
                            NULL, NULL, m_hinstance, NULL);

    // Bring the window up on the screen and set it as main focus.
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    // 
    std::cout << "New window(x=" << posX << ", y=" << posY << ", w=" << config->CurrentWidth << ", h=" << config->CurrentHeight << ")" << std::endl;

    // Save configuration
    cfg = *config;

    // Hide the mouse cursor.
    //ShowCursor(false);
}

void Window::Loop() {
    if( FrameFunction == nullptr ) {
        MessageBox(NULL, L"No frame function defined!", L"Error", MB_OK);
    }

    MSG msg;

    // Initialize the message structure.
    ZeroMemory(&msg, sizeof(MSG));
    
    // Loop until there is a quit message from the window or the user.
    while( true ) {
        // Reset state
        cfg.Resized = false;

        // Handle the windows messages.
        if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // If windows signals to end the application then exit out.
        if( msg.message == WM_QUIT ) {
            break;
        } else {
            // Otherwise do the frame processing.
            if( gDirectX->FrameFunction() ) { break; }
        }
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

/*LRESULT CALLBACK Window::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    switch( umsg ) {
        // Check if a key has been pressed on the keyboard.
        case WM_KEYDOWN:
        {
            // If a key is pressed send it to the input object so it can record that state.
            m_Input->KeyDown((unsigned int)wparam);
            return 0;
        }

        // Check if a key has been released on the keyboard.
        case WM_KEYUP:
        {
            // If a key is released then send it to the input object so it can unset the state for that key.
            m_Input->KeyUp((unsigned int)wparam);
            return 0;
        }

        // Any other messages send to the default message handler as our application won't make use of them.
        default:
        {
            return DefWindowProc(hwnd, umsg, wparam, lparam);
        }
    }
}*/

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
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
            ApplicationHandle->cfg.CurrentWidth  = LOWORD(lparam);
            ApplicationHandle->cfg.CurrentHeight = HIWORD(lparam);
            ApplicationHandle->cfg.Resized = true;
            return 0;

        // All other messages pass to the message handler in the system class.
        default:
        {
            return /*ApplicationHandle->MessageHandler*/DefWindowProc(hwnd, umessage, wparam, lparam);
        }
    }
}
