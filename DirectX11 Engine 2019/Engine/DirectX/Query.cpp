#include "pc.h"
#include "Query.h"

void Query::Create(D3D11_QUERY type, D3D11_QUERY_MISC_FLAG misc) {
    D3D11_QUERY_DESC pDesc;
    pDesc.MiscFlags = misc;
    pDesc.Query     = type;

    // Create new query
    auto res = gDirectX->gDevice->CreateQuery(&pDesc, &pQuery);
    if( FAILED(res) ) {
        std::cout << "Failed to create query" << std::endl;
    }

    // Customize class values a bit
    // https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_query
    switch( type ) {
        case D3D11_QUERY_EVENT:
            // When using this type of query, ID3D11DeviceContext::Begin is disabled.
            isBeginEnabled = false;

        case D3D11_QUERY_OCCLUSION_PREDICATE:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM0:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM1:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM2:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM3:
            //using ValueType = typename bool;
            // All of this query types above use bool type
            mValueType = Bool;
            break;

        case D3D11_QUERY_TIMESTAMP_DISJOINT:
            //using ValueType = typename D3D11_QUERY_DATA_TIMESTAMP_DISJOINT;
            mValueType = Timestamp;
            break;

        case D3D11_QUERY_PIPELINE_STATISTICS:
            //using ValueType = typename D3D11_QUERY_DATA_PIPELINE_STATISTICS;
            mValueType = Pipeline;
            break;

        case D3D11_QUERY_SO_STATISTICS:
        case D3D11_QUERY_SO_STATISTICS_STREAM0:
        case D3D11_QUERY_SO_STATISTICS_STREAM1:
        case D3D11_QUERY_SO_STATISTICS_STREAM2:
        case D3D11_QUERY_SO_STATISTICS_STREAM3:
            //using ValueType = typename D3D11_QUERY_DATA_SO_STATISTICS;
            mValueType = Statictics;
            break;
    }

    // Allocate data buffer
    pData = malloc(DataTypeSize[mValueType]);
}

// Begin query event
void Query::Begin() {
    if( isBeginEnabled ) gDirectX->gContext->Begin(pQuery);
}

// End query event
QueryValue Query::End() {
    // End query event
    gDirectX->gContext->End(pQuery);
    
    // A return value of S_OK indicates that the data at pData is available for the calling application to access. 
    // A return value of S_FALSE indicates that the data is not yet available. 
    // If the data is not yet available, the application must call GetData until the data is available.
    while( gDirectX->gContext->GetData(pQuery, &pData, DataTypeSize[mValueType], 0) != S_OK ) { }

    // Cache value
    QueryValue v;
    switch( mValueType ) {
        case Bool      : v.bLastValue =   (bool)pData; break;
        case Integer   : v.iLastValue = (UINT64)pData; break;
        case Timestamp : v.tLastValue = reinterpret_cast<D3D11_QUERY_DATA_TIMESTAMP_DISJOINT*> (&pData); break;
        case Pipeline  : v.pLastValue = reinterpret_cast<D3D11_QUERY_DATA_PIPELINE_STATISTICS*>(&pData); break;
        case Statictics: v.sLastValue = reinterpret_cast<D3D11_QUERY_DATA_SO_STATISTICS*>      (&pData); break;
    }

    // Return result data
    return v;
}

void Query::Release() {
    // Release query
    free(pData);
    if( pQuery ) pQuery->Release();
}
