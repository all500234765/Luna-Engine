#include "pc.h"

#include "UIManager.h"

std::array<VertexBuffer*, UIManager::gMaxLayers>                    UIManager::gVBLayer{};
std::array<uint64_t, UIManager::gMaxLayers>                         UIManager::gDCLayer{}; // DataCounter
std::vector<uint32_t>                                               UIManager::gLastActiveLayers{};
std::array<std::vector<UIManager::UIVertex>, UIManager::gMaxLayers> UIManager::gVertexLayer{};

// Rendering
Shader*                        UIManager::shScreen{};
Shader*                        UIManager::shPrimitives{};
ConstantBuffer*                UIManager::cbVSDataBuffer{};
ConstantBuffer*                UIManager::cbPSDataBuffer{};
RenderTarget2DColor1DepthMSAA* UIManager::rtDestination{};

// Params
float UIManager::gScaleX = 1.f;
float UIManager::gScaleY = 1.f;

void UIManager::Init() {
    for( uint i = 0; i < gMaxLayers; i++ ) {
        gVBLayer[i] = new VertexBuffer();
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
    for( uint i = 0; i < gLastActiveLayers.size(); i++ ) {
        uint32_t index = gLastActiveLayers[i];

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

    DXDraw(6, 0);
}

void UIManager::Release() {
    for( uint i = 0; i < gMaxLayers; i++ ) {
        SAFE_RELEASE(gVBLayer[i]);
    }

    SAFE_RELEASE(shScreen);
    SAFE_RELEASE(shPrimitives);
    SAFE_RELEASE(rtDestination);
    SAFE_RELEASE(cbVSDataBuffer);
    SAFE_RELEASE(cbPSDataBuffer);
}

float UIManager::Width() { return rtDestination->GetWidth(); }
float UIManager::Height() { return rtDestination->GetHeight(); }

