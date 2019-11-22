#include "../Common/Utils.hlsli"

struct BVHNode {
    float3 min;
    uint LeftChildIndex;
    float3 max;
    uint RightChildIndex;
};

ByteAddressBuffer       _PrimitiveCounter  : register(t0);
StructuredBuffer<uint>  _PrimitiveIDBuffer : register(t1);
StructuredBuffer<float> _PrimitiveMorton   : register(t2);

RWStructuredBuffer<BVHNode> _BVHNode   : register(u0);
RWStructuredBuffer<uint>    _BVHParent : register(u1);
RWStructuredBuffer<uint>    _BVHFlag   : register(u2);

int CountLeadingZeroes(uint num) {
    return 31 - firstbithigh(num);
}

int GetLongestCommonPrefix(uint indexA, uint indexB, uint elementCount) {
    if( indexA >= elementCount || indexB >= elementCount ) {
        return -1;
    } else {
        uint mortonCodeA = (float)_PrimitiveMorton[_PrimitiveIDBuffer[indexA]];
        uint mortonCodeB = (float)_PrimitiveMorton[_PrimitiveIDBuffer[indexB]];
        if( mortonCodeA != mortonCodeB ) {
            return CountLeadingZeroes(mortonCodeA ^ mortonCodeB);
        } else {
            // TODO: Technically this should be primitive ID
            return CountLeadingZeroes(indexA ^ indexB) + 31;
        }
    }
}

uint2 DetermineRange(uint idx, uint elementCount) {
    int d = GetLongestCommonPrefix(idx, idx + 1, elementCount) - GetLongestCommonPrefix(idx, idx - 1, elementCount);
    d = clamp(d, -1, 1);
    int minPrefix = GetLongestCommonPrefix(idx, idx - d, elementCount);

    // TODO: Consider starting this at a higher number
    int maxLength = 2;
    while( GetLongestCommonPrefix(idx, idx + maxLength * d, elementCount) > minPrefix ) {
        maxLength *= 4;
    }

    int length = 0;
    for( int t = maxLength / 2; t > 0; t /= 2 ) {
        if( GetLongestCommonPrefix(idx, idx + (length + t) * d, elementCount) > minPrefix ) {
            length = length + t;
        }
    }

    int j = idx + length * d;
    return uint2(min(idx, j), max(idx, j));
}

int FindSplit(int first, uint last, uint elementCount) {
    int commonPrefix = GetLongestCommonPrefix(first, last, elementCount);
    int split = first;
    int step = last - first;

    do {
        step = (step + 1) >> 1;
        int newSplit = split + step;

        if( newSplit < last ) {
            int splitPrefix = GetLongestCommonPrefix(first, newSplit, elementCount);
            if( splitPrefix > commonPrefix )
                split = newSplit;
        }
    } while( step > 1 );

    return split;
}

static const int BVH_BUILDER_GROUPSIZE = 64;

[numthreads(BVH_BUILDER_GROUPSIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    const uint idx = DTid.x;
    const uint primitiveCount = _PrimitiveCounter.Load(0);

    if( idx < primitiveCount - 1 ) {
        uint2 range = DetermineRange(idx, primitiveCount);
        uint first = range.x;
        uint last = range.y;

        uint split = FindSplit(first, last, primitiveCount);

        uint internalNodeOffset = 0;
        uint leafNodeOffset = primitiveCount - 1;
        uint childAIndex;
        if( split == first )
            childAIndex = leafNodeOffset + split;
        else
            childAIndex = internalNodeOffset + split;

        uint childBIndex;
        if( split + 1 == last )
            childBIndex = leafNodeOffset + split + 1;
        else
            childBIndex = internalNodeOffset + split + 1;

        // write to parent:
        _BVHNode[idx].LeftChildIndex = childAIndex;
        _BVHNode[idx].RightChildIndex = childBIndex;

        // write to children:
        _BVHParent[childAIndex] = idx;
        _BVHParent[childBIndex] = idx;

        // Reset bvh node flag (only internal nodes):
        _BVHFlag[idx] = 0;
    }
}
