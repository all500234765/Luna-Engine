#pragma once

#include "Engine/DirectX/DirectXChild.h"
#include "Engine/Camera/Camera.h"

#include "TextFactory.h"
#include "Text.h"

class TextController: public DirectXChild {
private:
    TextFactory *mFactory;
    Camera *mCamera;
    float mScale, mInvWidth, mInvHeight;
    CameraConfig mCameraCfg;
    D3D11_VIEWPORT mViewPort;

public:
    TextController(TextFactory* fac, float w=1024.f, float h=540.f, float scale=24.f): 
            mFactory(fac), mScale(scale) {
        mCamera = new Camera();
        SetSize(w, h);
    };
    
    inline void SetScale(float v) { mScale = v; }
    inline float GetScale() const { return mScale; }

    void SetSize(float w, float h) {
        mCameraCfg.fAspect = w / h;
        mCameraCfg.FOV = 90.f;
        mCameraCfg.fNear = .1f;
        mCameraCfg.fFar = 1.f;

        mCamera->SetParams(mCameraCfg);
        mCamera->BuildProj();
        mCamera->BuildView();
        mCamera->SetWorldMatrix(DirectX::XMMatrixIdentity());
        mCamera->BuildConstantBuffer();

        mViewPort.MinDepth = 0.f;
        mViewPort.MaxDepth = 1.f;
        mViewPort.TopLeftX = 0;
        mViewPort.TopLeftY = 0;
        mViewPort.Width  = w;
        mViewPort.Height = h;

        mInvWidth  = 1.f / w;
        mInvHeight = 1.f / h;
    }

    inline void SetFactory(TextFactory* f) { mFactory = f; }
    inline void SetFont(Font* f) { mFactory->SetFont(f); }

    Text* Build(const char* text, float maxWidth=-1.f);
    Text* Build(Text* txt, const char* text, float maxWidth=-1.f);

    void Draw(Text* text, float x=0.f, float y=0.f, float xs=1.f, float ys=1.f, float ang=0.f);
};
