#include "Mesh.h"

Mesh::Mesh() {
    vb = new VertexBuffer();
    ib = new IndexBuffer();
}

void Mesh::Bind() {
    vb->BindVertex(0);
    ib->BindIndex(0, DXGI_FORMAT_R32_UINT);
}

void Mesh::Render() {
    gDirectX->gContext->DrawIndexed(ib->GetNumber(), 0, 0);
}

void Mesh::Render(UINT num) {
    gDirectX->gContext->DrawIndexedInstanced(ib->GetNumber(), num, 0, 0, 0);
}

void Mesh::SetBuffer(VertexBuffer* inVB, IndexBuffer* inIB) {
    vb = inVB;
    ib = inIB;
}

void Mesh::Release() {
    if( vb ) vb->Release();
    if( ib ) ib->Release();
}
