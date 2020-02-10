#pragma once

#include "pc.h"

#include "UIManager.h"

class UIPrimitive: public UIManager {
protected:
    friend struct UIContainer;
    friend class UIManager;

    static uint32_t gLayerID;
    static float4 gColor;

    static void Notify() {
        // Add to the list if we haven't yet
        if( std::find(gLastActiveLayers.begin(), gLastActiveLayers.end(), gLayerID) == gLastActiveLayers.end() ) {
            gLastActiveLayers.push_back(gLayerID);
        }
    }

    static void AddVertex(const UIVertex& v) {
        gDCLayer[gLayerID]++;

        UIVertex v1(v);
        v1.Position.z++;
        gVertexLayer[gLayerID].push_back(v1);

        uint32_t SID = gContainerStackIDLayer[gLayerID] - 1;
        Notify();
    }

    static const UIVertex& Advance(const UIVertex& v) {
        UIVertex v1(v);
        v1.Position.x += gContainerOffsetLayer[gLayerID].x;
        v1.Position.y += gContainerOffsetLayer[gLayerID].y;
        v1.Position.z += gContainerOffsetLayer[gLayerID].z;

        return v1;
    }

    static const UIVertex& AdvanceScroll(const UIVertex& v, uint32_t SID);

    static void AddTriangle(const UIVertex& v0, const UIVertex& v1, const UIVertex& v2) {
        // Advance
        UIVertex v10 = Advance(v0);
        UIVertex v11 = Advance(v1);
        UIVertex v12 = Advance(v2);

        // Check, If we inside of the container
        uint32_t SID = gContainerStackIDLayer[gLayerID] - 1;
        const UIContainer* Container = gContainerStackLayer[gLayerID][SID];

        auto NotifyScroll = [&](const UIVertex& v20, const UIVertex& v21, const UIVertex& v22) {
            gScrollbarContentSize[gLayerID][SID][0] = LunaEngine::Math::max(gScrollbarContentSize[gLayerID][SID][0], float2({ v20.Position.x, v20.Position.y }));
            gScrollbarContentSize[gLayerID][SID][0] = LunaEngine::Math::max(gScrollbarContentSize[gLayerID][SID][0], float2({ v21.Position.x, v21.Position.y }));
            gScrollbarContentSize[gLayerID][SID][0] = LunaEngine::Math::max(gScrollbarContentSize[gLayerID][SID][0], float2({ v22.Position.x, v22.Position.y }));
        };

        if( Container->bActive ) {
            NotifyScroll(v10, v11, v12);

            v10 = AdvanceScroll(v10, SID);
            v11 = AdvanceScroll(v11, SID);
            v12 = AdvanceScroll(v12, SID);

            // If we outside of the container
            if( !Container->Inside(v10, v11, v12) ) {
                if( Container->AtleastInside(v10, v11, v12) ) {
                    // Add and clamp
                    v10 = Container->Clamp(v10);
                    v11 = Container->Clamp(v11);
                    v12 = Container->Clamp(v12);
                } else {
                    // Simply skip triangle
                    return;
                }
            }

            AddVertex(v10);
            AddVertex(v11);
            AddVertex(v12);
        } else {
            NotifyScroll(v0, v1, v2);

            v10 = AdvanceScroll(v10, SID);
            v11 = AdvanceScroll(v11, SID);
            v12 = AdvanceScroll(v12, SID);

            AddVertex(v0);
            AddVertex(v1);
            AddVertex(v2);
        }
    }

public:
    static void SetLayer(uint32_t layer) { gLayerID = std::clamp(layer, 0u, gMaxLayers); }
    static void SetColor(float4 color)   { gColor = color; }

    static uint32_t GetLayer() { return gLayerID; }
    static float4 GetColor() { return gColor; }
};
