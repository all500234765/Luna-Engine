#include "pc.h"

#include "UIManager.h"
#include "UIPrimitive.h"

#include "Primitives/UIRectangle.h"

/*************************************************************\
|   UIManager statics
\*************************************************************/
// Per layer data for vertex buffer gen
std::array<VertexBuffer*, UIManager::gMaxLayers>         UIManager::gVBLayer{};
std::array<uint64_t, UIManager::gMaxLayers>              UIManager::gDCLayer{}; // DataCounter
std::vector<uint32_t>                                    UIManager::gLastActiveLayers{};
std::array<std::vector<UIVertex>, UIManager::gMaxLayers> UIManager::gVertexLayer{};

// Containers
std::array<std::array<UIContainer*, UIManager::gMaxContainers>, UIManager::gMaxLayers> UIManager::gContainerStackLayer{};
std::array<uint32_t, UIManager::gMaxLayers>                                            UIManager::gContainerStackIDLayer{};
std::array<float3, UIManager::gMaxLayers>                                              UIManager::gContainerOffsetLayer{};

// Rendering
Shader*                        UIManager::shScreen{};
Shader*                        UIManager::shPrimitives{};
ConstantBuffer*                UIManager::cbVSDataBuffer{};
ConstantBuffer*                UIManager::cbPSDataBuffer{};
RenderTarget2DColor1DepthMSAA* UIManager::rtDestination{};
RasterState*                   UIManager::rsDefault{};
RasterState*                   UIManager::rsWire{};

// Misc
std::array<std::array<std::array<float2, UIManager::gMaxScrollbars>, UIManager::gMaxContainers>, UIManager::gMaxLayers> UIManager::gScrollbarContentSize{};
std::array<std::array<std::array<UIManager::UIScrollbarState*, UIManager::gMaxScrollbars>, UIManager::gMaxContainers>, UIManager::gMaxLayers> UIManager::gScrollbarState{};

uint32_t UIManager::gCirclePrecision = 32;

// Params
float UIManager::gScaleX = 1.f;
float UIManager::gScaleY = 1.f;

/*************************************************************\
|   UIContainer
\*************************************************************/
UIContainer::UIContainer(float3 off, float3 sz): bActive(true), Offset(off), Size(sz) { Init(); };
UIContainer::UIContainer(float2 off, float2 sz): bActive(true), Offset({ off.x, off.y, 0.f }), Size({ sz.x, sz.y, 1.f }) { Init(); };
UIContainer::UIContainer(float x, float y, float w, float h): bActive(true), Offset({ x, y, 0.f }), Size({ w, h, 1.f }) { Init(); };
UIContainer::~UIContainer() {
    uint32_t LID = UIPrimitive::gLayerID;
    uint32_t SID = std::max((int)UIManager::gContainerStackIDLayer[LID] - 1, 0);

    // Copy
    if( SID < UIManager::gMaxContainers ) {
        UIContainer *ptr = UIManager::gContainerStackLayer[LID][SID];
        if( ptr ) {
            ptr->bActive = false;
            UIManager::gContainerOffsetLayer[LID] -= ptr->Offset;
        } else {
            //UIManager::gContainerOffsetLayer[LID] -= Offset;
        }
    } else {
        //UIManager::gContainerOffsetLayer[LID] -= Offset;
    }

    // Advance
    UIManager::gContainerStackIDLayer[LID]--;
}

void UIContainer::Init() {
    uint32_t LID = UIPrimitive::gLayerID;
    uint32_t SID = UIManager::gContainerStackIDLayer[LID]; // TODO: Check if must add -1 ?

    // Copy
    if( SID < UIManager::gMaxContainers ) {
        UIContainer *ptr = UIManager::gContainerStackLayer[LID][SID];
        if( ptr ) {
            ptr->bActive = bActive;
            ptr->Offset  = Offset;
            ptr->Size    = Size;
        }
    }

    // Advance
    UIManager::gContainerStackIDLayer[LID]++;
    UIManager::gContainerOffsetLayer[LID] += Offset;
}

bool UIContainer::Inside(const UIVertex& v) const {
    float3 start = UIManager::gContainerOffsetLayer[UIPrimitive::gLayerID];
    float3 end = start + Size;

    return (v.Position.x >= start.x) && (v.Position.x <= end.x)
        && (v.Position.y >= start.y) && (v.Position.y <= end.y)
        && (v.Position.z >= start.z) && (v.Position.z <= end.z);
}

bool UIContainer::Inside(const UIVertex& v0, const UIVertex& v1, const UIVertex& v2) const {
    return Inside(v0) && Inside(v1) && Inside(v2);
}

bool UIContainer::AtleastInside(const UIVertex& v0, const UIVertex& v1, const UIVertex& v2) const {
    return Inside(v0) || Inside(v1) || Inside(v2);
}

UIVertex&& UIContainer::Clamp(const UIVertex& v) const {
    UIVertex v1(v);

    // We don't need to clamp this vertex
    if( Inside(v) ) 
        return std::move(v1);

    // Otherwise clamp
    float3 start = UIManager::gContainerOffsetLayer[UIPrimitive::gLayerID];
    float3 end = start + Size;

    v1.Position.x = std::clamp(v.Position.x, start.x, end.x);
    v1.Position.y = std::clamp(v.Position.y, start.y, end.y);
    v1.Position.z = std::clamp(v.Position.z, start.z, end.z);

    return std::move(v1);
}

/*************************************************************\
|   UIManager
\*************************************************************/
void UIManager::Init() {
    for( uint i = 0; i < gMaxLayers; i++ ) {
        gVBLayer[i] = new VertexBuffer();

        for( uint32_t j = 0; j < gMaxContainers; j++ ) {
            gContainerStackLayer[i][j] = new UIContainer();

            for( uint32_t k = 0; k < gMaxScrollbars; k++ ) {
                gScrollbarState[i][j][k] = new UIScrollbarState();
            }
        }
    }

    // Raster states
    rsDefault = new RasterState;
    rsWire    = new RasterState;

    {
        D3D11_RASTERIZER_DESC pDesc;
        ZeroMemory(&pDesc, sizeof(D3D11_RASTERIZER_DESC));
        pDesc.AntialiasedLineEnable = true;
        pDesc.CullMode              = D3D11_CULL_NONE;
        pDesc.DepthBias             = 0;
        pDesc.DepthBiasClamp        = 0.0f;
        pDesc.DepthClipEnable       = true;
        pDesc.FillMode              = D3D11_FILL_SOLID;
        pDesc.FrontCounterClockwise = true;
        pDesc.MultisampleEnable     = true;
        pDesc.ScissorEnable         = false;
        pDesc.SlopeScaledDepthBias  = 0.0f;

        // Normal
        rsDefault->Create(pDesc);

        // Wireframe
        pDesc.FillMode = D3D11_FILL_WIREFRAME;
        rsWire->Create(pDesc);
    }

    // Shaders
    shPrimitives = new Shader();
    shPrimitives->LoadFile("shUIPrimitivesVS.cso", Shader::Vertex);
    shPrimitives->LoadFile("shUIPrimitivesPS.cso", Shader::Pixel);

    shScreen = new Shader();
    shScreen->LoadFile("shTexturedQuadSimpliestVS.cso", Shader::Vertex);
    shScreen->LoadFile("shGUIPS.cso", Shader::Pixel);

    shPrimitives->ReleaseBlobs();
    shScreen->ReleaseBlobs();

    // Render Targets
    UINT w = (UINT)(1366.f / gScaleX), h = (UINT)(768.f / gScaleY);
    rtDestination = new RenderTarget2DColor1DepthMSAA(w, h);
    rtDestination->Create(16u);
    rtDestination->CreateList(0u, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Constant Buffers
    cbVSDataBuffer = new ConstantBuffer();
    cbVSDataBuffer->CreateDefault(sizeof(VSDataBuffer));

    // TODO: ?
    cbPSDataBuffer = new ConstantBuffer();
    //cbPSDataBuffer->CreateDefault(sizeof(PSDataBuffer));
}

void UIManager::Clear() {
    for( uint32_t i = 0; i < gMaxContainers; i++ ) {
        gContainerStackIDLayer[i] = 0;
    }

    for( uint32_t i = 0; i < gMaxLayers; i++ ) {
        gContainerOffsetLayer[i] = { 0.f, 0.f, 0.f };
    }

    for( uint32_t i = 0; i < gLastActiveLayers.size(); i++ ) {
        uint32_t index = gLastActiveLayers[i];

        for( uint32_t j = 0; j < gMaxContainers; j++ ) {
            gContainerStackLayer[i][j]->bActive = {};

            for( uint32_t k = 0; k < gMaxScrollbars; k++ )
                gScrollbarContentSize[i][j][k] = { -9999.f, -9999.f };
        }

        gVertexLayer[index].clear();
        gVBLayer[index]->Release();
        gDCLayer[index] = 0;
    }

    const float ClearColor[4] = { 0.f, 0.f, 0.f, 0.f };

    gLastActiveLayers.clear();
    rtDestination->Clear(0.f, 0);
    rtDestination->Clear(ClearColor);
}

void UIManager::Submit() {
    for( uint i = 0; i < gLastActiveLayers.size(); i++ ) {
        uint32_t index = gLastActiveLayers[i];

        gVBLayer[index]->CreateDefault(gDCLayer[index], sizeof(UIVertex), &gVertexLayer[index][0]);
    }
}

// TODO: Custom (w/ shaders)
void UIManager::Render(bool debug_wire) {
    {
        ScopeMapConstantBuffer<VSDataBuffer> q(cbVSDataBuffer);
        q.data->mProj = DirectX::XMMatrixOrthographicOffCenterLH(0.f, Width(), Height(), 0.f, .1f, float(gMaxLayers));
    }

    // Bind
    rtDestination->Bind();
    shPrimitives->Bind();
    cbVSDataBuffer->Bind(Shader::Vertex, 0u);

    // For Lisa
    RasterState::Push();
    (debug_wire ? rsWire : rsDefault)->Bind();

    for( uint i = 0; i < gLastActiveLayers.size(); i++ ) {
        uint32_t index = gLastActiveLayers[i];

        gVBLayer[index]->BindVertex();

        DXDraw(gDCLayer[index], 0);
    }

    // Restore state
    RasterState::Pop();
}

void UIManager::Screen() {
    shScreen->Bind();

    rtDestination->Bind(1u, Shader::Pixel, 0u);
    // TODO: Add sampler

    DXDraw(6, 0);
}

void UIManager::Release() {
    SAFE_RELEASE_N(gVBLayer, gMaxLayers);
    for( uint j = 0; j < gMaxLayers; j++ ) {
        SAFE_DELETE_N(gContainerStackLayer[j], gMaxContainers);

        for( uint k = 0; k < gMaxContainers; k++ ) {
            SAFE_DELETE_N(gScrollbarState[j][k], gMaxScrollbars);
        }
    }

    SAFE_RELEASE(rsWire);
    SAFE_RELEASE(shScreen);
    SAFE_RELEASE(rsDefault);
    SAFE_RELEASE(shPrimitives);
    SAFE_RELEASE(rtDestination);
    SAFE_RELEASE(cbVSDataBuffer);
    SAFE_RELEASE(cbPSDataBuffer);
}

float3 UIManager::GetOffset() {
    return gContainerOffsetLayer[UIPrimitive::gLayerID];
}

// TODO: 
//uint32_t SID = std::max((int)gContainerStackIDLayer[gLayerID] - 1, 0);

float UIManager::Width() { return rtDestination->GetWidth(); }
float UIManager::Height() { return rtDestination->GetHeight(); }

