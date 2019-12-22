#include "DrawCall.h"

UINT gDrawCallCount = 0;
UINT gDispatchCallCount = 0;
UINT gDrawCallInstanceCount = 0; // Total number of instanced instances (kek)

void DXDraw(UINT VertexCount, UINT StartVertexLocation) {
    gDirectX->gContext->Draw(VertexCount, StartVertexLocation);
    gDrawCallCount++;
}

void DXDrawAuto() {
    gDirectX->gContext->DrawAuto();
    gDrawCallCount++;
}

void DXDrawIndexed(UINT IndexCount, UINT StartIndexLocation, UINT BaseVertexLocation) {
    gDirectX->gContext->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
    gDrawCallCount++;
}

void DXDrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, UINT BaseVertexLocation, UINT StartInstanceLocation) {
    gDirectX->gContext->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    gDrawCallInstanceCount += InstanceCount;
    gDrawCallCount++;
}

void DXDrawIndexedInstancedIndirect(ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs) {
    gDirectX->gContext->DrawIndexedInstancedIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
    //gDrawCallInstanceCount += 0; // TODO: ?
    gDrawCallCount++;
}

void DXDrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {
    gDirectX->gContext->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
    gDrawCallInstanceCount += InstanceCount;
    gDrawCallCount++;
}

void DXDrawInstancedIndirect(ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs) {
    gDirectX->gContext->DrawInstancedIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
    //gDrawCallInstanceCount += 0; // TODO: ?
    gDrawCallCount++;
}

void DXDispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) {
    gDirectX->gContext->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    gDispatchCallCount++;
}

void DXDispatchIndirect(ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs) {
    gDirectX->gContext->DispatchIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
    gDispatchCallCount++;
}
