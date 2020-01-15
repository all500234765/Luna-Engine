#include "pc.h"
#include "RenderDocManager.h"

RenderDocManager::RenderDocManager(HWND p_Handle, LPCSTR pCapturePath, bool bRenderDoc) {
    if( !bRenderDoc ) { return; }
    m_Handle = p_Handle;
    m_CaptureStarted = false;

    m_RenderDocDLL = LoadLibrary(L"renderdoc.dll");
    if( !m_RenderDocDLL ) {
        printf_s("[RenderDoc]: renderdoc.dll not found or loaded! %d\n", GetLastError());
        FreeLibrary(m_RenderDocDLL);
        m_RenderDocDLL = 0;
        return;
    }

    // Init function pointers
    m_RenderDocGetAPI = (pRENDERDOC_GetAPI)GetRenderDocFunctionPointer(m_RenderDocDLL, "RENDERDOC_GetAPI");

    if( !m_RenderDocGetAPI ) {
        printf_s("[RenderDoc]: Error occured while capturing pointers inside renderdoc.dll. %d\n", GetLastError());
        FreeLibrary(m_RenderDocDLL);
        m_RenderDocDLL = 0;
        return;
    }

    if( m_RenderDocGetAPI(eRENDERDOC_API_Version_1_4_0, (void**)&m_API) == 0 ) {
        printf_s("[RenderDoc]: API initialization failed (1.4.0)!\n");
        FreeLibrary(m_RenderDocDLL);
        m_RenderDocDLL = 0;
        return;
    }

    int major, minor, patch;
    m_API->GetAPIVersion(&major, &minor, &patch);
    printf_s("[RenderDoc]: Loaded API %d.%d.%d\n", major, minor, patch);

    // Set default settings
    m_API->SetCaptureOptionU32(eRENDERDOC_Option_AllowFullscreen, 1u);
    m_API->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync     , 1u);

    // 
    RENDERDOC_InputButton ckey = eRENDERDOC_Key_F11;
    m_API->SetCaptureKeys(&ckey, 1);
    m_API->SetCaptureFilePathTemplate(pCapturePath);

    mUI = false;
}

void RenderDocManager::StartFrameCapture() {
    if( !m_RenderDocDLL ) return;
    m_API->StartFrameCapture(m_Handle, m_WndHandle);
    m_CaptureStarted = true;
}

// In some cases a capture can fail. It happens when Map() was called before the StartFrameCapture() and then Unmap() is called.
// It also happen if you start recording a command list before the StartFrameCapture() (unless you have the option
// CaptureAllCmdLists enabled).
// In these cases, m_RenderDocEndFrameCapture will return false and start capturing again until a capture succeed, unless 
// m_RenderDocEndFrameCapture is called again.
void RenderDocManager::EndFrameCapture() {
    if( !m_RenderDocDLL ) return;
    if( !m_CaptureStarted )
        return;

    if( m_API->EndFrameCapture(m_Handle, m_WndHandle) ) {
        m_CaptureStarted = false;
        return;
    }

    printf_s("[RenderDoc]: Capture failed!\n");

    // The capture has failed, calling m_RenderDocEndFrameCapture several time to make sure it won't keep capturing forever.
    while( !m_API->EndFrameCapture(m_Handle, m_WndHandle) ) {}

    m_CaptureStarted = false;
    return;
}

RenderDocManager::~RenderDocManager(void) {
    if( !m_RenderDocDLL ) return;
    FreeLibrary(m_RenderDocDLL);
}

void* RenderDocManager::GetRenderDocFunctionPointer(HINSTANCE ModuleHandle, LPCSTR FunctionName) {
    if( !m_RenderDocDLL ) return NULL;
    void* OutTarget = NULL;
    OutTarget = (void*)GetProcAddress(ModuleHandle, FunctionName);

    return OutTarget;
}

void RenderDocManager::LaunchUI() {
    if( !m_RenderDocDLL ) return;
    m_API->LaunchReplayUI(0, "");
}

bool RenderDocManager::GetUI() {
    if( !m_RenderDocDLL ) return false;
    return mUI;
}
