#include "Atlas.h"

size_t Atlas::Add(int w, int h) {
    // Try adding image
    size_t ImageID = mUVMap.size();
    Node* res = mRoot->Insert(w, h, ImageID);

    // Done
    if( res != nullptr && res->mImageID == ImageID ) {
        res->mImageID = ImageID;
        mUVMap.push_back(res->mRect);
        return ImageID;
    }

    // Can't add
    return -1;

    //AtlasRect* r;
    //int index = mEmpty.size() - 1;
    //for( auto it = mEmpty.end(); it != mEmpty.begin(); --it ) {
    //    r = &(*it);
    //    index--;
    //    if( r->w > w && r->h > h ) {
    //        // Found needed empty rectangle
    //        break;
    //    }
    //}
    //
    //// Calculate splits
    //
    //
    //// Swap and pop
    //mEmpty[index] = mEmpty.back();
    //mEmpty.pop_back();
    //
    //// 
    //
}

void Atlas::Release() {
    if( pTexture ) pTexture->Release();
    if( pSRV ) pSRV->Release();
    mRoot->Release();
}

Atlas::Node* Atlas::Node::Insert(int w, int h, int ImageID) {
    // Image doesn't exists
    if( ImageID == -1 ) { return nullptr; }

    // Not a leaf
    if( mChild[0] != nullptr && mChild[1] != nullptr ) {
        // Try inserting into 1st child
        Node* n = mChild[0]->Insert(w, h, ImageID);
        if( n != nullptr ) {
            n->mImageID = ImageID;
            return n;
        }

        // No room. Try inserting into 2nd
        return mChild[1]->Insert(w, h, ImageID);
    } else {
        // Already have image here
        if( mImageID != -1 ) { return nullptr; }

        // Too small
        if( !mRect.Fits(w, h) ) { return nullptr; }

        // Perfect fit
        if( mRect.Perfect(w, h) ) {
            mImageID = ImageID;
            return this;
        }

        // Create some kids
        mChild[0] = new Node();
        mChild[1] = new Node();

        // Split direction
        int dw = mRect.w - w;
        int dh = mRect.h - h;

        // 
        if( dw > dh ) {
            mChild[0]->mRect = { mRect.x, mRect.y, mRect.x + mRect.w - 1, mRect.y + mRect.h };
            mChild[1]->mRect = { mRect.x + mRect.w, mRect.y, mRect.x + mRect.w, mRect.y + mRect.h };
        } else {
            mChild[0]->mRect = { mRect.x, mRect.y, mRect.x + mRect.w, mRect.y + mRect.h - 1 };
            mChild[1]->mRect = { mRect.x, mRect.y + mRect.h, mRect.x + mRect.w, mRect.y + mRect.h };
        }

        // Insert into 1st child
        return mChild[0]->Insert(w, h, ImageID);
    }

    return nullptr;
}

void Atlas::Node::Release() {
    if( mChild[0] != nullptr ) { mChild[0]->Release(); }
    if( mChild[1] != nullptr ) { mChild[1]->Release(); }
}
