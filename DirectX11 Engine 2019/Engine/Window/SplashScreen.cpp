#include "SplashScreen.h"

HBITMAP SplashScreen::image = 0;
HINSTANCE SplashScreen::hinstance = 0;
UINT SplashScreen::timer_time = 10000; // Default is 10s

LRESULT CALLBACK SplashScreenWndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    PAINTSTRUCT ps;
    HDC         hdc;
    BITMAP      bitmap;
    HDC         hdcMem;
    HGDIOBJ     oldBitmap;

    switch( umessage ) {
        case WM_CREATE:
            SetTimer(hwnd, 1, SplashScreen::GetTimer(), NULL);
            return 0;

            // Check if the window is being destroyed.
        case WM_DESTROY:
        case WM_CLOSE:
        case WM_TIMER:
            DeleteObject(SplashScreen::GetImage());
            PostQuitMessage(0);
            KillTimer(hwnd, wparam);
            return 0;

        case WM_ERASEBKGND:
        case WM_PAINT:

            hdc = BeginPaint(hwnd, &ps);

            hdcMem = CreateCompatibleDC(hdc);
            oldBitmap = SelectObject(hdcMem, SplashScreen::GetImage());

            GetObject(SplashScreen::GetImage(), sizeof(bitmap), &bitmap);
            BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, oldBitmap);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            return 0;
    }

    return DefWindowProc(hwnd, umessage, wparam, lparam);
}

void SplashScreen::Launch(const wchar_t* fname, UINT time) {
    timer_time = time;

    // Load bmp
    image = (HBITMAP)LoadImage(NULL, fname, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    // Create window
    UINT flags = WS_POPUPWINDOW;

    // Get the instance of this application.
    hinstance = GetModuleHandle(NULL);

    // Setup the windows class with default settings.
    WNDCLASSEX wc{};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = SplashScreenWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinstance;
    wc.hIcon = NULL;
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"<s-s-o-s-a-i-e-n-t-e-r-t-a-i-m-e-n-t>";
    wc.cbSize = sizeof(WNDCLASSEX);

    // Register the window class.
    RegisterClassEx(&wc);

    // Get image size
    BITMAP bitmap;
    GetObject(image, sizeof(bitmap), (LPVOID)&bitmap);

    float Width = fabsf(bitmap.bmWidth);
    float Height = fabsf(bitmap.bmHeight);

    // Determine the resolution of the clients desktop screen.
    float screenWidth = GetSystemMetrics(SM_CXSCREEN);
    float screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Get window top left corner position
    float x = std::fmaxf(.5f * (screenWidth - Width), 0.f);
    float y = std::fmaxf(.5f * (screenHeight - Height), 0.f);

    // Calculate window rectangle
    RECT rect = { x, y, x + Width, y + Height };
    AdjustWindowRectEx(&rect, flags, false, WS_EX_APPWINDOW);

    Width = rect.right - rect.left;
    Height = rect.bottom - rect.top;

    // Create the window with the screen settings and get the handle to it.
    HWND m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, wc.lpszClassName, L"Luna Engine - Splash Screen",
                                 flags, x, y, Width, Height, NULL, NULL, hinstance, NULL);

    // Show as foreground
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    // Go for the timer
    MSG msg{};

    // Run loop
    while( true ) {
        // Handle the windows messages.
        while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // If windows signals to end the application then exit out.
            if( msg.message == WM_QUIT ) {
                return;
            }
        }
    }

    // Remove the window.
    DestroyWindow(m_hwnd);
    m_hwnd = NULL;

    // Remove the application instance.
    UnregisterClass(wc.lpszClassName, hinstance);
    hinstance = NULL;
}
