#pragma once

#include "pc.h"

// https://en.wikipedia.org/wiki/CPUID#EAX=0:_Highest_Function_Parameter_and_Manufacturer_ID

enum CPUVendor {
    CPU_Unknown, 
    CPU_Intel, 
    CPU_AMD, 
    CPU_Nvidia,  // I want to believe
    
    // Virtual
    CPU_Microsoft, 
    CPU_Parallels, 
    CPU_VMWare, 

};

static const char* CPUVendorStr[] = {
    "Unknown", "Intel", "AMD", "Nvidia", 
    "Microsoft", "Parallels desktop", "VMWare"
};

class CPUID {
private:
    uint32_t regs[4];
    uint32_t mLast = -1;
    static CPUVendor gCPUVendor;
    static bool gVirtual;

public:
    CPUID() {};

    void PrintBasicInfo() {
        Call(0);

        float BaseFreq = 0.f, MaxFreq = 0.f, BusFreq = 0.f;
        if( LargestParam() >= 0x16 ) {
            Call(0x16);
            BaseFreq = (float)EAX() / 1000.f;
            MaxFreq  = (float)EBX() / 1000.f;
            BusFreq  = (float)ECX() / 1000.f;
        } else {
            // TODO: Impl work around for older cpus

        }
        
        std::string vendor = Vendor();
        printf_s("CPU: %s[%s] @ %1.2f-%1.2f GHz / %1.2f GHz\n", 
                 vendor.c_str(), CPUVendorStr[gCPUVendor], BaseFreq, MaxFreq, BusFreq);
    }

    CPUVendor GetVendor() const { return gCPUVendor; }
    bool IsRanOnVirtual() const { return gVirtual; }

    void Call(uint32_t i) {
        if( i != mLast ) {
            __cpuid((int*)regs, (int)i);
            mLast = i;
        }
    }

    const uint32_t &EAX() const { return regs[0]; }
    const uint32_t &EBX() const { return regs[1]; }
    const uint32_t &ECX() const { return regs[2]; }
    const uint32_t &EDX() const { return regs[3]; }

    std::string Vendor() {
        Call(0);
        gVirtual = false;

        std::string vendor;
        vendor += std::string((const char *)&EBX(), 4);
        vendor += std::string((const char *)&EDX(), 4);
        vendor += std::string((const char *)&ECX(), 4);

        // Find vendor ID
        if( !vendor.compare("AMDisbetter!") 
         || !vendor.compare("AMDisbetter!") 
         || !vendor.compare("HygonGenuine")
         || !vendor.compare("NexGenDriven") ) {
            gCPUVendor = CPU_AMD;
        } else if( !vendor.compare("GenuineIntel") ) {
            gCPUVendor = CPU_Intel;
        } else if( !vendor.compare("Microsoft Hv") ) {
            gCPUVendor = CPU_Microsoft;
            gVirtual = true;
        } else if( !vendor.compare(" lrpepyh vr") ) {
            gCPUVendor = CPU_Parallels;
            gVirtual = true;
        } else if( !vendor.compare("VMwareVMware") ) {
            gCPUVendor = CPU_VMWare;
            gVirtual = true;
        } else if( !vendor.compare("XenVMMXenVMM")
                || !vendor.compare("ACRNACRNACRN")
                || !vendor.compare("bhyve bhyve ") ) {
            gCPUVendor = CPU_Unknown;
            gVirtual = true;
        }

        return vendor;
    }

    uint32_t LargestParam() {
        Call(0);
        return EAX();
    }

    uint32_t Info() {
        Call(1);
        return EAX();
    }

    uint32_t AdditionalInfo() {
        Call(1);
        return EBX();
    }
};

