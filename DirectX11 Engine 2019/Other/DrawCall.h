#pragma once

#include "pc.h"
#include "Engine/DirectX/DirectX.h"

extern _DirectX *gDirectX;
extern UINT gDrawCallCount;
extern UINT gDispatchCallCount;
extern UINT gDrawCallInstanceCount; // Total number of instanced instances (kek)

void DXDraw(UINT VertexCount, UINT StartVertexLocation);
void DXDrawAuto();
void DXDrawIndexed(UINT IndexCount, UINT StartIndexLocation, UINT BaseVertexLocation);
void DXDrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, UINT BaseVertexLocation, UINT StartInstanceLocation);
void DXDrawIndexedInstancedIndirect(ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs);
void DXDrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
void DXDrawInstancedIndirect(ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs);
void DXDispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);
void DXDispatchIndirect(ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs);
