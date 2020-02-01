#pragma once

#include "pc.h"

#include "UIManager.h"

class UIPrimitive: UIManager {
protected:
    friend struct UIContainer;

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

        Notify();
    }

    static const UIVertex& Advance(const UIVertex& v) {
        UIVertex v1(v);
        v1.Position.x += gContainerOffsetLayer[gLayerID].x;
        v1.Position.y += gContainerOffsetLayer[gLayerID].y;
        v1.Position.z += gContainerOffsetLayer[gLayerID].z;

        return v1;
    }

    static void AddTriangle(const UIVertex& v0, const UIVertex& v1, const UIVertex& v2) {
        // Advance
        UIVertex v10 = Advance(v0);
        UIVertex v11 = Advance(v1);
        UIVertex v12 = Advance(v2);

        // Check, If we inside of the container
        const UIContainer* Container = gContainerStackLayer[gLayerID][gContainerStackIDLayer[gLayerID] - 1];
        if( Container->bActive ) {
            // If we outside of the container
            if( !Container->Inside(v10, v11, v12) ) {
                // Simply skip triangle
                return;
            } else if( Container->AtleastInside(v10, v11, v12) ) {
                // Add and clamp
                AddVertex(Container->Clamp(v10));
                AddVertex(Container->Clamp(v11));
                AddVertex(Container->Clamp(v12));
            }
        } else {
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
