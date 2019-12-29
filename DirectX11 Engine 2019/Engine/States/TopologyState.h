#pragma once

#include "pc.h"
#include "PipelineState.h"

typedef D3D_PRIMITIVE_TOPOLOGY Topology;

class TopologyState: public PipelineState<TopologyState> {
private:
    Topology mTopology;

public:
    void Create(Topology t) { mTopology = t; }

    void Bind() { gDirectX->gContext->IASetPrimitiveTopology(mTopology); }
    void Bind(Topology topology) { gDirectX->gContext->IASetPrimitiveTopology(topology); mTopology = topology; }

    Topology Current() { return mTopology; }
};
