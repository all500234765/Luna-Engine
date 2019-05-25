#include "AudioDeviceChild.h"

void AudioDeviceChild::SetAudioDevice(AudioDevice* dev) {
    gAudioDevice = dev;
}

AudioDevice* AudioDeviceChild::gAudioDevice = 0;
