#include "Mesh.h"

Mesh::Mesh() {
    //vb = new VertexBuffer();
    //ib = new IndexBuffer();
}

void Mesh::Bind() {
    if( vb ) vb->BindVertex(0);
    if( ib ) ib->BindIndex(0, DXGI_FORMAT_R32_UINT);
}

void Mesh::Render() {
    if( ib ) {
        gDirectX->gContext->DrawIndexed(ib->GetNumber(), 0, 0);
    } else if( vb ) {
        gDirectX->gContext->Draw(vb->GetNumber(), 0);
    }
}

void Mesh::Render(UINT num) {
    if( ib ) {
        gDirectX->gContext->DrawIndexedInstanced(ib->GetNumber(), num, 0, 0, 0);
    } else if( vb ) {
        gDirectX->gContext->DrawInstanced(vb->GetNumber(), num, 0, 0);
    }
}

void Mesh::SetBuffer(VertexBuffer* inVB, IndexBuffer* inIB) {
    vb = inVB;
    ib = inIB;
}

void Mesh::Release() {
    if( vb ) vb->Release();
    if( ib ) ib->Release();
}
