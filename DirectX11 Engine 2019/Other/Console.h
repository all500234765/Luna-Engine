#pragma once

#include "pc.h"
#include "HighLevel/DirectX/Utlities.h"

// 
template<typename T, typename S>
class ConsoleVariableParent {
protected:
    static uint32_t gAtomicRefCounter;
    static std::vector<uint64_t> gRefList;

    friend class Console;

public:
    void AddRef() { gAtomicRefCounter++; gRefList.push_back((uint64_t)&this[0]); }
    
    ~ConsoleVariableParent() {
        gAtomicRefCounter--;

        auto index = std::remove(gRefList.begin(), gRefList.end(), (uint64_t)&this[0]);
        if( index != gRefList.cend() ) gRefList.erase(index, gRefList.end());
    };

    virtual S operator()() = 0;
    virtual void operator=(S val) = 0;
};

template<typename T, typename S>
uint32_t ConsoleVariableParent<T, S>::gAtomicRefCounter = 0;

template<typename T, typename S>
std::vector<uint64_t> ConsoleVariableParent<T, S>::gRefList {};

// 
template<typename T>
class ConsoleVariableReal: ConsoleVariableParent<ConsoleVariableReal<T>, T> {
private:
    const char* mName{};
    T mMin{}, mMax{}, mCurrent{};


public:
    ConsoleVariableReal(const char* name, T def, T min, T max): mName(name), mMin(min), mMax(max), mCurrent(def) {
        this->AddRef();
    }

    // Get
    T operator()() override { return mCurrent; }

    // Set
    void operator=(T val) override { mCurrent = LunaEngine::Math::clamp(val, mMin, mMax); }



};

class ConsoleVariableString: ConsoleVariableParent<ConsoleVariableString, const char*> {
private:
    const char* mName{};
    const char* mCurrent{};


public:
    ConsoleVariableString(const char* name, const char* def): mName(name), mCurrent(def) {
        this->AddRef();
    }

    // Get
    const char* operator()() override { return mCurrent; }

    // Set
    void operator=(const char* val) override { mCurrent = val; }



};

// 
class Console {
private:


public:
    //template<typename T, typename S>
    /*void Print() {
        auto list = ConsoleVariableParent<float, float>::gRefList;

        for( auto e : list ) {
            auto q = ((ConsoleVariableParent<float, float>*)e);
            printf_s("%v\n", q());
        }
    }*/


};

#define MakeConsoleVarReal(name, T, default, _min, _max) ConsoleVariableReal<T> name(#name, default, _min, _max);
#define MakeConsoleVarString(name, default) ConsoleVariableString name(#name, default);

MakeConsoleVarReal(test_var, float, 0.f, -1.f, 1.f);
MakeConsoleVarString(test_var2, "sosi");


float q = test_var();

