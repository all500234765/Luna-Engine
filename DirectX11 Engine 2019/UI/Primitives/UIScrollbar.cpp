#include "pc.h"

#include "UIScrollbar.h"
#include "UIRectangle.h"

std::array<std::array<float2, UIManager::gMaxContainers>, UIManager::gMaxLayers>          UIScrollbar::gScrollbarPosition{};
std::array<std::array<UIScrollbarType, UIManager::gMaxContainers>, UIManager::gMaxLayers> UIScrollbar::gScrollbarType{};
std::array<std::array<float2, UIManager::gMaxContainers>, UIManager::gMaxLayers>          UIScrollbar::gLastAreaSize{};

UIScrollbar::UIScrollbar(UIScrollbarType Type) {
    // Just to be sure
    mType = 0u;
    size = {};
    pos = {};

    // 
    uint32_t LID = UIPrimitive::gLayerID;
    uint32_t SID = std::max((int)gContainerStackIDLayer[gLayerID] - 1, 0);
    gScrollbarType[LID][SID] = Type;

    const UIContainer* Container = gContainerStackLayer[LID][SID];

    // Set positions
    x = Container->Size.x;
    y = Container->Size.y;

    uint32_t t = (uint32_t)Type;
    if( t & (uint32_t)UIScrollbarType::Vertical ) {
        // Get dimensions
        //gLastAreaSize[LID][SID].y -= gScrollbarContentSize[LID][SID].y;
        pos.y = gScrollbarPosition[LID][SID].y;

        // Only if we exceed container size
        //if( pos.y >= 0.f ) 
        {
            /*float content = gScrollbarContentSize[gLayerID][SID][0].y - gContainerOffsetLayer[LID].y;
            float scroll_area = y;

            // Scrollbar size
            size.y = (scroll_area * scroll_area) / content;*/
            
            // Move
            //gContainerOffsetLayer[LID].y -= pos.y;
            //gContainerStackLayer[LID][SID]->Offset.y -= pos.y;
            mType |= (uint32_t)UIScrollbarType::Vertical;
        }
    }
}

UIScrollbar::~UIScrollbar() {
    uint32_t LID = UIPrimitive::gLayerID;
    uint32_t SID = std::max((int)gContainerStackIDLayer[gLayerID] - 1, 0);

    // Draw
    if( mType != 0u ) UIPrimitive::SetColor(LunaEngine::Math::normrgba({ 97.f, 97.f, 97.f, 255.f }));
    if( mType & (uint32_t)UIScrollbarType::Vertical ) {
        float content = gScrollbarContentSize[gLayerID][SID][0].y - gContainerOffsetLayer[LID].y;
        float scroll_area = y;

        // Scrollbar size
        float pc = scroll_area / content;
        size.y = scroll_area * pc;

        if( size.y > 0.f && content > scroll_area ) {
            UIScrollbarState* state = gScrollbarState[LID][SID][0];
            float3 off = UIManager::GetOffset();
            //pos.y /= jmp;
            float y1 = (pos.y * pc) + pos.y; // Compensate offset. Move
            float y2 = y1 + size.y;

            float x2 = off.x + x;
            float x1 = x2 - 12.f;

            // Unconvert pos
            pos.y *= pc;


            // Controls
            if( gMouse->InRect(x1, off.y, x2, off.y + scroll_area) ) {
                // In scrollbar area
                if( gMouse->IsPressed(MouseButton::Left) ) {
                    printf_s("%f\n", content);
                    if( gMouse->InRect(x1, off.y + y1 - pos.y, off.x + x, off.y + y2 - pos.y) ) {
                        // Thumb
                        state->start = gMouse->GetY();
                        state->started = true;
                    } else {
                        // Outside of thumb
                        // TODO: 
                        printf_s("!-_!\n");
                        state->start = gMouse->GetY();
                        state->started = true;
                        pos.y = state->start - off.y - 0*size.y * .5f;
                    }
                }
            }

            if( state->started && gMouse->IsDown(MouseButton::Left) ) {
                float delta = gMouse->GetY() - state->start;
                state->start = gMouse->GetY();

                pos.y += delta;
                pos.y = std::clamp(pos.y, 0.f, scroll_area - size.y);
            }

            if( gMouse->IsReleased(MouseButton::Left) ) {
                state->started = false;
            }

            // Add to draw queue
            UIRectangle r(x - 12.f, y1, x, y2);

            // Save state & convert
            gScrollbarPosition[LID][SID].y = pos.y / pc;// *jmp;
        } else {
            gScrollbarPosition[LID][SID].y = 0.f;
        }
    } else {
        gScrollbarPosition[LID][SID].y = 0.f;
    }

    if( mType & (uint32_t)UIScrollbarType::Horizontal ) {
        //UIRectangle r(x - 12.f, pos.y, x, pos.y + size.y);
    }

    // Clean
    //if( mType & (uint32_t)UIScrollbarType::Vertical )
        //gContainerStackLayer[LID][SID]->Offset.y += pos.y;
        //gContainerOffsetLayer[LID].y += pos.y;
    
    //gScrollbarContentSize[LID][SID][0] = { -9999.f, -9999.f };
    gScrollbarType[LID][SID] = UIScrollbarType::None;
}
