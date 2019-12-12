#pragma once

#include "PipelineState.h"

#include "Engine/Singleton.h"

typedef D3D_PRIMITIVE_TOPOLOGY Topology;

class TopologyState: public PipelineState<TopologyState> {
private:
    Topology mTopology;

public:
    void Bind() { gDirectX->gContext->IASetPrimitiveTopology(mTopology); }
    void Bind(Topology topology) { gDirectX->gContext->IASetPrimitiveTopology(topology); mTopology = topology; }

    Topology Current() { return mTopology; }
};
