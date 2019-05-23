#pragma once

#include "DirectXChild.h"

class Query: public DirectXChild {
private:
    ID3D11Query *pQuery;

    // Some queries use bool type instead of UINT64
    // Some use structs
    //using ValueType = UINT64;
    //ValueType mLastValue = 0;
    union {
        bool   bLastValue;
        UINT64 iLastValue;
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT  *tLastValue;
        D3D11_QUERY_DATA_PIPELINE_STATISTICS *pLastValue;
        D3D11_QUERY_DATA_SO_STATISTICS       *sLastValue;
    };

    typedef enum {
        Bool,
        Integer,
        Timestamp, 
        Pipeline, 
        Statictics
    } ValueType;

    const int DataTypeSize[5] = {
        sizeof(bLastValue), sizeof(iLastValue),
        sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT),
        sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS),
        sizeof(D3D11_QUERY_DATA_SO_STATISTICS)
    };

    bool isBeginEnabled  = true;
    ValueType mValueType = Integer;
public:
    void Create(D3D11_QUERY type, D3D11_QUERY_MISC_FLAG misc=(D3D11_QUERY_MISC_FLAG)0);

    void  Begin();
    void* End();

    void Release();

};
