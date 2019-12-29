#pragma once

#include "pc.h"
#include "Engine/DirectX/DirectXChild.h"

struct AtlasRect {
    int x, y, w, h;

    AtlasRect(): x(0), y(0), w(0), h(0) {};
    AtlasRect(const int x, const int y, const int w, const int h): x(x), y(y), w(w), h(h) {};
    AtlasRect(const int x, const int y, const UINT w, const UINT h): x(x), y(y), w(w), h(h) {};

    /*int area()      const { return w * h; }
    int perimeter() const { return 2 * (w + h); }*/

    bool Fits(int w2, int h2) const {
        return (w2 <= w) && (h2 <= h);
    }
    bool Perfect(int w2, int h2) const { return (w2 == w) && (h2 == h); }
};

// Atlas generates single big texture
class Atlas: public DirectXChild {
private:
    struct Node {
        Node* mChild[2] = { nullptr, nullptr };
        AtlasRect mRect;
        int mImageID = -1;

        Node* Insert(int w, int h, int mImageID);
        void Release();
    };

    ID3D11Texture2D *pTexture;
    ID3D11ShaderResourceView *pSRV;

    UINT mWidth ;
    UINT mHeight;

    std::vector<AtlasRect> mEmpty;
    std::vector<AtlasRect> mUVMap;

    Node *mRoot;
public:
    Atlas(int w, int h): mWidth(w), mHeight(h) {
        mEmpty.push_back({0, 0, mWidth, mHeight});

        mRoot = new Node();
        mRoot->mImageID = -1;
        mRoot->mRect = { 0, 0, mWidth, mHeight }; // Set node size
    };

    void Resize(int w, int h);
    void Scale(int sx, int sy);

    size_t Add(int w, int h);

    void Release();
};
