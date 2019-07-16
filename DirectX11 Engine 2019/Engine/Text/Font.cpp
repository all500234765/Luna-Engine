#include "Font.h"

Font::Font(const char* fontFile, Sampler* s): sFont(s) {
    // Load font data
    std::ifstream file(std::string(FONT_DIRECTORY) + std::string(fontFile));

    if( file.fail() ) {
        std::cout << "[Font]: Error loading font data file! (fname=" << fontFile << ")" << std::endl;
        return;
    }

    // 
    std::string line, fullLine;

    // GB
    auto EraseGarbage = [&line](const char* begin, const char* end, int offset=0) {
        line.erase(0, offset + strlen(begin));
        int p0 = line.find(end);
        if( p0 == -1 ) { return; }
        line.erase(p0, line.length() - p0);
    };

    // Get font size
    std::getline(file, line);
    EraseGarbage("size=", " bold=", line.find("size="));
    mScale = static_cast<float>(atof(line.c_str()));

    // Get some data
    std::getline(file, fullLine);

    // Get lineHeight
    line = fullLine;
    EraseGarbage("common lineHeight=", " ");
    mHeight = static_cast<float>(atof(line.c_str()));

    // Get texture file name and load texture
    std::getline(file, line);
    EraseGarbage("page id=0 file=\"", "\"");
    tFont = new Texture(std::string(FONT_DIRECTORY) + std::string(line), DXGI_FORMAT_R8G8B8A8_UNORM);
    
    // Get number of characters
    std::getline(file, line);
    EraseGarbage("chars count=", "\n");
    int N = atoi(line.c_str());
    aChars.resize(N);

    // Read char data
    float fScale  = 1.f / mScale;
    float fWidth  = 1.f / static_cast<float>(tFont->GetWidth());
    float fHeight = 1.f / static_cast<float>(tFont->GetHeight());

    for( int i = 0; i < N; i++ ) {
        // Read data and fill structure
        Character ch = {};

        // 
        std::getline(file, fullLine);

        // Read ID
        line = fullLine;
        EraseGarbage("char id=", " ");
        ch.id = atoi(line.c_str());

        // Read x
        line = fullLine;
        EraseGarbage("char id=10      x=", " ");
        int x = atoi(line.c_str());

        // Read y
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=", " ");
        int y = atoi(line.c_str());

        // Read width
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=", " ");
        ch.sizeX = static_cast<float>(atof(line.c_str()));

        // Read height
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=0    height=", " ");
        ch.sizeY = static_cast<float>(atof(line.c_str()));

        // Read xoffset
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=0    height=0    xoffset=", " ");
        ch.xOffset = static_cast<float>(atof(line.c_str())) * fScale;

        // Read yoffset
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=0    height=0    xoffset=0    yoffset=", " ");
        ch.yOffset = static_cast<float>(atof(line.c_str())) * fScale;

        // Read xadvance
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=0    height=0    xoffset=0    yoffset=0    xadvance=", " ");
        ch.xStep = static_cast<float>(atof(line.c_str())) * fScale;

        // Calculate UVs
        ch.u0 = x * fWidth;
        ch.v0 = y * fHeight;

        ch.u1 = (x + ch.sizeX) * fWidth;
        ch.v1 = (y + ch.sizeY) * fHeight;

        // Rescale character size
        ch.sizeX *= fScale;
        ch.sizeY *= fScale;

        // Add character
        if( ch.id >= aChars.size() ) {
            aChars.push_back(ch);
        } else {
            aChars[ch.id] = ch;
        }
    }
}

void Font::Bind(UINT slot) {
    tFont->Bind(Shader::Pixel, slot);
    sFont->Bind(Shader::Pixel, slot);
}

void Font::Release() {
    tFont->Release(); // Release font texture
}
