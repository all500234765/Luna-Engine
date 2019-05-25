#include "AudioDevice.h"

#include <shlwapi.h>
#include <iostream>
#include <string>

AudioDeviceCreation AudioDevice::Create(const AudioDeviceConfig* config, XAUDIO2_PROCESSOR processor) {
    cfg = *config; // Store config for further use

    // Create device
    XAudio2Create(&gAudio, cfg.Flags, processor);
    if( !gAudio ) {
        std::cout << "XA2: Failed to inialize audio device." << std::endl;
        return DeviceCreationFailed;
    }

    // Create mastering voice
    gAudio->CreateMasteringVoice(&gMasteringVoice);
    if( !gMasteringVoice ) {
        std::cout << "XA2: Failed to create mastering voice." << std::endl;
        return MasteringVoiceFailed;
    }

    // Done
    bCreated = true;
    return Success;
}

void AudioDevice::Release() {
    // Release engine
    gMasteringVoice->DestroyVoice();
    gAudio->Release();
}

void AudioDevice::SetMasterVolume(float value) {
    gMasteringVoice->SetVolume(value);
}

float AudioDevice::GetMasterVolume() {
    float f = 0.f; gMasteringVoice->GetVolume(&f);
    return f;
}
