#include "Text.h"

void Text::Clear() {
    data->Release();
}

void Text::Release() {
    Clear();
}

void Text::Draw() {
    data->Bind();
    data->Render();
}
