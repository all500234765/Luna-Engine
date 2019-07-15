#include "Font.h"

void Font::Bind(UINT slot) {
    tFont->Bind(Shader::Pixel, slot);
    sFont->Bind(Shader::Pixel, slot);
}
