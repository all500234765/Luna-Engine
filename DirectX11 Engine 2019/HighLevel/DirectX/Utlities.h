#pragma once

#include "Engine/DirectX/DirectX.h"

extern _DirectX *gDirectX;

namespace LunaEngine {
    // Discard items(SRV, UAV, CB) from shader slots
#pragma region Compute Shader
    template<UINT dim>
    void CSDiscardUAV() {
        ID3D11UnorderedAccessView *pEmpty[dim] = nullptr;
        gDirectX->gContext->CSSetUnorderedAccessViews(0, dim, pEmpty, 0);
    }

    template<>
    void CSDiscardUAV<1>() {
        ID3D11UnorderedAccessView *pEmpty = nullptr;
        gDirectX->gContext->CSSetUnorderedAccessViews(0, 1, &pEmpty, 0);
    }

    template<UINT dim>
    void CSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->CSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void CSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->CSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void CSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->CSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void CSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->CSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Vertex Shader
    template<UINT dim>
    void VSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->VSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void VSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->VSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void VSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->VSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void VSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->VSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Pixel Shader
    template<UINT dim>
    void PSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->PSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void PSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->PSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void PSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->PSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void PSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->PSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Geometry Shader
    template<UINT dim>
    void GSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->GSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void GSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->GSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void GSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->GSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void GSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->GSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Hull Shader
    template<UINT dim>
    void HSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->HSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void HSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->HSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void HSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->HSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void HSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->HSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion

#pragma region Domain Shader
    template<UINT dim>
    void DSDiscardCB() {
        ID3D11Buffer *pEmpty[dim] = nullptr;
        gDirectX->gContext->DSSetConstantBuffers(0, dim, pEmpty, 0);
    }

    template<>
    void DSDiscardCB<1>() {
        ID3D11Buffer *pEmpty = nullptr;
        gDirectX->gContext->DSSetConstantBuffers(0, 1, &pEmpty);
    }

    template<UINT dim>
    void DSDiscardSRV() {
        ID3D11ShaderResourceView *pEmpty[dim] = nullptr;
        gDirectX->gContext->DSSetShaderResources(0, dim, pEmpty, 0);
    }

    template<>
    void DSDiscardSRV<1>() {
        ID3D11ShaderResourceView *pEmpty = nullptr;
        gDirectX->gContext->DSSetShaderResources(0, 1, &pEmpty);
    }
#pragma endregion
    
    

};
