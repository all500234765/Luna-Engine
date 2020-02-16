#include "pc.h"

#include "UIAtlas.h"

// Render
Texture*                      UIAtlas::gAtlasTexture{};
std::vector<UIAtlasItem*>     UIAtlas::gAtlasItems{};

// State
float UIAtlas::gCurrentX{};
float UIAtlas::gCurrentY{};
float UIAtlas::gUsedArea{};
float UIAtlas::gShelfHeight{};

UIAtlas::UIItemList UIAtlas::BucketSort(UIItemList list, uint32_t bucket_num) {
    using Item = UIAtlasItem*;

    UIItemList sorted;
    std::map<uint32_t, UIItemList> buckets;

    // Allocate memory
    sorted.reserve(list.size());
    buckets[bucket_num - 1] = {};
    for( uint32_t i = 0; i < bucket_num - 2; i++ ) buckets[i] = {};

    // Get max value
    auto fncmp = [](Item left, Item right) { return left->size() < right->size(); };
    uint32_t largest = (*std::max_element(std::begin(list), std::end(list), fncmp))->size();

    // Push to buckets
    for( int i = 0; i < list.size(); i++ ) {
        buckets[floor(bucket_num * list[i]->size() / largest)].push_back(list[i]);
        printf_s("list[%u]=%u -> list[%u]\n", i, list[i]->size(), (uint32_t)floor(bucket_num * list[i]->size() / largest));
    }

    // Sort
    auto fncmp2 = [](Item left, Item right) { return left->size() < right->size(); };
    for( int i = 0; i < bucket_num; i++ ) {
        std::sort(buckets[i].begin(), buckets[i].end(), fncmp2);

        // Concat values
        for( Item v : buckets[i] ) sorted.push_back(v);
    }

    // Reverse the order
    auto fncmp3 = [](Item left, Item right) { return left->size() > right->size(); };
    std::sort(sorted.begin(), sorted.end(), fncmp3);

    return sorted;
}

void UIAtlas::Init(uint32_t width, uint32_t height, uint32_t array_size) {
    gAtlasTexture = new Texture(tf_Array | tf_dim_2, DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1u, array_size, "UIAtlas");
}

void UIAtlas::Release() {
    SAFE_RELEASE(gAtlasTexture);

    for( UIAtlasItem* item : gAtlasItems ) {
        SAFE_RELEASE(item->texture);

        // TODO: Notify item users somehow
        item->usable = false;
        delete item;
    }

    gAtlasItems.clear();
}

UIAtlasItem* UIAtlas::Insert(Texture* texture) {
    UIAtlasItem* item = new UIAtlasItem{};
    item->texture = texture;
    item->usable  = false;

    gAtlasItems.push_back(item);

    return item;
}

UIAtlasItem* UIAtlas::Insert(const char* file) {
    return Insert(new Texture(file, 0u, file));
}

void UIAtlas::Update() {
    float gCurrentX{};
    float gCurrentY{};
    float gUsedArea{};
    float gShelfHeight{};

    implTexture* stg = gAtlasTexture->CreateStaging();

    BucketSort(gAtlasItems, 5);
    for( UIAtlasItem* item : gAtlasItems ) {
        InternalUpdateSingle(item);
        Texture::CopyS(stg, item->texture, item->x, item->y, 0, 0, 0, 0, 0);
    }

    gAtlasTexture->Copy(stg);
    SAFE_RELEASE(stg);
}

void UIAtlas::InternalUpdateSingle(UIAtlasItem* item) {
    float width = item->texture->GetWidth();
    float height = item->texture->GetHeight();

    // There are three cases:
    // 1. short edge <= long edge <= shelf height. Then store the long edge vertically.
    // 2. short edge <= shelf height <= long edge. Then store the short edge vertically.
    // 3. shelf height <= short edge <= long edge. Then store the short edge vertically.

    // If the long edge of the new rectangle fits vertically onto the current shelf,
    // flip it. If the short edge is larger than the current shelf height, store
    // the short edge vertically.
    if( ((width > height && width < gShelfHeight) ||
         (width < height && height > gShelfHeight)) ) {
        item->flipped = true;
        std::swap(width, height);
    } else {
        item->flipped = false;
    }

    if( gCurrentX + width > gAtlasTexture->GetWidth() ) {
        gCurrentX = 0;
        gCurrentY += gShelfHeight;
        gShelfHeight = 0;

        // When starting a new shelf, store the new long edge of the new rectangle horizontally
        // to minimize the new shelf height.
        if( width < height ) {
            std::swap(width, height);
            item->flipped ^= true;
        }
    }

    // If the rectangle doesn't fit in this orientation, try flipping.
    if( width > gAtlasTexture->GetWidth() || gCurrentY + height > gAtlasTexture->GetHeight() ) {
        std::swap(width, height);
        item->flipped ^= true;
    }

    // If flipping didn't help, return failure.
    if( width > gAtlasTexture->GetWidth() || gCurrentY + height > gAtlasTexture->GetHeight() ) {
        memset(&item, 0, sizeof(UIAtlasItem));
        item->usable = false;
        return;
    }

    item->w = width;
    item->h = height;
    item->x = gCurrentX;
    item->y = gCurrentY;
    item->usable = true;

    gCurrentX += width;
    gShelfHeight = std::max(gShelfHeight, height);

    gUsedArea += width * height;
}

