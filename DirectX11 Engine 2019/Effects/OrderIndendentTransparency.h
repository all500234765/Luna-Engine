#pragma once

#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/DirectX/Shader.h"
#include "Engine/Materials/Texture.h"
#include "Engine/Utility/Utils.h"
#include "Engine/Profiler/ScopedRangeProfiler.h"
#include "Engine Includes/Types.h"
#include <iostream>
#include <vector>

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
        for( auto e : gResizables ) {
            e->Resize(Width, Height);
        }
    }
};

class OrderIndendentTransparency: public CallbackResize, EffectBase {
private:
    Shader *shCreateLinkedLists;

#include "Shaders/Common/OITCommon.h"

    


public:
    OrderIndendentTransparency() {
        EffectBase::AddRef(this);
        
        uint32_t Width = 1366;
        uint32_t Height = 768;

        rwListHead = Texture(Width, Height, DXGI_FORMAT_R32_UINT, true);
        //sbLinkedLists = CounterStructuredBuffer<ListItem>();
        sbLinkedLists.CreateDefault(MAX_ELEMENTS * Width * Height, nullptr, true);

    }

    ~OrderIndendentTransparency() {

    }


    void Resize(UINT Width, UINT Height) override {
        sbLinkedLists.Release();
        sbLinkedLists.CreateDefault(MAX_ELEMENTS * Width * Height, nullptr, true);

        rwListHead.Resize(Width, Height);
    }
    
    template<size_t dim, size_t BufferNum, bool DepthBuffer=false,
             size_t ArraySize=1,  /* if Cube == true  => specify how many cubemaps
                                                         to create per RT buffer   */
             bool WillHaveMSAA=false, bool Cube=false>
    void Begin(RenderTarget<dim, BufferNum, DepthBuffer, ArraySize, WillHaveMSAA, Cube> *RB) {



    }

};
