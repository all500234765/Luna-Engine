#include "pc.h"

#include "UIPrimitive.h"
#include "Primitives/UIScrollbar.h"

uint32_t UIPrimitive::gLayerID = 0u;
float4 UIPrimitive::gColor = { 1.f, 1.f, 1.f, 1.f };

const UIVertex& UIPrimitive::AdvanceScroll(const UIVertex& v, uint32_t SID) {
    UIVertex v1(v);

    uint32_t type = (uint32_t)UIScrollbar::gScrollbarType[gLayerID][SID];
    if( type & (uint32_t)UIScrollbarType::Horizontal ) 
        v1.Position.x -= UIScrollbar::gScrollbarPosition[gLayerID][SID].x;
    
    if( type & (uint32_t)UIScrollbarType::Vertical )
        v1.Position.y -= UIScrollbar::gScrollbarPosition[gLayerID][SID].y;

    return v1;
}
