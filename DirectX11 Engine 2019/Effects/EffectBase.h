#pragma once

#include "pc.h"
#include "Engine/Utility/Utils.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Extensions/Safe.h"

#include "Engine Includes/Types.h"

class CallbackResize {
public:
    virtual void Resize(UINT Width, UINT Height) = 0;
};

class EffectBase {
protected:
    static void AddRef(CallbackResize* self) {
        gResizables.push_back(self);
    }

private:
    static std::vector<CallbackResize*> gResizables;

public:

    static void ResizeGlobal(UINT Width, UINT Height) {
        for( CallbackResize* e : gResizables ) {
            e->Resize(Width, Height);
        }
    }
};
