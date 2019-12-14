#pragma once
// 
// Based on: https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiAudio.h
// 

#include "Audio.h"

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

enum AudioDeviceCreation {
    Success,

    // Errors goes here
    NoMultithreading, 
    DeviceCreationFailed,
    MasteringVoiceFailed,
    SubmixFailed, 
    Audio3DFailed, 


};

class AudioDevice: Audio {
private:


public:
    AudioDeviceCreation Create();
    void Release();

    void SetMasterVolume(float value);
    float GetMasterVolume();

    HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
    HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
    HRESULT OpenFile(const TCHAR* fname);

};
