#include "TextFactory.h"

TextFactory::TextFactory() {
    // Create text transformation buffer
    mCBTransform = new ConstantBuffer();
    mCBTransform->CreateDefault(sizeof(TextTransform));
    TextTransform *data = (TextTransform*)mCBTransform->Map();
        data->_mWorld = DirectX::XMMatrixIdentity(); // Set default matrix
    mCBTransform->Unmap();
}

Text* TextFactory::Build(char* text) {
    // We have to have font for that
    if( mFont == nullptr ) { return; }

    using namespace DirectX; // So it won't be such pain in the ass

    int length = strlen(text); // Text length
    float offsetX = 0.f,       // Character offset
          offsetY = 0.f;

    float cw = mFont->GetCharW(); // Single character size
    float ch = mFont->GetCharH();

    std::vector<Vertex_PT> vertices; // 

    // Push single char
    auto PushChar = [&vertices, &offsetX, &offsetY, &ch, &cw](Font* mFont, char ch) {
        // Push single vertex
        auto PushVertex = [&vertices](XMFLOAT3 p, XMFLOAT2 uv) {
            Vertex_PT v;
            v.Position = p;
            v.Texcoord = uv;
            vertices.push_back(v);
        };

        XMFLOAT2 uv0 /* Top-Left */, uv1 /* Bottom-Right */;

        // Tri 1
        PushVertex({ offsetX     , offsetY     , 0.f }, uv0);              // Top-Left
        PushVertex({ offsetX + cw, offsetY     , 0.f }, { uv1.x, uv0.y }); // Top-Right
        PushVertex({ offsetX + cw, offsetY + ch, 0.f }, uv1);              // Bottom-Right

        // Tri 2
        PushVertex({ offsetX     , offsetY     , 0.f }, uv0);              // Top-Left
        PushVertex({ offsetX + cw, offsetY + ch, 0.f }, uv1);              // Bottom-Right
        PushVertex({ offsetX     , offsetY + ch, 0.f }, { uv0.x, uv1.y }); // Bottom-Left

        // Move caret to the right
        offsetX += mFont->GetCharW();
    };

    // Process text
    for( int i = 0; i < length; i++ ) {
        char c = text[i]; // Get current char

        if( c == 32 ) {        // " "
            offsetX += cw;
            continue;
        } else if( c == 10 ) { // "\n"
            offsetY += ch;
            continue;
        }

        // Push single char
        PushChar(mFont, c);
    }

    // Build vertex buffer and create mesh
    Mesh *mText;
    VertexBuffer *vb;

    vb->CreateDefault(vertices.size(), sizeof(Vertex_PT), &vertices[0]);
    mText->SetBuffer(vb, nullptr);

    Text* _Text;
    _Text->SetMesh(mText);

    return _Text;
}

Text* TextFactory::Build(Text* old, char* text) {
    old->Release();
    return Build(text);
}

void TextFactory::Draw(Text* text, float x, float y) {
    if( mFont == nullptr ) { return; } // Won't draw, since there's will be nothing to draw

    // Fill transformation buffer
    TextTransform *data = (TextTransform*)mCBTransform->Map();
        data->_mWorld = DirectX::XMMatrixTranslation(x, y, 0);
    mCBTransform->Unmap();

    // Bind transformation
    mCBTransform->Bind(Shader::Vertex, 0);

    // Bind font
    mFont->Bind(0);

    // Render text
    text->Draw();
}
