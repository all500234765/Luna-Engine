#include "Shader.h"
#include "PolygonLayout.h"

#include "Engine Includes/EngineDirectories.h"

#include <D3Dcompiler.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

Shader::Shader() {
    pl = new PolygonLayout();
}

void Shader::SetNullShader(ShaderType type) {
    // Create shader
    switch( type ) {
        case Vertex    : sVertex   = 0; break;
        case Pixel     : sPixel    = 0; break;
        case Geometry  : sGeometry = 0; break;
        case Hull      : sHull     = 0; break;
        case Domain    : sDomain   = 0; break;
        case Compute   : sCompute  = 0; break;
        case GeometrySO: sGeometry = 0; break;
    }

    // Shader was compiled natively
    Type |= type;
}

bool Shader::LoadFile(std::string fname, ShaderType type, D3D11_SO_DECLARATION_ENTRY* pSODecl, UINT SODeclNum, UINT* Strides, UINT NumStrides, UINT RStream) {
    // Create shader
    HRESULT hr;
    ID3DBlob *ShaderBuffer = 0;

    // 
    fname = SHADER_DIRECTORY + fname;

    // Read blob
    hr = D3DReadFileToBlob(std::wstring(fname.begin(), fname.end()).c_str(), &ShaderBuffer);
    if( FAILED(hr) ) {
        std::cout << "Can't read shader to blob! (" << type << ")" << std::endl;
        return 1;
    }

    // Create shader
    switch( type ) {
        case Vertex    : hr = gDirectX->gDevice->CreateVertexShader(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), NULL, &sVertex);       break;
        case Pixel     : hr = gDirectX->gDevice->CreatePixelShader(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), NULL, &sPixel);         break;
        case Geometry  : hr = gDirectX->gDevice->CreateGeometryShader(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), NULL, &sGeometry);   break;
        case Hull      : hr = gDirectX->gDevice->CreateHullShader(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), NULL, &sHull);           break;
        case Domain    : hr = gDirectX->gDevice->CreateDomainShader(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), NULL, &sDomain);       break;
        case Compute   : hr = gDirectX->gDevice->CreateComputeShader(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), NULL, &sCompute);     break;
        case GeometrySO: hr = gDirectX->gDevice->CreateGeometryShaderWithStreamOutput(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), 
                                                                                      pSODecl, SODeclNum, Strides, NumStrides, RStream, NULL, &sGeometry); break;
    }

    if( FAILED(hr) ) {
        ShaderBuffer->Release();
        ShaderBuffer = 0;
        std::cout << "Can't create shader! (" << gcShaderName[(int)log2((int)type)] << ")" << std::endl;
        return 1;
    }

    // Show message about succ loaded shader
    std::cout << "Successfully loaded " << fname << " file as " << gcShaderName[(int)log2((int)type)] << " shader type" << std::endl;

    // Shader was compiled natively
    Type |= type;

    // Store blob for futher use
    bShader[(type == GeometrySO) ? Geometry : (int)log2((int)type)] = ShaderBuffer;

    if( type == Vertex ) {
        CreatePolygonLayout();
    }

    // Done
    return 0;
}

#define __Has(x) ((Type & (x)) == (x))
#define __GSO    (__Has(Shader::Geometry) || __Has(Shader::GeometrySO))
#define __LNK(x) ((Linked & (x)) == 0)
#define __HLK(x) (((x == Shader::Geometry) ? (__Has(Shader::GeometrySO) && __LNK(Shader::GeometrySO)) : 0) || __Has(x) && __LNK(x))

void Shader::DeleteShaders() {
    // We can't release data that we don't own
    if( __HLK(Vertex  ) && sVertex   ) { sVertex->Release();   }
    if( __HLK(Pixel   ) && sPixel    ) { sPixel->Release();    }
    if( __HLK(Geometry) && sGeometry ) { sGeometry->Release(); }
    if( __HLK(Hull    ) && sHull     ) { sHull->Release();     }
    if( __HLK(Domain  ) && sDomain   ) { sDomain->Release();   }
    if( __HLK(Compute ) && sCompute  ) { sCompute->Release();  }

    // Release and delete polygon layout
    delete pl;
}

void Shader::DontTouch(std::initializer_list<Shader::ShaderType> types) {
    for( ShaderType type : types ) { DTouch |= type; }
}

void Shader::Remove(ShaderType type) {
    // Remove linked
    if( !__HLK(type) ) {
        switch( type ) {
            case Vertex  : sVertex   = nullptr; break;
            case Pixel   : sPixel    = nullptr; break;
            case Geometry: sGeometry = nullptr; break;
            case Hull    : sHull     = nullptr; break;
            case Domain  : sDomain   = nullptr; break;
            case Compute : sCompute  = nullptr; break;
        }

        Linked &= ~type;
        return;
    }

    // Remove loaded
    switch( type ) {
        case Vertex    : if( sVertex   ) sVertex->Release();   break;
        case Pixel     : if( sPixel    ) sPixel->Release();    break;
        case Geometry  :
        case GeometrySO: if( sGeometry ) sGeometry->Release(); break;
        case Hull      : if( sHull     ) sHull->Release();     break;
        case Domain    : if( sDomain   ) sDomain->Release();   break;
        case Compute   : if( sCompute  ) sCompute->Release();  break;
    }

    // Remove from bitfield
    Type &= ~type;
}

void Shader::Reload(std::string fname, ShaderType type, D3D11_SO_DECLARATION_ENTRY* pSODecl, UINT SODeclNum, UINT* Strides, UINT NumStrides, UINT RStream) {
    Remove(type);
    LoadFile(fname, type, pSODecl, SODeclNum, Strides, NumStrides, RStream);

    // Release blob
    /*int index = (int)log2((int)type);
    if( bShader[index] ) {
        bShader[index]->Release();
        bShader[index] = 0;
    }*/
}

void Shader::ReleaseBlobs() {
    for( int i = 0; i < Count; i++ ) {
        char j = 1 << i;

        // If exists and not linked
        if( __HLK(j) && bShader[i] ) {
            bShader[i]->Release();
            bShader[i] = 0;
        }
    }
}

ID3DBlob* Shader::GetBlob(ShaderType type) {
    return bShader[(int)log2((int)type)];
}

void Shader::AttachShader(Shader* origin, ShaderType type) {
    // If already loaded compiled shader
    if( (Type & type) == type ) {
        std::cout << "Can't attach shader type of " << type << ", since it's already loaded." << std::endl;
        return;
    }

    // Add ref to shader
    switch( type ) {
        case Vertex  : sVertex   = origin->sVertex  ; break;
        case Pixel   : sPixel    = origin->sPixel   ; break;
        case Geometry: sGeometry = origin->sGeometry; break;
        case Hull    : sHull     = origin->sHull    ; break;
        case Domain  : sDomain   = origin->sDomain  ; break;
        case Compute : sCompute  = origin->sCompute ; break;
    }

    // Add ref to polygon layout class
    if( type == Vertex ) {
        pl->Assign(origin->pl->GetLayout());
    }

    // Add as linked
    Linked |= type;
    Type |= type;
}

#define _DTouchGeo ((DTouch & Geometry) == 0 && (DTouch & GeometrySO) == 0)

void Shader::Bind() {
    gState = this; // 

    pl->Bind();

    if( __Has(Vertex  ) ) { gDirectX->gContext->VSSetShader(sVertex  , NULL, 0); } else if( (DTouch & Vertex ) == 0 ) { gDirectX->gContext->VSSetShader(nullptr, NULL, 0); }
    if( __Has(Pixel   ) ) { gDirectX->gContext->PSSetShader(sPixel   , NULL, 0); } else if( (DTouch & Pixel  ) == 0 ) { gDirectX->gContext->PSSetShader(nullptr, NULL, 0); }
    if( __GSO           ) { gDirectX->gContext->GSSetShader(sGeometry, NULL, 0); } else if( _DTouchGeo              ) { gDirectX->gContext->GSSetShader(nullptr, NULL, 0); }
    if( __Has(Hull    ) ) { gDirectX->gContext->HSSetShader(sHull    , NULL, 0); } else if( (DTouch & Hull   ) == 0 ) { gDirectX->gContext->HSSetShader(nullptr, NULL, 0); }
    if( __Has(Domain  ) ) { gDirectX->gContext->DSSetShader(sDomain  , NULL, 0); } else if( (DTouch & Domain ) == 0 ) { gDirectX->gContext->DSSetShader(nullptr, NULL, 0); }
//  if( __Has(Compute ) ) { gDirectX->gContext->CSSetShader(sCompute , NULL, 0); } else if( (DTouch & Compute) == 0 ) { gDirectX->gContext->CSSetShader(nullptr, NULL, 0); }
}

void Shader::Dispatch(UINT x, UINT y, UINT z) {
    if( __Has(Compute) ) {
        gDirectX->gContext->CSSetShader(sCompute, NULL, 0);
        DXDispatch(x, y, z);
    }
}

#undef __Has
#undef __GSO
#undef __LNK
#undef __HLK
#undef _DTouchGeo

HRESULT Shader::CreateInputLayoutDescFromVertexShaderSignature(ID3D11InputLayout** pInputLayout, int* inputLayoutByteLength) {
    std::vector<D3D11_INPUT_ELEMENT_DESC> q;
    return CreateInputLayoutDescFromVertexShaderSignature(pInputLayout, inputLayoutByteLength, q);
}

// Slightly modified version of: 
// https://gist.github.com/Cody-Duncan/d85740563ceea99f6619
HRESULT Shader::CreateInputLayoutDescFromVertexShaderSignature(ID3D11InputLayout** pInputLayout, int* inputLayoutByteLength, std::vector<D3D11_INPUT_ELEMENT_DESC> elements) {
    ID3DBlob* pShaderBlob = bShader[0]; // Get vertex shader blob

    // Reflect shader info
    ID3D11ShaderReflection* pVertexShaderReflection = nullptr;
    HRESULT hr = S_OK;
    if( FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)) ) {
        return S_FALSE;
    }

    // get shader description
    D3D11_SHADER_DESC shaderDesc;
    pVertexShaderReflection->GetDesc(&shaderDesc);

    // Read input layout description from shader info
    uint32_t byteOffset = 0;
    uint32_t j = 0;
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
    for( unsigned int i = 0; i < shaderDesc.InputParameters; ++i ) {
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
        pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

        // Skip some auto-generated semantics
        if( paramDesc.SemanticName == "SV_InstanceID" || paramDesc.SemanticName == "SV_VertexID" ) {
            continue;
        }

        // create input element desc
        D3D11_INPUT_ELEMENT_DESC elementDesc;
        elementDesc.SemanticName         = paramDesc.SemanticName;
        elementDesc.SemanticIndex        = paramDesc.SemanticIndex;
        elementDesc.InputSlot            = (Layout == LayoutGenerator::LgMesh) ? j++ : 0;
        elementDesc.AlignedByteOffset    = (Layout == LayoutGenerator::LgDefault) ? byteOffset : 0;
        elementDesc.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        elementDesc.InstanceDataStepRate = 0;
        //std::cout << elementDesc.SemanticName << std::endl;

        // determine DXGI format
        if( paramDesc.Mask == 1 ) {
            if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32_UINT;
            else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32_SINT;
            else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
            byteOffset += 4;
        } else if( paramDesc.Mask <= 3 ) {
            if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
            else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
            else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
            byteOffset += 8;
        } else if( paramDesc.Mask <= 7 ) {
            if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
            else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
            else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            byteOffset += 12;
        } else if( paramDesc.Mask <= 15 ) {
            if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
            else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
            else if( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            byteOffset += 16;
        }

        // Save element desc
        inputLayoutDesc.push_back(elementDesc);
    }

    // 
    for( int i = 0; i < elements.size(); i++ ) {
        // Determine DXGI format
        switch( elements[i].Format ) {
            case DXGI_FORMAT_R32_UINT:
            case DXGI_FORMAT_R32_SINT:
            case DXGI_FORMAT_R32_FLOAT:
                byteOffset += 4;
                break;

            case DXGI_FORMAT_R32G32_UINT:
            case DXGI_FORMAT_R32G32_SINT:
            case DXGI_FORMAT_R32G32_FLOAT:
                byteOffset += 8;
                break;

            case DXGI_FORMAT_R32G32B32_UINT:
            case DXGI_FORMAT_R32G32B32_SINT:
            case DXGI_FORMAT_R32G32B32_FLOAT:
                byteOffset += 12;
                break;

            case DXGI_FORMAT_R32G32B32A32_UINT:
            case DXGI_FORMAT_R32G32B32A32_SINT:
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
                byteOffset += 16;
                break;
        }

        // Save element desc
        inputLayoutDesc.push_back(elements[i]);
    }

    // Try to create Input Layout
    hr = gDirectX->gDevice->CreateInputLayout(&inputLayoutDesc[0], static_cast<UINT>(inputLayoutDesc.size()), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout);

    if( FAILED(hr)) {
        // Failed
        std::cout << "Failed to create Input Layout!" << std::endl;
    }

    // Free allocation shader reflection memory
    pVertexShaderReflection->Release();

    // Record byte length
    *inputLayoutByteLength = byteOffset;

    return hr;
}

bool Shader::CreatePolygonLayout() {
    int tmp;
    ID3D11InputLayout *IL;

    if( FAILED(CreateInputLayoutDescFromVertexShaderSignature(&IL, &tmp)) ) {
        return true;
    }

    pl->Assign(IL);

    return false;
}

bool Shader::CreatePolygonLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> elements) {
    int tmp;
    ID3D11InputLayout *IL;

    if( FAILED(CreateInputLayoutDescFromVertexShaderSignature(&IL, &tmp, elements)) ) {
        return true;
    }

    pl->Assign(IL);

    return false;
}
