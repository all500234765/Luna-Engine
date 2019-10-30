#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <D3Dcompiler.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <initializer_list>

#include "DirectXChild.h"
#include "Engine/States/PipelineState.h"

class PolygonLayout;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

static const char* gcShaderName[] = {
        "Vertex", "Pixel", "Geometry",
        "Hull", "Domain", "Compute",
        "Geometry With Stream Output"
};

class Shader: public PipelineState<Shader> {
private:
    ID3D11VertexShader   *sVertex;
    ID3D11PixelShader    *sPixel;
    ID3D11GeometryShader *sGeometry;
    ID3D11HullShader     *sHull;
    ID3D11DomainShader   *sDomain;
    ID3D11ComputeShader  *sCompute;

    ID3DBlob *bShader[6];

    PolygonLayout *pl;

    // Original shader / Loaded shaders
    uint8_t Type = 0;

    // Linked shaders from different shaders
    uint8_t Linked = 0;

    // Shader slots that won't be bound to null
    // If this shader doesn't have any
    uint8_t DTouch = 0;

public:
    typedef enum {
        Vertex     = 1, 
        Pixel      = 2, 
        Geometry   = 4, 
        Hull       = 8, 
        Domain     = 16, 
        Compute    = 32, 
        GeometrySO = 64, 
        Count      = 7
    } ShaderType;

    Shader();

    void SetNullShader(ShaderType type);

    bool LoadFile(std::string fname, ShaderType type, 
                  D3D11_SO_DECLARATION_ENTRY* pSODecl=(D3D11_SO_DECLARATION_ENTRY*)0, 
                  UINT SODeclNum=0, UINT* Strides={0}, UINT NumStrides=0, UINT RStream=0);
    void DeleteShaders();

    void DontTouch(std::initializer_list<Shader::ShaderType> types);

    // > Can be called only after ReleaseBlobs
    // > To prevent >>>MEMORY LEAKS<<<
    void Remove(ShaderType type);
    void Reload(std::string fname, ShaderType type, 
                  D3D11_SO_DECLARATION_ENTRY* pSODecl=(D3D11_SO_DECLARATION_ENTRY*)0, 
                  UINT SODeclNum=0, UINT* Strides={0}, UINT NumStrides=0, UINT RStream=0);

    void ReleaseBlobs();
    ID3DBlob* GetBlob(ShaderType type);

    void AttachShader(Shader *origin, ShaderType type);

    void Bind();
    void Dispatch(UINT x=1, UINT y=1, UINT z=1);

    // TODO: Make something like that
    HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3D11InputLayout** pInputLayout, int* inputLayoutByteLength);
    HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3D11InputLayout** pInputLayout, int* inputLayoutByteLength, std::vector<D3D11_INPUT_ELEMENT_DESC> elements);
    bool CreatePolygonLayout();
    bool CreatePolygonLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> elements);

    void Release() {
        DeleteShaders();
    }
};
