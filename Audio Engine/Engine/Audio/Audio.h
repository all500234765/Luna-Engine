#pragma once
// 
// Based on: https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiAudio.h
// 

#include "Engine Includes/Types.h"

#include <vector>

// Include XAudio2
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>

// Link library
#pragma comment(lib, "xaudio2.lib")

#define SUBMIX_COUNT 128

enum SUBMIX_TYPE {
    SUBMIX_TYPE_SOUNDEFFECT,
    SUBMIX_TYPE_MUSIC,
    SUBMIX_TYPE_USER0,
    SUBMIX_TYPE_USER1,
    SUBMIX_TYPE_COUNT,

    ENUM_FORCE_UINT32 = 0xFFFFFFFF, // submix type can be serialized
};

void AudioRelease(Sound*);
void AudioRelease(SoundInstance*);

struct SoundInstance {
    SUBMIX_TYPE mType = SUBMIX_TYPE_SOUNDEFFECT;
    float mLoopBegin  = 0.f;                     // Loop region begin in seconds (0 = from beginning)
    float mLoopLength = 0.f;                     // Loop region legth in seconds (0 = until the end)
    uint64_t mHandle  = 0;

    void operator=(SoundInstance&& other) {
        mType       = other.mType;
        mLoopBegin  = other.mLoopBegin;
        mLoopLength = other.mLoopLength;
        mHandle     = other.mHandle;

        other.mHandle = 0;
    }

    SoundInstance() {}
    SoundInstance(SoundInstance&& other) {
        mType       = other.mType;
        mLoopBegin  = other.mLoopBegin;
        mLoopLength = other.mLoopLength;
        mHandle     = other.mHandle;

        other.mHandle = 0;
    }
    ~SoundInstance() { AudioRelease(this); }
};

struct Sound {
    uint64_t mHandle = 0;

    void operator=(Sound&& other) {
        mHandle = other.mHandle;
        other.mHandle = 0;
    }

    Sound() {}
    Sound(Sound&& other) {
        mHandle = other.mHandle;

        other.mHandle = 0;
    }
    ~Sound() { AudioRelease(this); }
};

struct SoundInternal {
    WAVEFORMATEX mWFX{};
    std::vector<uint8_t> mAudioData;
};

struct SoundInstanceInternal {
    IXAudio2SourceVoice* mSourceVoice{};
    XAUDIO2_VOICE_DETAILS mVoiceDetails{};
    std::vector<float> mOutputMatrix;
    std::vector<float> mChannelAzimuths;
    XAUDIO2_BUFFER mBuffer{};
    const SoundInternal* mSoundinternal{};
};

void AudioRelease(Sound* snd) {
    if( snd != nullptr && snd->mHandle != 0 ) {
        SoundInternal* soundinternal = (SoundInternal*)snd->mHandle;
        delete soundinternal;
        snd->mHandle = 0;
    }
}

void AudioRelease(SoundInstance* inst) {
    if( inst != nullptr && inst->mHandle != 0 ) {
        SoundInstanceInternal* instanceinternal = (SoundInstanceInternal*)inst->mHandle;
        instanceinternal->mSourceVoice->Stop();
        instanceinternal->mSourceVoice->DestroyVoice();
        delete instanceinternal;
        inst->mHandle = 0;
    }
}

struct Sound3D {
    float3 listenerPos      = float3(0.f, 0.f, 0.f);
    float3 listenerUp       = float3(0.f, 1.f, 0.f);
    float3 listenerFront    = float3(0.f, 0.f, 1.f);
    float3 listenerVelocity = float3(0.f, 0.f, 0.f);
    float3 emitterPos       = float3(0.f, 0.f, 0.f);
    float3 emitterUp        = float3(0.f, 1.f, 0.f);
    float3 emitterFront     = float3(0.f, 0.f, 1.f);
    float3 emitterVelocity  = float3(0.f, 0.f, 0.f);
    float emitterRadius     = 0.f;
};

static const XAUDIO2FX_REVERB_I3DL2_PARAMETERS gReverbPresets[] = {
    XAUDIO2FX_I3DL2_PRESET_DEFAULT,
    XAUDIO2FX_I3DL2_PRESET_GENERIC,
    XAUDIO2FX_I3DL2_PRESET_FOREST,
    XAUDIO2FX_I3DL2_PRESET_PADDEDCELL,
    XAUDIO2FX_I3DL2_PRESET_ROOM,
    XAUDIO2FX_I3DL2_PRESET_BATHROOM,
    XAUDIO2FX_I3DL2_PRESET_LIVINGROOM,
    XAUDIO2FX_I3DL2_PRESET_STONEROOM,
    XAUDIO2FX_I3DL2_PRESET_AUDITORIUM,
    XAUDIO2FX_I3DL2_PRESET_CONCERTHALL,
    XAUDIO2FX_I3DL2_PRESET_CAVE,
    XAUDIO2FX_I3DL2_PRESET_ARENA,
    XAUDIO2FX_I3DL2_PRESET_HANGAR,
    XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY,
    XAUDIO2FX_I3DL2_PRESET_HALLWAY,
    XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR,
    XAUDIO2FX_I3DL2_PRESET_ALLEY,
    XAUDIO2FX_I3DL2_PRESET_CITY,
    XAUDIO2FX_I3DL2_PRESET_MOUNTAINS,
    XAUDIO2FX_I3DL2_PRESET_QUARRY,
    XAUDIO2FX_I3DL2_PRESET_PLAIN,
    XAUDIO2FX_I3DL2_PRESET_PARKINGLOT,
    XAUDIO2FX_I3DL2_PRESET_SEWERPIPE,
    XAUDIO2FX_I3DL2_PRESET_UNDERWATER,
    XAUDIO2FX_I3DL2_PRESET_SMALLROOM,
    XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM,
    XAUDIO2FX_I3DL2_PRESET_LARGEROOM,
    XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL,
    XAUDIO2FX_I3DL2_PRESET_LARGEHALL,
    XAUDIO2FX_I3DL2_PRESET_PLATE,
};

enum REVERB_PRESET {
    REVERB_PRESET_DEFAULT,
    REVERB_PRESET_GENERIC,
    REVERB_PRESET_FOREST,
    REVERB_PRESET_PADDEDCELL,
    REVERB_PRESET_ROOM,
    REVERB_PRESET_BATHROOM,
    REVERB_PRESET_LIVINGROOM,
    REVERB_PRESET_STONEROOM,
    REVERB_PRESET_AUDITORIUM,
    REVERB_PRESET_CONCERTHALL,
    REVERB_PRESET_CAVE,
    REVERB_PRESET_ARENA,
    REVERB_PRESET_HANGAR,
    REVERB_PRESET_CARPETEDHALLWAY,
    REVERB_PRESET_HALLWAY,
    REVERB_PRESET_STONECORRIDOR,
    REVERB_PRESET_ALLEY,
    REVERB_PRESET_CITY,
    REVERB_PRESET_MOUNTAINS,
    REVERB_PRESET_QUARRY,
    REVERB_PRESET_PLAIN,
    REVERB_PRESET_PARKINGLOT,
    REVERB_PRESET_SEWERPIPE,
    REVERB_PRESET_UNDERWATER,
    REVERB_PRESET_SMALLROOM,
    REVERB_PRESET_MEDIUMROOM,
    REVERB_PRESET_LARGEROOM,
    REVERB_PRESET_MEDIUMHALL,
    REVERB_PRESET_LARGEHALL,
    REVERB_PRESET_PLATE,
};

class Audio {
protected:
    IXAudio2 *gAudio{};
    IXAudio2MasteringVoice *gMasteringVoice{};
    XAUDIO2_VOICE_DETAILS gMasteringVoiceDetails{};
    IXAudio2SubmixVoice *gSubmixVoices[SUBMIX_COUNT]{}, *gReverbSubmix{};
    X3DAUDIO_HANDLE g3DAudio{};
    IUnknown *gReverbEffect{};

private:


public:
    bool SetReverb(REVERB_PRESET preset) {
        XAUDIO2FX_REVERB_PARAMETERS Native;
        ReverbConvertI3DL2ToNative(&gReverbPresets[preset], &Native);
        HRESULT hr = gReverbSubmix->SetEffectParameters(0, &Native, sizeof(Native));

        return SUCCEEDED(hr);
    }

    bool CreateSoundInstance(const Sound* snd, SoundInstance* inst) {
        HRESULT hr;
        const SoundInternal* soundinternal = (const SoundInternal*)snd->mHandle;
        SoundInstanceInternal* instanceinternal = new SoundInstanceInternal();
        instanceinternal->mSoundinternal = soundinternal;

        XAUDIO2_SEND_DESCRIPTOR SFXSend[] = {
            { XAUDIO2_SEND_USEFILTER, gSubmixVoices[inst->mType] },
            { XAUDIO2_SEND_USEFILTER, gReverbSubmix },
        };
        XAUDIO2_VOICE_SENDS SFXSendList = { ARRAYSIZE(SFXSend), SFXSend };

        hr = gAudio->CreateSourceVoice(&instanceinternal->mSourceVoice, &soundinternal->mWFX, 
                                       0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, &SFXSendList, NULL);
        if( FAILED(hr) ) return false;

        instanceinternal->mSourceVoice->GetVoiceDetails(&instanceinternal->mVoiceDetails);

        instanceinternal->mOutputMatrix.resize(size_t(instanceinternal->mVoiceDetails.InputChannels) * size_t(gMasteringVoiceDetails.InputChannels));
        instanceinternal->mChannelAzimuths.resize(instanceinternal->mVoiceDetails.InputChannels);
        for( size_t i = 0; i < instanceinternal->mChannelAzimuths.size(); ++i ) {
            instanceinternal->mChannelAzimuths[i] = X3DAUDIO_2PI * float(i) / float(instanceinternal->mChannelAzimuths.size());
        }

        instanceinternal->mBuffer.AudioBytes = (UINT32)soundinternal->mAudioData.size();
        instanceinternal->mBuffer.pAudioData = soundinternal->mAudioData.data();
        instanceinternal->mBuffer.Flags      = XAUDIO2_END_OF_STREAM;
        instanceinternal->mBuffer.LoopCount  = XAUDIO2_LOOP_INFINITE;
        instanceinternal->mBuffer.LoopBegin  = UINT32(inst->mLoopBegin * gMasteringVoiceDetails.InputSampleRate);
        instanceinternal->mBuffer.LoopLength = UINT32(inst->mLoopLength * gMasteringVoiceDetails.InputSampleRate);

        hr = instanceinternal->mSourceVoice->SubmitSourceBuffer(&instanceinternal->mBuffer);
        return SUCCEEDED(hr);
    }

    bool Play(SoundInstance* inst) {
        if( inst != nullptr && inst->mHandle != 0 ) {
            SoundInstanceInternal* instanceinternal = (SoundInstanceInternal*)inst->mHandle;
            HRESULT hr = instanceinternal->mSourceVoice->Start();
            if( FAILED(hr) ) return false;
        }
    }

    bool Pause(SoundInstance* inst) {
        if( inst != nullptr && inst->mHandle != 0 ) {
            SoundInstanceInternal* instanceinternal = (SoundInstanceInternal*)inst->mHandle;
            HRESULT hr = instanceinternal->mSourceVoice->Stop(); // Preserves cursor position
            if( FAILED(hr) ) return false;
        }
    }

    bool Stop(SoundInstance* inst) {
        if( inst != nullptr && inst->mHandle != 0 ) {
            SoundInstanceInternal* instanceinternal = (SoundInstanceInternal*)inst->mHandle;
            HRESULT hr = instanceinternal->mSourceVoice->Stop(); // Preserves cursor position
            if( FAILED(hr) ) return false;
            
            hr = instanceinternal->mSourceVoice->FlushSourceBuffers(); // Reset submitted audio buffer
            if( FAILED(hr) ) return false;
            
            hr = instanceinternal->mSourceVoice->SubmitSourceBuffer(&instanceinternal->mBuffer); // Resubmit
            return SUCCEEDED(hr);
        }
    }
    
    bool SetVolume(float volume, SoundInstance* inst) {
        if( inst == nullptr || inst->mHandle == 0 ) {
            HRESULT hr = gMasteringVoice->SetVolume(volume);
            return SUCCEEDED(hr);
        } else {
            SoundInstanceInternal* instanceinternal = (SoundInstanceInternal*)inst->mHandle;
            HRESULT hr = instanceinternal->mSourceVoice->SetVolume(volume);
            return SUCCEEDED(hr);
        }
    }

    float GetVolume(const SoundInstance* instance) {
        float volume = 0;
        if( instance == nullptr || instance->mHandle == 0 ) {
            gMasteringVoice->GetVolume(&volume);
        } else {
            const SoundInstanceInternal* instanceinternal = (const SoundInstanceInternal*)instance->mHandle;
            instanceinternal->mSourceVoice->GetVolume(&volume);
        }
        return volume;
    }
    
    bool ExitLoop(SoundInstance* instance) {
        if( instance != nullptr && instance->mHandle != 0 ) {
            SoundInstanceInternal* instanceinternal = (SoundInstanceInternal*)instance->mHandle;
            HRESULT hr = instanceinternal->mSourceVoice->ExitLoop();
            return SUCCEEDED(hr);
        }
    }

    bool SetSubmixVolume(SUBMIX_TYPE type, float volume) {
        HRESULT hr = gSubmixVoices[type]->SetVolume(volume);
        return SUCCEEDED(hr);
    }

    float GetSubmixVolume(SUBMIX_TYPE type) {
        float volume;
        gSubmixVoices[type]->GetVolume(&volume);
        return volume;
    }

    bool Update3D(SoundInstance* instance, const Sound3D& instance3D) {
        if( instance != nullptr && instance->mHandle != 0 ) {
            SoundInstanceInternal* instanceinternal = (SoundInstanceInternal*)instance->mHandle;

            X3DAUDIO_LISTENER listener = {};
            listener.Position    = instance3D.listenerPos;
            listener.OrientFront = instance3D.listenerFront;
            listener.OrientTop   = instance3D.listenerUp;
            listener.Velocity    = instance3D.listenerVelocity;

            X3DAUDIO_EMITTER emitter = {};
            emitter.Position            = instance3D.emitterPos;
            emitter.OrientFront         = instance3D.emitterFront;
            emitter.OrientTop           = instance3D.emitterUp;
            emitter.Velocity            = instance3D.emitterVelocity;
            emitter.InnerRadius         = instance3D.emitterRadius;
            emitter.InnerRadiusAngle    = X3DAUDIO_PI / 4.f;
            emitter.ChannelCount        = instanceinternal->mVoiceDetails.InputChannels;
            emitter.pChannelAzimuths    = instanceinternal->mChannelAzimuths.data();
            emitter.ChannelRadius       = .1f;
            emitter.CurveDistanceScaler = 1;
            emitter.DopplerScaler       = 1;

            UINT32 flags = 0;
            flags |= X3DAUDIO_CALCULATE_MATRIX;
            flags |= X3DAUDIO_CALCULATE_LPF_DIRECT;
            flags |= X3DAUDIO_CALCULATE_REVERB;
            flags |= X3DAUDIO_CALCULATE_LPF_REVERB;
            flags |= X3DAUDIO_CALCULATE_DOPPLER;
            //flags |= X3DAUDIO_CALCULATE_DELAY;
            //flags |= X3DAUDIO_CALCULATE_EMITTER_ANGLE;
            //flags |= X3DAUDIO_CALCULATE_ZEROCENTER;
            //flags |= X3DAUDIO_CALCULATE_REDIRECT_TO_LFE;

            X3DAUDIO_DSP_SETTINGS settings = {};
            settings.SrcChannelCount     = instanceinternal->mVoiceDetails.InputChannels;
            settings.DstChannelCount     = gMasteringVoiceDetails.InputChannels;
            settings.pMatrixCoefficients = instanceinternal->mOutputMatrix.data();

            X3DAudioCalculate(g3DAudio, &listener, &emitter, flags, &settings);

            HRESULT hr;

            hr = instanceinternal->mSourceVoice->SetFrequencyRatio(settings.DopplerFactor);
            if( FAILED(hr) ) return false;

            hr = instanceinternal->mSourceVoice->SetOutputMatrix(gSubmixVoices[instance->mType], 
                                                                 settings.SrcChannelCount,
                                                                 settings.DstChannelCount, 
                                                                 settings.pMatrixCoefficients);
            if( FAILED(hr) ) return false;

            hr = instanceinternal->mSourceVoice->SetOutputMatrix(gReverbSubmix, settings.SrcChannelCount, 1, &settings.ReverbLevel);
            if( FAILED(hr) ) return false;

            XAUDIO2_FILTER_PARAMETERS FilterParametersDirect = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * settings.LPFDirectCoefficient), 1.0f };
            hr = instanceinternal->mSourceVoice->SetOutputFilterParameters(gSubmixVoices[instance->mType], &FilterParametersDirect);
            if( FAILED(hr) ) return false;
            
            XAUDIO2_FILTER_PARAMETERS FilterParametersReverb = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * settings.LPFReverbCoefficient), 1.0f };
            hr = instanceinternal->mSourceVoice->SetOutputFilterParameters(gReverbSubmix, &FilterParametersReverb);
            return SUCCEEDED(hr);
        }

        return false;
    }

    
};
