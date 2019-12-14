// 
// Based on: https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiAudio.h
// 

#include "AudioDevice.h"

AudioDeviceCreation AudioDevice::Create() {
    HRESULT hr;
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if( FAILED(hr) ) return AudioDeviceCreation::NoMultithreading;

    hr = XAudio2Create(&gAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if( FAILED(hr) ) return AudioDeviceCreation::DeviceCreationFailed;

#ifdef _DEBUG
    XAUDIO2_DEBUG_CONFIGURATION DebugConfig = {};
    DebugConfig.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
    DebugConfig.BreakMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
    gAudio->SetDebugConfiguration(&DebugConfig);
#endif // _DEBUG

    hr = gAudio->CreateMasteringVoice(&gMasteringVoice);
    if( FAILED(hr) ) return AudioDeviceCreation::MasteringVoiceFailed;

    // 
    gMasteringVoice->GetVoiceDetails(&gMasteringVoiceDetails);

    // Create submixes
    for( int i = 0; i < SUBMIX_COUNT; ++i ) {
        hr = gAudio->CreateSubmixVoice(&gSubmixVoices[i], gMasteringVoiceDetails.InputChannels, gMasteringVoiceDetails.InputSampleRate);
        if( FAILED(hr) ) return AudioDeviceCreation::SubmixFailed;
    }

    // 
    DWORD channelMask;
    gMasteringVoice->GetChannelMask(&channelMask);
    hr = X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, g3DAudio);
    if( FAILED(hr) ) return AudioDeviceCreation::Audio3DFailed;

    // Reverb setup:
    {
        hr = XAudio2CreateReverb(&gReverbEffect);
        assert(SUCCEEDED(hr));

        XAUDIO2_EFFECT_DESCRIPTOR effects[] = { { gReverbEffect, TRUE, 1 } };
        XAUDIO2_EFFECT_CHAIN effectChain = { arraysize(effects), effects };
        hr = gAudio->CreateSubmixVoice(&gReverbSubmix, 1, // Reverb is mono
            gMasteringVoiceDetails.InputSampleRate, 0, 0, nullptr, &effectChain);
        assert(SUCCEEDED(hr));

        SetReverb(REVERB_PRESET_DEFAULT);
    }

    return AudioDeviceCreation::Success;
}

HRESULT AudioDevice::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition) {
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointerEx(hFile, LARGE_INTEGER(), NULL, FILE_BEGIN) )
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while( hr == S_OK ) {
        DWORD dwRead;
        if( 0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) )
            hr = HRESULT_FROM_WIN32(GetLastError());

        if( 0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) )
            hr = HRESULT_FROM_WIN32(GetLastError());

        switch( dwChunkType ) {
            case fourccRIFF:
                dwRIFFDataSize = dwChunkDataSize;
                dwChunkDataSize = 4;
                if( 0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) )
                    hr = HRESULT_FROM_WIN32(GetLastError());
                break;

            default:
                LARGE_INTEGER li = LARGE_INTEGER();
                li.QuadPart = dwChunkDataSize;
                if( INVALID_SET_FILE_POINTER == SetFilePointerEx(hFile, li, NULL, FILE_CURRENT) )
                    return HRESULT_FROM_WIN32(GetLastError());
        }

        dwOffset += sizeof(DWORD) * 2;
        if( dwChunkType == fourcc ) {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;
        if( bytesRead >= dwRIFFDataSize ) return S_FALSE;
    }

    return S_OK;
}

HRESULT AudioDevice::ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset) {
    HRESULT hr = S_OK;

    LARGE_INTEGER li = LARGE_INTEGER();
    li.QuadPart = bufferoffset;
    if( INVALID_SET_FILE_POINTER == SetFilePointerEx(hFile, li, NULL, FILE_BEGIN) )
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwRead;
    if( !ReadFile(hFile, buffer, buffersize, &dwRead, NULL) )
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

HRESULT AudioDevice::OpenFile(const TCHAR* fname) {
    HRESULT hr;

    wfx = WAVEFORMATEX();
    pBuff = XAUDIO2_BUFFER();

    // Open the file
    HANDLE hFile;

    hFile = CreateFile2(fname, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
    if( INVALID_HANDLE_VALUE == hFile ) return HRESULT_FROM_WIN32(GetLastError());

    if( INVALID_SET_FILE_POINTER == SetFilePointerEx(hFile, LARGE_INTEGER(), NULL, FILE_BEGIN) )
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    // Check the file type, should be fourccWAVE or 'XWMA'
    FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
    DWORD filetype;
    if( FAILED(hr = ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition)) )
        return hr;
    if( filetype != fourccWAVE )
        return S_FALSE;

    if( FAILED(hr = FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition)) )
        return hr;
    if( FAILED(hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition)) )
        return hr;
    wfx.wFormatTag = WAVE_FORMAT_PCM;


    // Fill out the audio data buffer with the contents of the fourccDATA chunk
    if( FAILED(hr = FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition)) )
        return hr;
    BYTE * pDataBuffer = new BYTE[dwChunkSize];
    if( FAILED(hr = ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition)) )
        return hr;


    pBuff.AudioBytes = dwChunkSize;  // Buffer containing audio data
    pBuff.pAudioData = pDataBuffer;  // Size of the audio buffer in bytes
    pBuff.Flags = XAUDIO2_END_OF_STREAM; // Tell the source voice not to expect any data after this buffer

    return S_OK;
}