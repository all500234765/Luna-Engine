#pragma once

#include "pc.h"
#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Extensions/Safe.h"
#include "Engine/DirectX/Query.h"

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

static const uint gRangeProfilerStackSize = 16u;

class RangeProfilerGPU {
protected:
    static std::array<Query*, gRangeProfilerStackSize> gQStackDisjoint;
    static std::array<Query*, gRangeProfilerStackSize> gQStackTimestampBegin;
    static std::array<Query*, gRangeProfilerStackSize> gQStackTimestampEnd;
    static std::array<UINT64, gRangeProfilerStackSize> gStackTime;
    static uint gStackPos;

public:
    static void Init() {
        for( uint i = 0; i < gRangeProfilerStackSize; i++ ) {
            Query *q0 = new Query();
            Query *q1 = new Query();
            Query *q2 = new Query();

            q0->Create(D3D11_QUERY_TIMESTAMP_DISJOINT);
            q1->Create(D3D11_QUERY_TIMESTAMP);
            q2->Create(D3D11_QUERY_TIMESTAMP);

            gQStackDisjoint      [i] = q0;
            gQStackTimestampBegin[i] = q1;
            gQStackTimestampEnd  [i] = q2;
        }
    }

    static void Release() {
        for( uint i = 0; i < gRangeProfilerStackSize; i++ ) {
            SAFE_RELEASE(gQStackDisjoint      [i]);
            SAFE_RELEASE(gQStackTimestampBegin[i]);
            SAFE_RELEASE(gQStackTimestampEnd  [i]);
        }
    }

    static void Begin() {
        if( gStackPos >= gRangeProfilerStackSize - 1 ) {
            printf_s("[RangeProfilerGPU::Begin]: Stack range exceeded!\n");
            return;
        }

        gQStackDisjoint[gStackPos]->Begin();
        gStackTime[gStackPos] = (UINT64)gQStackTimestampBegin[gStackPos]->End().iLastValue;

        gStackPos++;
    }

    static void End() {
        if( gStackPos >= gRangeProfilerStackSize - 1 ) {
            printf_s("[RangeProfilerGPU::End]: Stack range exceeded!\n");
        }

        gStackPos--;

        UINT delta;
        
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT* a = gQStackDisjoint[gStackPos]->End().tLastValue;
        if( a->Disjoint ) { return; }

        UINT64 b = (UINT64)gQStackTimestampEnd[gStackPos]->End().iLastValue - gStackTime[gStackPos];
        gStackTime[gStackPos] = (float)b / (float)a->Frequency * 1000.f;
        //printf_s("[GPU Time]: %f\n", gStackTime[gStackPos]);
    }

    // TODO: Add stack pos checks
    static UINT GetTime() { return gStackTime[gStackPos]; }
};
