#pragma once

#include "pc.h"

#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/Scene/Texture.h"
#include "Engine/States/PipelineState.h"

struct UIAtlasItemGPU {
    float x, y, w, h;
};

struct UIAtlasItem: UIAtlasItemGPU {
    Texture* texture;

    inline float size() const { return w * h; }

    union {
        uint flags;
        struct {
            uint usable  : 1;
            uint flipped : 1;
        };
    };
};

class UIAtlas: public PipelineState<UIAtlas> {
protected:
    StructuredBuffer<UIAtlasItemGPU> sbAtlasItems{};
    std::vector<UIAtlasItem*>        gAtlasItems{};
    Texture*                         gAtlasTexture{};

    float gCurrentX{};
    float gCurrentY{};
    float gUsedArea{};
    float gShelfHeight{};

    using UIItemList = std::vector<UIAtlasItem*>;

private:
    UIItemList BucketSort(UIItemList list, uint32_t bucket_num);
    void InternalUpdateSingle(UIAtlasItem* item);

public:
    UIAtlas() { gState = this; }

    void Init(uint32_t width, uint32_t height, uint32_t array_size=1u);
    void Release();

    UIAtlasItem* Insert(Texture* texture);
    UIAtlasItem* Insert(const char* file);
    void Remove(UIAtlasItem* item);

    void Update();

    void Bind() { gState = this; }

    // Compute ratio of used surface area.
    inline float Occupancy() { return (float)gUsedArea / (gAtlasTexture->GetWidth() * gAtlasTexture->GetHeight()); }

    inline Texture* GetTexture() { return gAtlasTexture; }

    // Getters
    inline float GetWidth() { return gAtlasTexture->GetWidth(); }
    inline float GetHeight() { return gAtlasTexture->GetHeight(); }

};
