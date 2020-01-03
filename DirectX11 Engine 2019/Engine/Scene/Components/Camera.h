#include "ShaderDefines.h"

#ifndef __cplusplus // HLSL

// Oh fuck...
    #ifndef ___CAMERA_BUFF
        #define ___CAMERA_BUFF 0
        #define NAME___(name) name##0
    #else
        #undef NAME___
        
        #if ___CAMERA_BUFF == 0
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 1
            #define NAME___(name) name##1
        #elif ___CAMERA_BUFF == 1
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 2
            #define NAME___(name) name##2
        #elif ___CAMERA_BUFF == 2
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 3
            #define NAME___(name) name##3
        #elif ___CAMERA_BUFF == 3
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 4
            #define NAME___(name) name##4
        #elif ___CAMERA_BUFF == 4
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 5
            #define NAME___(name) name##5
        #elif ___CAMERA_BUFF == 5
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 6
            #define NAME___(name) name##6
        #elif ___CAMERA_BUFF == 6
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 7
            #define NAME___(name) name##7
        #elif ___CAMERA_BUFF == 7
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 8
            #define NAME___(name) name##8
        #elif ___CAMERA_BUFF == 8
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 9
            #define NAME___(name) name##9
        #elif ___CAMERA_BUFF == 9
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 10
            #define NAME___(name) name##10
        #elif ___CAMERA_BUFF == 10
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 11
            #define NAME___(name) name##11
        #elif ___CAMERA_BUFF == 11
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 12
            #define NAME___(name) name##12
        #elif ___CAMERA_BUFF == 12
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 13
            #define NAME___(name) name##13
        #elif ___CAMERA_BUFF == 13
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 14
            #define NAME___(name) name##14
        #elif ___CAMERA_BUFF == 14
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 15
            #define NAME___(name) name##15
        #elif ___CAMERA_BUFF == 15
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 16
            #define NAME___(name) name##16
        #elif ___CAMERA_BUFF == 16
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 17
            #define NAME___(name) name##17
        #elif ___CAMERA_BUFF == 17
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 18
            #define NAME___(name) name##18
        #elif ___CAMERA_BUFF == 18
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 19
            #define NAME___(name) name##19
        #elif ___CAMERA_BUFF == 19
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 20
            #define NAME___(name) name##20
        #elif ___CAMERA_BUFF == 20
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 21
            #define NAME___(name) name##21
        #elif ___CAMERA_BUFF == 21
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 22
            #define NAME___(name) name##22
        #elif ___CAMERA_BUFF == 22
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 23
            #define NAME___(name) name##23
        #elif ___CAMERA_BUFF == 23
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 24
            #define NAME___(name) name##24
        #elif ___CAMERA_BUFF == 24
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 25
            #define NAME___(name) name##25
        #elif ___CAMERA_BUFF == 25
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 26
            #define NAME___(name) name##26
        #elif ___CAMERA_BUFF == 26
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 27
            #define NAME___(name) name##27
        #elif ___CAMERA_BUFF == 27
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 28
            #define NAME___(name) name##28
        #elif ___CAMERA_BUFF == 28
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 29
            #define NAME___(name) name##29
        #elif ___CAMERA_BUFF == 29
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 30
            #define NAME___(name) name##30
        #elif ___CAMERA_BUFF == 30
            #undef ___CAMERA_BUFF
            #define ___CAMERA_BUFF 31
            #define NAME___(name) name##31
        #endif
    #endif
    
#else // C++
    #define NAME___(name) name
#endif // __cplusplus

mfloat4x4 NAME___(mView);
mfloat4x4 NAME___(mProj);
mfloat4x4 NAME___(mInvView);
mfloat4x4 NAME___(mInvProj);
uint32_t NAME___(bOrtho); // Bool
float NAME___(fWidth);
float NAME___(fHeight);
float NAME___(fNear);
float NAME___(fFar);
float NAME___(fFOV_X);
float NAME___(fFOV_Y);
float NAME___(fAspect);

#undef NAME___
