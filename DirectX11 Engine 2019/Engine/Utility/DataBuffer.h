#pragma once

#include "pc.h"

class DataBuffer {
private:
    char* data;
    size_t size; // Bytes
    int tell = 0;

public:
    DataBuffer();
    
    //template<typename T>
    void Write(int value);

    char* GetBufferPointer();
    size_t GetBufferSize();
    void Release();
    void Resize(size_t sz, bool rel);
};

/*template<typename T>
inline void DataBuffer::Write(T value) {
    if( tell + sizeof(value) > size ) Resize(sizeof(value), true);
    memcpy(&data[tell], &value, sizeof(value));
    tell += sizeof(value);
}*/
