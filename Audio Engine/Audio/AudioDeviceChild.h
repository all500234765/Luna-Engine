#pragma once

#include "AudioDevice.h"

// 
class AudioDeviceChild {
protected:
    static AudioDevice* gAudioDevice;

public:
    static void SetAudioDevice(AudioDevice* dev);

};
