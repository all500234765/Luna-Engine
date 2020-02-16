#include "pc.h"

#include "UIText.h"

std::array<VertexBuffer*, UIManager::gMaxLayers>         UIText::gVertexBuffers{};
std::array<std::vector<UIVertex>, UIManager::gMaxLayers> UIText::gVertices{};
std::array<uint64_t, UIManager::gMaxLayers>              UIText::gVertexNum{};

ConstantBuffer*  UIText::cbTextEffects{};
ConstantBuffer*  UIText::cbSDFSettings{};
TextFactory*     UIText::gTextFactory{};
Font*            UIText::gDefaultFont{};
Font*            UIText::gCurrentFont{};
Shader*          UIText::gSDFShader{};

UIText::UIText(float2 pos, const char* text) {
    UIText(pos.x, pos.y, text);
}

UIText::UIText(float x, float y, const char* text) {
    // We have to have font for that
    if( gCurrentFont == nullptr ) { return; }

    using namespace DirectX; // So it won't be such pain in the ass

    auto& vertices = gVertices[gLayerID]; // Pointer to vertices
    size_t length = strlen(text); // Text length
    float offsetX = x,            // Character offset
          offsetY = y;

    // Approximate amount of space requared (should skip all spaces " " from equation)
    vertices.reserve(4 * length);

    // Font scaling
    float fScala = static_cast<float>(gCurrentFont->GetScale());
    float fScale = gDefaultFont->GetScaling();// / fScala;

    // Push single char
    auto PushChar = [&fScale, &vertices, &offsetX, &offsetY](Font* mFont, char ch) {
        // Push single vertex
        auto PushVertex = [&vertices](float3 p, float2 uv) {
            UIVertex v;
            v.Position = p;
            v.Texcoord = uv;
            v.Color = UIPrimitive::gColor;
            //gVertexNum[gLayerID]++;
            return v;
        };

        float cW = mFont->GetWidth(ch)  * fScale;
        float cH = mFont->GetHeight(ch) * fScale;

        float2 uv0 /* Top-Left     */ = mFont->GetCharUV0(ch),
               uv1 /* Bottom-Right */ = mFont->GetCharUV1(ch);

        // Tri 1
        float ox = mFont->GetCharX(ch) * fScale;
        float oy = mFont->GetCharY(ch) * fScale;

        UIVertex v0 = PushVertex({ offsetX      + ox, offsetY      + oy, .5f }, uv0);              // Top-Left
        UIVertex v1 = PushVertex({ offsetX + cW + ox, offsetY      + oy, .5f }, { uv1.x, uv0.y }); // Top-Right
        UIVertex v2 = PushVertex({ offsetX + cW + ox, offsetY + cH + oy, .5f }, uv1);              // Bottom-Right
        AddTriangle(gVertexNum[gLayerID], &vertices, v0, v1, v2);

        // Tri 2
        v0 = PushVertex({ offsetX      + ox, offsetY      + oy, .5f }, uv0);              // Top-Left
        v1 = PushVertex({ offsetX + cW + ox, offsetY + cH + oy, .5f }, uv1);              // Bottom-Right
        v2 = PushVertex({ offsetX      + ox, offsetY + cH + oy, .5f }, { uv0.x, uv1.y }); // Bottom-Left

        AddTriangle(gVertexNum[gLayerID], &vertices, v0, v1, v2);
    };

    // Process text
    //maxWidth *= fScale;
    //int iSpaces = 0;
    float offsetX2 = 0.f;
    for( size_t i = 0; i < length; i++ ) {
        char c = text[i]; // Get current char

        // New line
        //if( maxWidth > -1 && offsetX >= maxWidth ) {
        //    //offsetX -= maxWidth;
        //    //offsetY += mFont->GetLineHeight() * fScale;
        //}

        if( c == 32 ) {        // " "
            offsetX += 30.f/30.f * fScale * gCurrentFont->GetSpacing();
            //paces++;
            continue;
        } else if( c == 10 ) { // "\n"
            offsetY += gCurrentFont->GetLineHeight() * fScale;
            offsetX = 0.f;
            //iSpaces = 0;
            continue;
        }

        // Push single char
        PushChar(gCurrentFont, c);

        // Move caret to the right
        offsetX += gCurrentFont->GetAdvance(c) * gCurrentFont->GetSpacing() * fScale;
        if( i + 1 < length || length == 1 )
            offsetX2 += gCurrentFont->GetCharX(c) + gCurrentFont->GetAdvance(c) * gCurrentFont->GetSpacing();
    }

    // Create mesh
    //vertices

    // Generate text
    //return new Text(mText, offsetX2, offsetY + mFont->GetLineHeight() * fScale, text);
}

void UIText::Init() {
    gSDFShader = new Shader();
    gSDFShader->LoadFile("shUIPrimitivesVS.cso", Shader::Vertex);
    gSDFShader->LoadFile("shUITextSDFPS.cso", Shader::Pixel);

    gSDFShader->ReleaseBlobs();

    //gTextFactory = new TextFactory(gSDFShader);

    gDefaultFont = new Font("../Fonts/fConsolasSDF.fnt", UIManager::gPointSampler, true);
    gDefaultFont->SetSpacing(.7f);
    gDefaultFont->SetScaling(17.f);
    gCurrentFont = gDefaultFont;

    cbTextEffects = new ConstantBuffer();
    cbTextEffects->CreateDefault(sizeof(TextEffects));

    cbSDFSettings = new ConstantBuffer();
    cbSDFSettings->CreateDefault(sizeof(SDFSettings));
    
    {
        ScopeMapConstantBuffer<SDFSettings> q(cbSDFSettings);
        q.data->_CharWidth = .4f;
        q.data->_Softening = .1f;
        q.data->_BorderWidth = .5f;
        q.data->_BorderSoft = .1f;
    }

    for( uint32_t i = 0; i < gMaxLayers; i++ ) {
        gVertexNum[i] = 0;
        gVertices[i] = {};
        gVertexBuffers[i] = new VertexBuffer();
    }
}

void UIText::Release() {
    SAFE_RELEASE(gTextFactory);
    SAFE_RELEASE(gDefaultFont);
    SAFE_RELEASE(gSDFShader);

    SAFE_RELEASE_N(gVertexBuffers, gMaxLayers);
}

void UIText::Clear() {
    for( uint32_t i = 0; i < gMaxLayers; i++ ) {
        gVertexBuffers[i]->Release();
        gVertexNum[i] = 0;
        gVertices[i] = {};
    }
    

}

void UIText::Submit() {
    for( uint32_t layer = 0; layer < gMaxLayers; layer++ ) {
        if( gVertexNum[layer] ) {
            gVertexBuffers[layer]->CreateDefault(gVertexNum[layer], sizeof(UIVertex), &gVertices[layer][0]);
        }
    }
}

void UIText::Render() {
    gSDFShader->Bind();
    gCurrentFont->Bind(0u);

    // Bind effects buffer
    cbTextEffects->Bind(Shader::Pixel, 0);

    // Bind SDF Settings buffer
    cbSDFSettings->Bind(Shader::Pixel, 1);

    for( uint32_t layer = 0; layer < gMaxLayers; layer++ ) {
        if( gVertexBuffers[layer]->GetNumber() ) {
            gVertexBuffers[layer]->BindVertex();
            DXDraw(gVertexBuffers[layer]->GetNumber(), 0);
        }
    }
}
