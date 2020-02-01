#include "pc.h"

#include "UIManager.h"
#include "UIPrimitive.h"

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
std::array<uint32_t, UIManager::gMaxContainers>                                        UIManager::gContainerStackIDLayer{};
std::array<float3, UIManager::gMaxLayers>                                              UIManager::gContainerOffsetLayer{};

// Rendering
Shader*                        UIManager::shScreen{};
Shader*                        UIManager::shPrimitives{};
ConstantBuffer*                UIManager::cbVSDataBuffer{};
ConstantBuffer*                UIManager::cbPSDataBuffer{};
RenderTarget2DColor1DepthMSAA* UIManager::rtDestination{};

// Params
float UIManager::gScaleX = 1.f;
float UIManager::gScaleY = 1.f;

/*************************************************************\
|   UIContainer
\*************************************************************/
UIContainer::UIContainer(float3 off, float3 sz): bActive(true), Offset(off), Size(sz) { Init(); };
UIContainer::UIContainer(float2 off, float2 sz): bActive(true), Offset({ off.x, off.y, 0.f }), Size({ sz.x, sz.y, 1.f }) { Init(); };
UIContainer::UIContainer(float x, float y, float w, float h): bActive(true), Offset({ x, y, 0.f }), Size({ w, h, 1.f }) { Init(); };

void UIContainer::Init() {
    uint32_t LID = UIPrimitive::gLayerID;

    // Copy
    UIContainer *ptr = UIManager::gContainerStackLayer[LID][UIManager::gContainerStackIDLayer[LID]];
    ptr->bActive = bActive;
    ptr->Offset  = Offset;
    ptr->Size    = Size;

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

const UIVertex& UIContainer::Clamp(const UIVertex& v) const {
    // We don't need to clamp this vertex
    if( Inside(v) ) return v;

    // Otherwise clamp
    UIVertex v1(v);
    float3 start = UIManager::gContainerOffsetLayer[UIPrimitive::gLayerID] + Offset;
    v1.Position.x = std::clamp(v.Position.x, Offset.x, Offset.x + Size.x);
    v1.Position.y = std::clamp(v.Position.y, Offset.y, Offset.y + Size.y);
    v1.Position.z = std::clamp(v.Position.z, Offset.z, Offset.z + Size.z);

    return v1;
}

/*************************************************************\
|   UIManager
\*************************************************************/
void UIManager::Init() {
    for( uint i = 0; i < gMaxLayers; i++ ) {
        gVBLayer[i] = new VertexBuffer();

        for( uint32_t j = 0; j < gMaxContainers; j++ ) {
            gContainerStackLayer[i][j] = new UIContainer();
        }
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
void UIManager::Render() {
    {
        ScopeMapConstantBuffer<VSDataBuffer> q(cbVSDataBuffer);
        q.data->mProj = DirectX::XMMatrixOrthographicOffCenterLH(0.f, Width(), Height(), 0.f, .1f, float(gMaxLayers));
    }

    // Bind
    rtDestination->Bind();
    shPrimitives->Bind();
    cbVSDataBuffer->Bind(Shader::Vertex, 0u);

    for( uint i = 0; i < gLastActiveLayers.size(); i++ ) {
        uint32_t index = gLastActiveLayers[i];

        gVBLayer[index]->BindVertex();

        DXDraw(gDCLayer[index], 0);
    }
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
    }

    SAFE_RELEASE(shScreen);
    SAFE_RELEASE(shPrimitives);
    SAFE_RELEASE(rtDestination);
    SAFE_RELEASE(cbVSDataBuffer);
    SAFE_RELEASE(cbPSDataBuffer);
}

float UIManager::Width() { return rtDestination->GetWidth(); }
float UIManager::Height() { return rtDestination->GetHeight(); }

