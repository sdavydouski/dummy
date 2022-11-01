#include <xaudio2.h>
#include <xapofx.h>

class XAudio2VoiceCallback : public IXAudio2VoiceCallback
{
public:
    HANDLE BufferEndEvent;

    XAudio2VoiceCallback() : BufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
    ~XAudio2VoiceCallback()
    {
        CloseHandle(BufferEndEvent);
    }

    void OnBufferEnd(void *pBufferContext)
    {
        SetEvent(BufferEndEvent);
    }

    void OnStreamEnd() { }
    void OnVoiceProcessingPassEnd() {}
    void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
    void OnBufferStart(void *pBufferContext) {}
    void OnLoopEnd(void *pBufferContext) {}
    void OnVoiceError(void *pBufferContext, HRESULT Error) {}
};

struct xaudio2_source_voice
{
    IXAudio2SourceVoice *SourceVoice;

    u32 Key;
    xaudio2_source_voice *Prev;
    xaudio2_source_voice *Next;
};

struct xaudio2_state
{
    IXAudio2 *XAudio2;
    IXAudio2MasteringVoice *MasterVoice;
    hash_table<xaudio2_source_voice> Voices;
    xaudio2_source_voice VoiceSentinel;

    XAudio2VoiceCallback VoiceCallback;

    memory_arena Arena;
};

void internal
Win32InitXAudio2(xaudio2_state *State)
{
    CoInitializeEx(0, COINIT_MULTITHREADED);

    XAudio2Create(&State->XAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

    State->XAudio2->CreateMasteringVoice(&State->MasterVoice);

    xaudio2_source_voice *VoiceSentinel = &State->VoiceSentinel;
    VoiceSentinel->SourceVoice = 0;
    VoiceSentinel->Next = VoiceSentinel->Prev = VoiceSentinel;

    State->Voices.Count = 31;
    State->Voices.Values = PushArray(&State->Arena, State->Voices.Count, xaudio2_source_voice);
}

// todo: check https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2voice-setoutputmatrix for 3d sounds
internal void
XAudio2ProcessAudioCommands(xaudio2_state *State, audio_commands *Commands)
{
    audio_commands_settings Settings = Commands->Settings;

    State->MasterVoice->SetVolume(Settings.Volume);

    for (u32 BaseAddress = 0; BaseAddress < Commands->AudioCommandsBufferSize;)
    {
        audio_command_header *Entry = (audio_command_header *) ((u8 *) Commands->AudioCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
            case AudioCommand_Play:
            {
                audio_command_play *Command = (audio_command_play *) Entry;

                xaudio2_source_voice *Voice = HashTableLookup(&State->Voices, Command->Id);

                audio_clip *AudioClip = Command->AudioClip;

                WAVEFORMATEX wfx = {};
                wfx.wFormatTag = AudioClip->Format;
                wfx.nChannels = AudioClip->Channels;
                wfx.nSamplesPerSec = AudioClip->SamplesPerSecond;
                wfx.wBitsPerSample = AudioClip->BitsPerSample;
                wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
                wfx.nAvgBytesPerSec = wfx.nChannels * wfx.nSamplesPerSec * wfx.wBitsPerSample / 8;

                IXAudio2SourceVoice *SourceVoice;
                State->XAudio2->CreateSourceVoice(&SourceVoice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &State->VoiceCallback);

                Voice->SourceVoice = SourceVoice;
                Voice->Key = Command->Id;

                AddToLinkedList(&State->VoiceSentinel, Voice);

                XAUDIO2_BUFFER AudioBuffer = {};
                AudioBuffer.AudioBytes = AudioClip->AudioBytes;
                AudioBuffer.pAudioData = AudioClip->AudioData;
                AudioBuffer.Flags = XAUDIO2_END_OF_STREAM;

                if (Command->IsLooping)
                {
                    AudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
                }

                SourceVoice->SubmitSourceBuffer(&AudioBuffer);
                SourceVoice->Start(0);

                break;
            }
            case AudioCommand_Pause:
            {
                audio_command_pause *Command = (audio_command_pause *) Entry;

                xaudio2_source_voice *Voice = HashTableLookup(&State->Voices, Command->Id);

                Assert(Voice->SourceVoice);

                Voice->SourceVoice->Stop(0);

                break;
            }
            case AudioCommand_Resume:
            {
                audio_command_resume *Command = (audio_command_resume *) Entry;

                xaudio2_source_voice *Voice = HashTableLookup(&State->Voices, Command->Id);

                Assert(Voice->SourceVoice);

                Voice->SourceVoice->Start(0);

                break;
            }
            case AudioCommand_Stop:
            {
                audio_command_stop *Command = (audio_command_stop *) Entry;

                xaudio2_source_voice *Voice = HashTableLookup(&State->Voices, Command->Id);

                Assert(Voice->SourceVoice);

                Voice->SourceVoice->Stop(0);
                Voice->SourceVoice->FlushSourceBuffers();

                break;
            }
            default:
            {
                Assert(!"Render command is not supported");
            }
        }

        BaseAddress += Entry->Size;
    }

    // todo: https://github.dev/microsoft/DirectXTK12/blob/main/Audio/AudioEngine.cpp
    HANDLE Events[] = { State->VoiceCallback.BufferEndEvent };

    switch (WaitForMultipleObjectsEx(ArrayCount(Events), Events, FALSE, 0, FALSE))
    {
        case WAIT_OBJECT_0:
        {
            xaudio2_source_voice *Voice = State->VoiceSentinel.Next;

            while (Voice->SourceVoice)
            {
                XAUDIO2_VOICE_STATE VoiceState;
                Voice->SourceVoice->GetState(&VoiceState, XAUDIO2_VOICE_NOSAMPLESPLAYED);

                if (VoiceState.BuffersQueued == 0)
                {
                    // todo: could block! - https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2voice-destroyvoice
                    Voice->SourceVoice->DestroyVoice();
                    Voice->SourceVoice = 0;

                    RemoveFromLinkedList(Voice);
                }

                Voice = Voice->Next;
            }

            break;
        }
    }
}
