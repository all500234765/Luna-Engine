#pragma once

#include "pc.h"

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
