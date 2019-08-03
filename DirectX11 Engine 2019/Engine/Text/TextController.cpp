#include "TextController.h"

Text* TextController::Build(const char* text, float maxWidth) {
    return mFactory->Build(text, maxWidth);
}

Text* TextController::Build(Text* txt, const char* text, float maxWidth) {
    return mFactory->Build(text, maxWidth);
}

void TextController::Draw(Text* text, float x, float y, float xs, float ys, float ang) {
    // Set view port
    gDirectX->gContext->RSSetViewports(1, &mViewPort);

    // Apply alignment
    float fScale = mFactory->GetFont()->GetScale();
    switch( mFactory->GetHAlignment() ) {
        case TextFlags::TextAlignment_H_Left  :                                                      break;
        case TextFlags::TextAlignment_H_Middle: x -= text->GetSizeW() * .5f * fScale * xs / mXScale; break;
        case TextFlags::TextAlignment_H_Right : x -= text->GetSizeW() * fScale * mXScale;            break;
    }

    switch( mFactory->GetVAlignment() ) {
        case TextFlags::TextAlignment_V_Top   :                                                 break;
        case TextFlags::TextAlignment_V_Center: y -= text->GetSizeH() * .5f * fScale * mYScale; break;
        case TextFlags::TextAlignment_V_Bottom: y -= text->GetSizeH() * fScale * mYScale;       break;
    }

    // Update matrices
    mCamera->SetWorldMatrix(DirectX::XMMatrixScaling(xs * mInvWidth  * mScale * mCamera->GetAspect(),
                                                     ys * mInvHeight * mScale, 1.f) *
                            DirectX::XMMatrixTranslation(x, y, 0.f) *
                            DirectX::XMMatrixRotationAxis({ 0., 0., 1. }, ang));
    mCamera->BuildProj();
    mCamera->BuildView();
    mCamera->BuildConstantBuffer();
    mCamera->BindBuffer(Shader::Vertex, 0);

    // Render text
    mFactory->Draw(text);
}
