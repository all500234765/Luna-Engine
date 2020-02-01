#pragma once

#include "pc.h"
#include "DirectXChild.h"


// Some queries use bool type instead of UINT64
// Some use structs
//using ValueType = UINT64;
//ValueType mLastValue = 0;
union QueryValue {
    bool   bLastValue;
    UINT64 iLastValue;
    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT  *tLastValue;
    D3D11_QUERY_DATA_PIPELINE_STATISTICS *pLastValue;
    D3D11_QUERY_DATA_SO_STATISTICS       *sLastValue;
};

class Query: public DirectXChild {
private:
    ID3D11Query *pQuery;

    typedef enum {
        Bool,
        Integer,
        Timestamp, 
        Pipeline, 
        Statictics
    } ValueType;

    const int DataTypeSize[5] = {
        sizeof(QueryValue::bLastValue), sizeof(QueryValue::iLastValue),
        sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT),
        sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS),
        sizeof(D3D11_QUERY_DATA_SO_STATISTICS)
    };

    bool isBeginEnabled  = true;
    ValueType mValueType = Integer;
    void* pData;

public:
    void Create(D3D11_QUERY type, D3D11_QUERY_MISC_FLAG misc=(D3D11_QUERY_MISC_FLAG)0);

    void       Begin();
    QueryValue End();

    void Release();

};
