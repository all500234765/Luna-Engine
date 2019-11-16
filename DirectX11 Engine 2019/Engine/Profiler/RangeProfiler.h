#pragma once

#include "Engine/DirectX/DirectXChild.h"

class RangeProfiler: public DirectXChild {
protected:
    static ID3DUserDefinedAnnotation *gUDAnnotation;
    static bool gInitialized;

private:


public:
    static void Init() {
        if( !gInitialized ) {
            gInitialized = true;
            gDirectX->gContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&gUDAnnotation);

            if( gUDAnnotation ) {
                static const char* lNames[] = { "disabled", "Launched under graphics debugging tool" };

                printf_s("[Profiling]: Successfully initialized UDA\n");
                printf_s("[Profiling]: Status: %s\n", lNames[GetStatus()]);
            } else {
                printf_s("[Profiling]: Failed to initialize UDA\n");
            }
        }
    }

    static void Release() {
        if( gInitialized && gUDAnnotation ) gUDAnnotation->Release();
    }

    static void Begin(const wchar_t* RangeName) {
        if( !gUDAnnotation ) return; // UDA interface is unavaliable
        if( !gUDAnnotation->GetStatus() ) return; // Not running under a profiling tool
        gUDAnnotation->BeginEvent(RangeName);
    }

    static void Begin(const char* RangeName) {
        if( !gUDAnnotation ) return; // UDA interface is unavaliable
        if( !gUDAnnotation->GetStatus() ) return; // Not running under a profiling tool
        
        // const char*  to  const wchar_t*
        wchar_t dest[255];
        size_t len = 0;
        mbstowcs_s(&len, dest, size_t(strlen(RangeName) + 1), RangeName, _TRUNCATE);

        // 
        gUDAnnotation->BeginEvent(dest);
    }

    static void End() {
        if( !gUDAnnotation ) return; // UDA interface is unavaliable
        if( !gUDAnnotation->GetStatus() ) return; // Not running under a profiling tool
        gUDAnnotation->EndEvent();
    }
    
    static bool GetStatus() {
        if( !gUDAnnotation ) return false; // UDA interface is unavaliable
        return gUDAnnotation->GetStatus();
    }

    static void Mark(const wchar_t* MarkName) {
        if( !gUDAnnotation ) return; // UDA interface is unavaliable

        gUDAnnotation->SetMarker(MarkName);
    }
};
