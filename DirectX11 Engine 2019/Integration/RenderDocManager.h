#pragma once

#include "pc.h"

// RenderDoc
#define RENDERDOC_NO_STDINT
#define RENDERDOC_PLATFORM_WIN32
#include "RenderDoc/app/renderdoc_app.h"
#include "RenderDoc/replay/renderdoc_replay.h"

class RenderDocManager {
public:
    RenderDocManager(HWND p_Handle, LPCSTR pCapturePath, bool bRenderDoc);
    ~RenderDocManager(void);
    void StartFrameCapture();
    void EndFrameCapture();
    void LaunchUI();
    bool GetUI();

private:
    RENDERDOC_API_1_4_0 *m_API = NULL;
    RENDERDOC_WindowHandle m_WndHandle;
    HINSTANCE m_RenderDocDLL;
    UINT32 m_SocketPort;
    HWND m_Handle;
    bool m_CaptureStarted;
    bool mUI;

    // General
    pRENDERDOC_GetAPI m_RenderDocGetAPI;

    void* GetRenderDocFunctionPointer(HINSTANCE ModuleHandle, LPCSTR FunctionName);
};
