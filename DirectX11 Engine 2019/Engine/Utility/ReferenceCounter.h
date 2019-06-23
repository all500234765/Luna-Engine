#pragma once

class ReferenceCounter {
public:
    ReferenceCounter():
        _mRefCounter(1) {
    }

    inline int GetReferenceCount() { return _mRefCounter; }

    inline void AddReference() { _mRefCounter++; }
    inline bool RemoveReference() { _mRefCounter--; return _mRefCounter == 0; }
private:
    int _mRefCounter;
};
