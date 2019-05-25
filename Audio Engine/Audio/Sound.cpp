#include "Sound.h"

Sound::Sound() {
    
}

void Sound::Release() {
    pVoice->DestroyVoice();
}

HRESULT Sound::Play() {
    if( !gAudioDevice         ) return E_FAIL;
    if( !gAudioDevice->gAudio ) return E_FAIL;

    HRESULT hr;
    if( FAILED(hr = pVoice->SubmitSourceBuffer(&pBuff)) ) return hr;
    if( FAILED(hr = pVoice->Start(0)) ) return hr;

    return S_OK;
}

HRESULT Sound::Play(DWORD delay) {
    if( delay > 0 ) {
        std::thread([=] {
            Sleep(delay);
            Play();
        }).detach();

        return S_OK;
    }

    return Play();
}

HRESULT Sound::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition) {
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

HRESULT Sound::ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset) {
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

HRESULT Sound::OpenFile(const TCHAR* fname) {
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

void Sound::Stop() {
    pVoice->Stop();
}

void Sound::SetVolume(float val) {
    pVoice->SetVolume(val);
}

float Sound::GetVolume() {
    float val; pVoice->GetVolume(&val);
    return val;
}

// Load Vorbis(.ogg) audio
bool Sound::LoadOGG(const char* fname) {
    // Load Vorbis file
    int channels, rate;
    short* data;
    //int q = stb_vorbis_decode_filename(fname, &channels, &rate, &data);

    return false;
}

HRESULT Sound::Create() {
    if( !gAudioDevice ) return E_FAIL;
    if( !gAudioDevice->gAudio ) return E_FAIL;

    HRESULT hr;
    if( FAILED(hr = gAudioDevice->gAudio->CreateSourceVoice(&pVoice, &wfx)) ) return hr;

    return S_OK;
}

