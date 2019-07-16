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
    
    // Skip some garbage
    std::getline(file, line);

    // GB
    auto EraseGarbage = [&line](const char* begin, const char* end, int offset=0) {
        line.erase(offset, strlen(begin));
        int p0 = line.find(end);
        if( p0 == -1 ) { return; }
        line.erase(p0, line.length() - p0);
    };

    // Get some data
    std::getline(file, fullLine);

    // Get lineHeight
    line = fullLine;
    EraseGarbage("common lineHeight=", " ");
    mHeight = atoi(line.c_str());

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
        int width = atoi(line.c_str());

        // Read height
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=0    height=", " ");
        int height = atoi(line.c_str());

        // Read xoffset
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=0    height=0    xoffset=", " ");
        ch.xOffset = atoi(line.c_str());

        // Read yoffset
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=0    height=0    xoffset=0    yoffset=", " ");
        ch.yOffset = atoi(line.c_str());

        // Read xadvance
        line = fullLine;
        EraseGarbage("char id=10      x=0    y=0    width=0    height=0    xoffset=0    yoffset=0    xadvance=", " ");
        ch.xStep = atoi(line.c_str());

        // Calculate UVs
        ch.u0 = x / tFont->GetWidth();
        ch.v0 = y / tFont->GetHeight();

        ch.u1 = (x + width ) / tFont->GetWidth();
        ch.v1 = (y + height) / tFont->GetWidth();

        // Add character
        if( ch.id >= aChars.size() ) {
            aChars.push_back(ch);
        } else {
            aChars[ch.id] = ch;
        }
    }
    
    // Load texture
    /*tinyddsloader::DDSFile DDS;
    auto res = DDS.Load((std::string(FONT_DIRECTORY) + std::string(dds)).c_str());
    if( res != tinyddsloader::Result::Success ) {
        std::cout << "[Font]: Error loading dds file! (fname=" << dds << ", error=" << res << ")" << std::endl;
        return;
    }

    const tinyddsloader::DDSFile::ImageData* data = DDS.GetImageData();

    // Create texture
    tFont = new Texture();
    tFont->Create(data->m_mem, (DXGI_FORMAT)DDS.GetFormat(), DDS.GetBitsPerPixel(DDS.GetFormat()));

    // Load font data
    std::ifstream file(std::string(FONT_DIRECTORY) + std::string(fontFile));
    if( file.fail() ) {
        std::cout << "[Font]: Error loading font data file! (fname=" << fontFile << ")" << std::endl;
        return;
    }

    char temp;
    while( !file.eof() ) {
        

    }*/
}

void Font::Bind(UINT slot) {
    tFont->Bind(Shader::Pixel, slot);
    sFont->Bind(Shader::Pixel, slot);
}

void Font::Release() {
    tFont->Release(); // Release font texture
}
