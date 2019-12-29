//#pragma once

// Fuck...
//#pragma once
#ifndef ___SAFE_GUARD_TYPE_OPERATORS2___3333
#define ___SAFE_GUARD_TYPE_OPERATORS2___3333

//////////////////////////////////////// float3 x float3
#ifdef _____________TYPE_FLOAT_OPERATORS22____3333
    #define OPERATOR(op) \
    float3 __cdecl operator op(float3 lhs, float3 rhs) { \
        return { lhs.x op rhs.x, lhs.y op rhs.y, lhs.z op rhs.z}; \
    }
#else 
    #define OPERATOR(op) \
    float3 __cdecl operator op(float3 lhs, float3 rhs);
#endif

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

//////////////////////////////////////// float3 x float
#ifdef _____________TYPE_FLOAT_OPERATORS22____3333
    #define OPERATOR(op) \
    float3 __cdecl operator op(float3 lhs, float rhs) { \
        return { lhs.x op rhs, lhs.y op rhs, lhs.z op rhs }; \
    }
#else
    #define OPERATOR(op) \
    float3 __cdecl operator op(float3 lhs, float rhs);
#endif

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR
#endif // ___SAFE_GUARD_TYPE_OPERATORS2___3333

#ifdef ___SAFE_GUARD_TYPE_OPERATORS___
#define ___SAFE_GUARD_TYPE_OPERATORS___
//////////////////////////////////////// float3 x float3
#define OPERATOR(op) \
float3 operator op(float3 lhs, float3 rhs) { \
    return { lhs.x op rhs.x, lhs.y op rhs.y, lhs.z op rhs.z}; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

#define OPERATOR(op) \
float3 operator op(float3 lhs, float3 rhs) { \
    lhs.x op rhs.x; \
    lhs.y op rhs.y; \
    lhs.z op rhs.z; \
    return lhs; \
}

OPERATOR(+= )
OPERATOR(-= )
OPERATOR(*= )
OPERATOR(/= )

#undef OPERATOR

//////////////////////////////////////// float4 x float4
#define OPERATOR(op) \
float4 operator op(float4 lhs, float4 rhs) { \
    return { lhs.x op rhs.x, lhs.y op rhs.y, lhs.z op rhs.z, lhs.w op rhs.w }; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

#define OPERATOR(op) \
float4 operator op(float4 lhs, float4 rhs) { \
    lhs.x op rhs.x; \
    lhs.y op rhs.y; \
    lhs.z op rhs.z; \
    lhs.w op rhs.w; \
    return lhs; \
}

OPERATOR(+= )
OPERATOR(-= )
OPERATOR(*= )
OPERATOR(/= )

#undef OPERATOR

//////////////////////////////////////// float2 x float2
#define OPERATOR(op) \
float2 operator op(float2 lhs, float2 rhs) { \
    return { lhs.x op rhs.x, lhs.y op rhs.y }; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

#define OPERATOR(op) \
float2 operator op(float2 lhs, float2 rhs) { \
    lhs.x op rhs.x; \
    lhs.y op rhs.y; \
    return lhs; \
}

OPERATOR(+= )
OPERATOR(-= )
OPERATOR(*= )
OPERATOR(/= )

#undef OPERATOR

//////////////////////////////////////// float3 x float
#define OPERATOR(op) \
float3 operator op(float3 lhs, float rhs) { \
    return { lhs.x op rhs, lhs.y op rhs, lhs.z op rhs }; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

#define OPERATOR(op) \
float3 operator op(float3 lhs, float rhs) { \
    lhs.x op rhs; \
    lhs.y op rhs; \
    lhs.z op rhs; \
    return lhs; \
}

OPERATOR(+= )
OPERATOR(-= )
OPERATOR(*= )
OPERATOR(/= )

#undef OPERATOR

//////////////////////////////////////// float2 x float
#define OPERATOR(op) \
float2 operator op(float2 lhs, float rhs) { \
    return { lhs.x op rhs, lhs.y op rhs }; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

#define OPERATOR(op) \
float2 operator op(float2 lhs, float rhs) { \
    lhs.x op rhs; \
    lhs.y op rhs; \
    return lhs; \
}

OPERATOR(+= )
OPERATOR(-= )
OPERATOR(*= )
OPERATOR(/= )

#undef OPERATOR

//////////////////////////////////////// float4 x float
#define OPERATOR(op) \
float4 operator op(float4 lhs, float rhs) { \
    return { lhs.x op rhs, lhs.y op rhs, lhs.z op rhs, lhs.w op rhs }; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

#define OPERATOR(op) \
float4 operator op(float4 lhs, float rhs) { \
    lhs.x op rhs; \
    lhs.y op rhs; \
    lhs.z op rhs; \
    lhs.w op rhs; \
    return lhs; \
}

OPERATOR(+= )
OPERATOR(-= )
OPERATOR(*= )
OPERATOR(/= )

#undef OPERATOR


//////////////////////////////////////// float x float4
#define OPERATOR(op) \
float4 operator op(float rhs, float4 lhs) { \
    return { lhs.x op rhs, lhs.y op rhs, lhs.z op rhs, lhs.w op rhs }; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

//////////////////////////////////////// float x float3
#define OPERATOR(op) \
float3 operator op(float rhs, float3 lhs) { \
    return { lhs.x op rhs, lhs.y op rhs, lhs.z op rhs }; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

//////////////////////////////////////// float x float2
#define OPERATOR(op) \
float2 operator op(float rhs, float2 lhs) { \
    return { lhs.x op rhs, lhs.y op rhs }; \
}

OPERATOR(+)
OPERATOR(-)
OPERATOR(*)
OPERATOR(/ )

#undef OPERATOR

#endif // ___SAFE_GUARD_TYPE_OPERATORS___
