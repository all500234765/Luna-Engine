#pragma once

#include <xaudio2.h>
#include <vector>

#define STB_VORBIS_HEADER_ONLY
#include "Vendor/STB/stb_vorbis.h"

#pragma comment(lib, "xaudio2.lib")

#ifdef _XBOX
// Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT  'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#else
// Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT  ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

// Audio device config structure
struct AudioDeviceConfig {
    UINT32 Flags;


};

// 
typedef enum {
    Success,

    // Errors goes here
    DeviceCreationFailed, 
    MasteringVoiceFailed, 



} AudioDeviceCreation;

// Audio device class
class AudioDevice {
private:
    bool bCreated = false;
    AudioDeviceConfig cfg;
public:
    IXAudio2 *gAudio = 0;
    IXAudio2MasteringVoice *gMasteringVoice = 0;

    AudioDeviceCreation Create(const AudioDeviceConfig& config, XAUDIO2_PROCESSOR processor=XAUDIO2_DEFAULT_PROCESSOR);
    void Release();
    
    void SetMasterVolume(float value);
    float GetMasterVolume();
};
