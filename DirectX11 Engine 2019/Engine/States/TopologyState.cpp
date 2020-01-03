#include "pc.h"
#include "TopologyState.h"

Topology STopologyState::gState    = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
Topology STopologyState::gStateOld = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

void STopologyState::Bind(Topology T) {
    gDirectX->gContext->IASetPrimitiveTopology(T);
    gState = T;
}

Topology STopologyState::Current() { return gState;                   }
void STopologyState::Push()        { gStateOld = gState;              } // Store current state
void STopologyState::Pop()         { if( gStateOld ) Bind(gStateOld); } // Re-Store old state
