#pragma once

#include "pc.h"
#include "PipelineState.h"
#include "Engine/DirectX/DirectX.h"

extern _DirectX* gDirectX;

using Topology = D3D_PRIMITIVE_TOPOLOGY;

class TopologyState: public PipelineState<TopologyState> {
private:
    Topology mTopology;

public:
    void Create(Topology t) { mTopology = t; }

    void Bind() { gDirectX->gContext->IASetPrimitiveTopology(mTopology); }
    void Bind(Topology topology) { gDirectX->gContext->IASetPrimitiveTopology(topology); mTopology = topology; }

    Topology Current() { return mTopology; }
};

// Static
class STopologyState {
protected:
    static Topology gState;
    static Topology gStateOld;

public:
    static Topology Current();
    static void Push();
    static void Pop();

    static void Bind(Topology T);
};
