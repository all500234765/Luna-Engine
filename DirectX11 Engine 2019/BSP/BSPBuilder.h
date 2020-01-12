#pragma once

#include "pc.h"

#include "Engine/DirectX/StructuredBuffer.h"
#include "Engine/DirectX/ConstantBuffer.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/IndexBuffer.h"
#include "Engine/Extensions/Safe.h"
#include "Engine/ScopedMapper.h"
#include "BSPTree.h"

class BSPBuilder {
private:
    struct Entry {
        VertexBuffer *VB;
        IndexBuffer *IB;
        mfloat4x4 mWorld;
    };

    struct Pivot {
        uint front, back;
    };

    // Up to 32 buffers in queue
    std::array<Entry, 32> BufferQueue;

    // Staging buffers
    std::array<Entry, 96> BufferStaging;

    // Current process
    Entry CurrentProcess;
    StructuredBuffer<Pivot> *sbPivot;

    uint32_t StagingPos{};
    bool bProcessing{};

    void CombineBuffers() {
        // Find total element number, and store all elements
        uint32_t icount = 0, vcount = 0;
        std::vector<float3> vertex;
        std::vector<uint> index;

        for( uint32_t i = 0; i < StagingPos; i++ ) {
            icount += BufferStaging[i].IB->GetNumber();
            vcount += BufferStaging[i].VB->GetNumber();

            vertex.reserve(vcount);
            index.reserve(icount);

            // Copy Buffers
            {
                ScopeMapIndexBuffer sib(BufferStaging[i].IB);
                for( uint32_t j = 0, n = BufferStaging[i].IB->GetNumber(); j < n; j++ )
                    index.push_back(sib.data[j]);
            }

            {
                ScopeMapVertexBuffer<float3> svb(BufferStaging[i].VB);

                for( uint32_t j = 0, n = BufferStaging[i].VB->GetNumber(); j < n; j++ ) {
                    mfloat4 v = DirectX::XMVector4Transform(mfloat4({ svb.data[j].x, svb.data[j].y, svb.data[j].z, 1.f }),
                                                            BufferStaging[i].mWorld);
                    vertex.push_back({ v.m128_f32[0], v.m128_f32[1], v.m128_f32[2] });
                }
            }
        }

        // Create buffers
        CurrentProcess.IB->CreateDefault(icount, index.data());
        CurrentProcess.VB->CreateDefault(icount, sizeof(float3), &vertex[0]);
    }

public:
    BSPBuilder() {
        CurrentProcess.IB = new IndexBuffer;
        CurrentProcess.VB = new VertexBuffer;
        sbPivot = new StructuredBuffer<Pivot>;


    }

    ~BSPBuilder() {
        SAFE_RELEASE(CurrentProcess.IB);
        SAFE_RELEASE(CurrentProcess.VB);
        SAFE_RELEASE(sbPivot);
    }

    void Insert(VertexBuffer* VB, IndexBuffer* IB, const mfloat4x4& world) {
        if( bProcessing ) {
            // Enqueue

        } else {
            BufferStaging[StagingPos] = { VB, IB, world };
        }
    }

    BSPTree&& Build() {
        if( bProcessing ) {
            printf_s("[BSP::Build]: Building already began!\n");
            return {};
        }

        if( StagingPos == 0 ) {
            printf_s("[BSP::Build]: No mesh entries provided!\n");
            return {};
        }

        // Combine buffers
        printf_s("[BSP::Build]: Combining buffers...\n");
        CombineBuffers();

        // 
        printf_s("[BSP::Build]: Evaluating pivot...\n");

        // 
        BSPTree tree{};






        return std::move(tree);
    }
};