#pragma once

#include "pc.h"

#include "../UIManager.h"
#include "../UIPrimitive.h"

enum class UIScrollbarType {
    None        = 0, 
    Vertical    = 1, 
    Horizontal  = 2, 
    Both        = 3
};

class UIScrollbar: public UIPrimitive {
protected:
    friend class UIPrimitive;

    static std::array<std::array<float2, UIManager::gMaxContainers>, UIManager::gMaxLayers> gLastAreaSize;
    static std::array<std::array<float2, UIManager::gMaxContainers>, UIManager::gMaxLayers> gScrollbarPosition;
    static std::array<std::array<UIScrollbarType, UIManager::gMaxContainers>, UIManager::gMaxLayers> gScrollbarType;

private:
    float2 pos{}, size{};
    float x{}, y{};
    uint32_t mType = 0u;

public:
    UIScrollbar(UIScrollbarType Type=UIScrollbarType::None);
    ~UIScrollbar();


};

