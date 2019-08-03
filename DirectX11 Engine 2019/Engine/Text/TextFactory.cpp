#include "TextFactory.h"

TextFactory::TextFactory(Shader* shader) {
    mShader = shader;

    cbTextEffects = new ConstantBuffer();
    cbTextEffects->CreateDefault(sizeof(TextEffects));

    cbSDFSettings = new ConstantBuffer();
    cbSDFSettings->CreateDefault(sizeof(SDFSettings));
}

Text* TextFactory::Build(const char* text, float maxWidth) {
    // We have to have font for that
    if( mFont == nullptr ) { return nullptr; }

    using namespace DirectX; // So it won't be such pain in the ass

    size_t length = strlen(text); // Text length
    float offsetX = 0.f,       // Character offset
          offsetY = 0.f;

    std::vector<Vertex_PT> vertices; // Vertex array

    // Font scaling
    float fScale = 1.f / static_cast<float>(mFont->GetScale());

    // Push single char
    auto PushChar = [&vertices, &offsetX, &offsetY](Font* mFont, char ch) {
        // Push single vertex
        auto PushVertex = [&vertices](XMFLOAT3 p, XMFLOAT2 uv) {
            Vertex_PT v;
                v.Position = p;
                v.Texcoord = uv;
            vertices.push_back(v);
        };

        float cW = mFont->GetWidth(ch);
        float cH = mFont->GetHeight(ch);

        XMFLOAT2 uv0 /* Top-Left     */ = mFont->GetCharUV0(ch), 
                 uv1 /* Bottom-Right */ = mFont->GetCharUV1(ch);

        // Tri 1
        PushVertex({ offsetX      + mFont->GetCharX(ch), offsetY      + mFont->GetCharY(ch), .5f }, uv0);              // Top-Left
        PushVertex({ offsetX + cW + mFont->GetCharX(ch), offsetY      + mFont->GetCharY(ch), .5f }, { uv1.x, uv0.y }); // Top-Right
        PushVertex({ offsetX + cW + mFont->GetCharX(ch), offsetY + cH + mFont->GetCharY(ch), .5f }, uv1);              // Bottom-Right

        // Tri 2
        PushVertex({ offsetX      + mFont->GetCharX(ch), offsetY      + mFont->GetCharY(ch), .5f }, uv0);              // Top-Left
        PushVertex({ offsetX + cW + mFont->GetCharX(ch), offsetY + cH + mFont->GetCharY(ch), .5f }, uv1);              // Bottom-Right
        PushVertex({ offsetX      + mFont->GetCharX(ch), offsetY + cH + mFont->GetCharY(ch), .5f }, { uv0.x, uv1.y }); // Bottom-Left
    };

    // Process text
    maxWidth *= fScale;
    //int iSpaces = 0;
    float offsetX2 = 0.f;
    for( size_t i = 0; i < length; i++ ) {
        char c = text[i]; // Get current char

        // New line
        if( maxWidth > -1 && offsetX >= maxWidth ) {
            //offsetX -= maxWidth;
            //offsetY += mFont->GetLineHeight() * fScale;
        }

        if( c == 32 ) {        // " "
            offsetX += 30.f * fScale * mFont->GetSpacing();
            //paces++;
            continue;
        } else if( c == 10 ) { // "\n"
            offsetY += mFont->GetLineHeight() * fScale;
            offsetX = 0.f;
            //iSpaces = 0;
            continue;
        }

        // Push single char
        PushChar(mFont, c);

        // Move caret to the right
        offsetX += mFont->GetAdvance(c) * mFont->GetSpacing();
        if( i + 1 < length || length == 1 ) offsetX2 += mFont->GetCharX(c) + mFont->GetAdvance(c) * mFont->GetSpacing();
    }

    // Build vertex buffer and create mesh
    Mesh *mText = new Mesh();
    VertexBuffer *vb = new VertexBuffer();

    vb->CreateDefault(static_cast<UINT>(vertices.size()), sizeof(Vertex_PT), &vertices[0]);
    mText->SetBuffer(vb, nullptr);

    // Generate text
    return new Text(mText, offsetX2, offsetY + mFont->GetLineHeight() * fScale, text);
}

Text* TextFactory::Build(Text* old, const char* text, float maxWidth) {
    //if( strcmp(old->GetText(), text) == 0 ) { return old; }

    old->Release();
    delete old;
    return Build(text, maxWidth);
}

void TextFactory::Draw(Text* text) {
    if( mFont == nullptr ) { return; } // Won't draw, since there's will be nothing to draw

    // Bind shader
    gDirectX->gContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mShader->Bind();

    // Bind effects buffer
    cbTextEffects->Bind(Shader::Pixel, 0);

    // Bind SDF Settings buffer
    cbSDFSettings->Bind(Shader::Pixel, 1);

    // Bind font
    mFont->Bind();

    // Render text
    text->Draw();
}

void TextFactory::Release() {
    cbTextEffects->Release();
    cbSDFSettings->Release();

    delete cbSDFSettings;
    delete cbTextEffects;
}
