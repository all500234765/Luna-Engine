#pragma once

#include "Texture.h"

struct TextureMap {

};

struct DiffuseMap: TextureMap {
    Texture *mTexture;
    // Gamma, etc...
};

struct NormalMap: TextureMap {
    Texture *mTexture;
    float fStrength = 1.f;
};

struct RougnessMap: TextureMap {
    Texture *mTexture;
    float fRoughness = .5f;
};
