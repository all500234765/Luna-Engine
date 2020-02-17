#pragma once

#include "pc.h"

#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/Scene/Texture.h"

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

class UIAtlas {
protected:
    static StructuredBuffer<UIAtlasItemGPU> sbAtlasItems;
    static std::vector<UIAtlasItem*>        gAtlasItems;
    static Texture*                         gAtlasTexture;

    static float gCurrentX;
    static float gCurrentY;
    static float gUsedArea;
    static float gShelfHeight;

    using UIItemList = std::vector<UIAtlasItem*>;

private:
    static UIItemList BucketSort(UIItemList list, uint32_t bucket_num);
    static void InternalUpdateSingle(UIAtlasItem* item);

public:
    static void Init(uint32_t width, uint32_t height, uint32_t array_size=1u);
    static void Release();

    static UIAtlasItem* Insert(Texture* texture);
    static UIAtlasItem* Insert(const char* file);
    static void Remove(const UIAtlasItem* item);

    static void Update();

    // Compute ratio of used surface area.
    static inline float Occupancy() { return (float)gUsedArea / (gAtlasTexture->GetWidth() * gAtlasTexture->GetHeight()); }

    static inline Texture* GetTexture() { return gAtlasTexture; }

    // Getters
    static inline float GetWidth() { return gAtlasTexture->GetWidth(); }
    static inline float GetHeight() { return gAtlasTexture->GetHeight(); }

};
