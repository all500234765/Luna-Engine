#pragma once

template<typename T>
class PipelineState {
protected:
    static T *gState;
    static T *gStateOld;

public:
    bool IsBound() const { return (T*)this == gState; }

    static T* Current() { return gState;                     }
    static void Push()  { gStateOld = gState;                } // Store current state
    static void Pop()   { if( gStateOld ) gStateOld->Bind(); } // Re-Store old state
};

template<typename T>
T *PipelineState<T>::gState = 0;

template<typename T>
T *PipelineState<T>::gStateOld = 0;

// TODO: Support up to 4 stored states
