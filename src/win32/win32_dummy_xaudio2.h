#pragma once

#define SPEAKER_MONO             (SPEAKER_FRONT_CENTER)
#define SPEAKER_STEREO           (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)
#define SPEAKER_2POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY)
#define SPEAKER_SURROUND         (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER)
#define SPEAKER_QUAD             (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
#define SPEAKER_4POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
#define SPEAKER_5POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
#define SPEAKER_7POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER)
#define SPEAKER_5POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
#define SPEAKER_7POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT  | SPEAKER_SIDE_RIGHT)

class XAudio2EngineCallback : public IXAudio2EngineCallback
{
public:
    HANDLE CriticalError;

    XAudio2EngineCallback() : CriticalError(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
    ~XAudio2EngineCallback()
    {
        CloseHandle(CriticalError);
    }

    void OnProcessingPassEnd() noexcept {}
    void OnProcessingPassStart() noexcept {}
    void OnCriticalError(HRESULT Error) noexcept
    {
        SetEvent(CriticalError);
    }
};

class XAudio2VoiceCallback : public IXAudio2VoiceCallback
{
public:
    HANDLE BufferEnd;

    XAudio2VoiceCallback() : BufferEnd(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
    ~XAudio2VoiceCallback()
    {
        CloseHandle(BufferEnd);
    }

    void OnBufferEnd(void *pBufferContext) noexcept
    {
        SetEvent(BufferEnd);
    }

    void OnStreamEnd() noexcept { }
    void OnVoiceProcessingPassEnd() noexcept {}
    void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept {}
    void OnBufferStart(void *pBufferContext) noexcept {}
    void OnLoopEnd(void *pBufferContext) noexcept {}
    void OnVoiceError(void *pBufferContext, HRESULT Error) noexcept {}
};

struct xaudio2_source_voice
{
    IXAudio2SourceVoice *SourceVoice;
    XAUDIO2_VOICE_DETAILS VoiceDetails;

    u32 Key;
    xaudio2_source_voice *Prev;
    xaudio2_source_voice *Next;
};

struct xaudio2_state
{
    IXAudio2 *XAudio2;
    IXAudio2MasteringVoice *MasterVoice;
    XAUDIO2_VOICE_DETAILS MasterVoiceDetails;
    DWORD MasterVoiceChannelMask;

    XAudio2EngineCallback EngineCallback;
    XAudio2VoiceCallback VoiceCallback;

    hash_table<xaudio2_source_voice> Voices;
    xaudio2_source_voice VoiceSentinel;

    vec3 ListenerPosition;
    vec3 ListenerDirection;

    platform_profiler *Profiler;
    memory_arena Arena;
    stream Stream;
};
