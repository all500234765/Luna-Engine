#pragma once

// Ptr
template<typename T>
class SingletonPtr {
protected:
    static T *gInstance;

private:


public:


};

template<typename T>
T *SingletonPtr<T>::gInstance = 0;

// Non-ptr
template<typename T>
class Singleton {
protected:
    static T gInstance;

private:


public:


};

template<typename T>
T Singleton<T>::gInstance = (T)0;
