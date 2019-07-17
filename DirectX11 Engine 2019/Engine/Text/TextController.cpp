#include "TextController.h"

Text* TextController::Build(const char* text, float maxWidth) {
    return mFactory->Build(text, maxWidth);
}

Text* TextController::Build(Text* txt, const char* text, float maxWidth) {
    return mFactory->Build(text, maxWidth);
}

void TextController::Draw(Text* text, float x, float y, float xs, float ys, float ang) {
    gDirectX->gContext->RSSetViewports(1, &mViewPort);

    switch( mFactory->GetHAlignment() ) {
        case TextFlags::TextAlignment_H_Left  : x -= text->GetSizeW();       break;
        case TextFlags::TextAlignment_H_Middle: x -= text->GetSizeW() * .5f; break;
        case TextFlags::TextAlignment_H_Right :                              break;
    }

    switch( mFactory->GetVAlignment() ) {
        case TextFlags::TextAlignment_V_Top   : y -= text->GetSizeH();       break;
        case TextFlags::TextAlignment_V_Center: y -= text->GetSizeH() * .5f; break;
        case TextFlags::TextAlignment_V_Bottom:                              break;
    }
    
    x -= mViewPort.Width / 1024.f;
    //y -= mViewPort.Height;

    mCamera->SetWorldMatrix(DirectX::XMMatrixTranslation(x, y, 0.f) *
                            DirectX::XMMatrixScaling( xs * mInvWidth  * mScale * mCamera->GetAspect(),
                                                     -ys * mInvHeight * mScale, 1.f) *
                            DirectX::XMMatrixRotationAxis({ 0., 0., 1. }, ang));
    mCamera->BuildConstantBuffer();
    mCamera->BindBuffer(Shader::Vertex, 0);

    mFactory->Draw(text);
}
