#pragma once

#include "pc.h"
#include "AudioDeviceChild.h"

class Sound: public AudioDeviceChild {
protected:
    WAVEFORMATEX wfx;
    XAUDIO2_BUFFER pBuff;
    IXAudio2SourceVoice *pVoice;
    
    HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
    HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset);
    HRESULT OpenFile(const TCHAR* fname);
public:
    Sound();
    void Release();

    bool LoadOGG(const char* fname);

    HRESULT Create();

    void SetVolume(float val);
    float GetVolume();

    HRESULT Play();
    HRESULT Play(DWORD delay);
    HRESULT PlayNQ();
    HRESULT PlayNQ(DWORD delay);
    void Stop();

    bool isPlaying();
};
