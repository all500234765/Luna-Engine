#pragma once

#include "pc.h"

#include "UIManager.h"

class UIPrimitive: UIManager {
protected:
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
        gVertexLayer[gLayerID].push_back(v);

        Notify();
    }

    static void AddVertex(const std::vector<UIVertex>& vert) {
        for( const UIVertex& v : vert ) AddVertex(v);
    }

    template<size_t Size>
    static void AddVertex(const std::array<UIVertex, Size>& vert) {
        for( const UIVertex& v : vert ) AddVertex(v);
    }


public:
    static void SetLayer(uint32_t layer) { gLayerID = std::clamp(layer, 1u, gMaxLayers); }
    static void SetColor(float4 color)   { gColor = color; }

    static uint32_t GetLayer() { return gLayerID; }
    static float4 GetColor() { return gColor; }
};
