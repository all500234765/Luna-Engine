#include "DataBuffer.h"

#include <memory>

DataBuffer::DataBuffer() {
    data = new char();
}

void DataBuffer::Write(int value) {
    if( tell + sizeof(value) > size ) Resize(sizeof(value), true);
    memcpy(&data[tell], &value, sizeof(value));
    tell += sizeof(value);
}

char* DataBuffer::GetBufferPointer() {
    return &data[0];
}

size_t DataBuffer::GetBufferSize() {
    return size;
}

void DataBuffer::Release() {
    delete[] data;
}

void DataBuffer::Resize(size_t sz, bool rel) {
    size_t old = size;
    if( rel ) {
        // Relative
        size += sz;
    } else {
        // Absolute
        size = sz;
    }

    // Temp buffer
    char* Temp = (char*)malloc(old);
    memcpy(&Temp, &data[0], old);

    // Create new buffer
    //free(&data[0]);
    delete[] data;
    data = (char*)malloc(size);

    // Copy data back
    memcpy(&data[0], &Temp[0], old);

    // Free memory
    delete[] Temp;
}
