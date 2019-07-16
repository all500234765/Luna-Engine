#include "TextFactory.h"

TextFactory::TextFactory(Shader* shader) {
    mShader = shader;
}

Text* TextFactory::Build(const char* text, int maxWidth) {
    // We have to have font for that
    if( mFont == nullptr ) { return nullptr; }

    using namespace DirectX; // So it won't be such pain in the ass

    int length = strlen(text); // Text length
    float offsetX = 0.f,       // Character offset
          offsetY = 0.f;

    std::vector<Vertex_PT> vertices; // Vertex array

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
        PushVertex({ offsetX     , offsetY     , 0.f }, uv0);              // Top-Left
        PushVertex({ offsetX + cW, offsetY     , 0.f }, { uv1.x, uv0.y }); // Top-Right
        PushVertex({ offsetX + cW, offsetY + cH, 0.f }, uv1);              // Bottom-Right

        // Tri 2
        PushVertex({ offsetX     , offsetY     , 0.f }, uv0);              // Top-Left
        PushVertex({ offsetX + cW, offsetY + cH, 0.f }, uv1);              // Bottom-Right
        PushVertex({ offsetX     , offsetY + cH, 0.f }, { uv0.x, uv1.y }); // Bottom-Left

        // Move caret to the right
        offsetX += mFont->GetAdvance(ch);
    };

    // Process text
    for( int i = 0; i < length; i++ ) {
        char c = text[i]; // Get current char

        if( offsetX >= maxWidth ) {
            offsetX -= maxWidth;
            offsetY += mFont->GetLineHeight();
        }

        if( c == 32 ) {        // " "
            offsetX += 3.f;
            continue;
        } else if( c == 10 ) { // "\n"
            offsetY += 16.f;
            continue;
        }

        // Push single char
        PushChar(mFont, c);
    }

    // Build vertex buffer and create mesh
    Mesh *mText = new Mesh();
    VertexBuffer *vb = new VertexBuffer();

    vb->CreateDefault(vertices.size(), sizeof(Vertex_PT), &vertices[0]);
    mText->SetBuffer(vb, nullptr);

    // Generate text
    return new Text(mText, offsetX + mFont->GetAdvance(text[length - 1]), offsetY + mFont->GetLineHeight());
}

Text* TextFactory::Build(Text* old, const char* text, int maxWidth) {
    old->Release();
    return Build(text, maxWidth);
}

void TextFactory::Draw(Text* text) {
    if( mFont == nullptr ) { return; } // Won't draw, since there's will be nothing to draw

    // Bind shader
    gDirectX->gContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mShader->Bind();

    // Bind font
    mFont->Bind();

    // Render text
    text->Draw();
}

void TextFactory::Release() {

}
