#include "Material.h"

Material::Material() {
    /*mDiffuse   = new DiffuseMap;
    mNormal    = new NormalMap;
    mRoughness = new RougnessMap;*/
}

void Material::SetDiffuse(DiffuseMap* map) {
    mDiffuse = map;
}

void Material::SetNormal(NormalMap* map) {
    mNormal = map;
}

void Material::SetRoughness(RougnessMap* map) {
    mRoughness = map;
}

void Material::SetSampler(Sampler* sampl) {
    sSampler = sampl;
}

void Material::BindTextures(Shader::ShaderType type, UINT slot) {
    if(   mDiffuse &&   mDiffuse->mTexture->IsCreated() )   mDiffuse->mTexture->Bind(type, slot + 0U);
    if(    mNormal &&    mNormal->mTexture->IsCreated() )    mNormal->mTexture->Bind(type, slot + 1U);
    if( mRoughness && mRoughness->mTexture->IsCreated() ) mRoughness->mTexture->Bind(type, slot + 2U);
    if(   sSampler &&             sSampler->IsCreated() ) sSampler->Bind(type, slot);
}

void Material::Release() {
    if(   mDiffuse &&   mDiffuse->mTexture->IsCreated() )   mDiffuse->mTexture->Release();
    if(    mNormal &&    mNormal->mTexture->IsCreated() )    mNormal->mTexture->Release();
    if( mRoughness && mRoughness->mTexture->IsCreated() ) mRoughness->mTexture->Release();
    if(   sSampler &&             sSampler->IsCreated() ) sSampler->Release();
}
