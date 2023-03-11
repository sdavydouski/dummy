#include <xaudio2.h>
#include "dummy.h"
#include "win32_dummy_xaudio2.h"

f32 inline
XAudio2CalculateVoiceVolume(vec3 EmitterPosition, vec3 ListenerPosition)
{
    f32 Distance = Magnitude(EmitterPosition - ListenerPosition);

    // todo: put in settings?
    f32 MinDistance = 10.f;
    f32 MaxDistance = 100.f;

    f32 Volume = 0.f;

    if (Distance <= MinDistance)
    {
        Volume = 1.f;
    }
    else if (Distance >= MaxDistance)
    {
        Volume = 0.f;
    }
    else
    {
        Volume = 1.f - (Distance - MinDistance) / (MaxDistance - MinDistance);
    }

    Assert(Volume >= 0.f);
    Assert(Volume <= 1.f);

    return Volume;
}

inline void
XAudio2CalculateOutputMatrix(xaudio2_state *State, vec3 EmitterPosition, vec3 ListenerPosition, vec3 ListenerDirection, f32 *OutputMatrix)
{
    vec2 n = Normalize(PerpendicularCW(vec2(ListenerDirection.x, ListenerDirection.z)));
    vec2 d = Normalize(vec2(EmitterPosition.x, EmitterPosition.z) - vec2(ListenerPosition.x, ListenerPosition.z));

    // Pan of - 1.0 indicates all left speaker,
    // 1.0 is all right speaker, 0.0 is split between left and right
    f32 Pan = Dot(n, d);

    f32 Left = 0.5f - Pan / 2;
    f32 Right = 0.5f + Pan / 2;

    switch (State->MasterVoiceChannelMask)
    {
        case SPEAKER_MONO:
        {
            OutputMatrix[0] = 1.f;
            break;
        }
        case SPEAKER_STEREO:
        case SPEAKER_2POINT1:
        case SPEAKER_SURROUND:
        {
            OutputMatrix[0] = Left;
            OutputMatrix[1] = Right;
            break;
        }
        case SPEAKER_QUAD:
        {
            OutputMatrix[0] = OutputMatrix[2] = Left;
            OutputMatrix[1] = OutputMatrix[3] = Right;
            break;
        }
        case SPEAKER_4POINT1:
        {
            OutputMatrix[0] = OutputMatrix[3] = Left;
            OutputMatrix[1] = OutputMatrix[4] = Right;
            break;
        }
        case SPEAKER_5POINT1:
        case SPEAKER_7POINT1:
        case SPEAKER_5POINT1_SURROUND:
        {
            OutputMatrix[0] = OutputMatrix[4] = Left;
            OutputMatrix[1] = OutputMatrix[5] = Right;
            break;
        }
        case SPEAKER_7POINT1_SURROUND:
        {
            OutputMatrix[0] = OutputMatrix[4] = OutputMatrix[6] = Left;
            OutputMatrix[1] = OutputMatrix[5] = OutputMatrix[7] = Right;
            break;
        }
    }
}

dummy_internal void
Win32InitXAudio2(xaudio2_state *State)
{
    XAudio2Create(&State->XAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

    State->Stream = CreateStream(SubMemoryArena(&State->Arena, Megabytes(1)));

    State->XAudio2->RegisterForCallbacks(&State->EngineCallback);
    State->XAudio2->CreateMasteringVoice(&State->MasterVoice);

    XAUDIO2_VOICE_DETAILS MasterVoiceDetails;
    State->MasterVoice->GetVoiceDetails(&MasterVoiceDetails);

    DWORD MasterVoiceChannelMask;
    State->MasterVoice->GetChannelMask(&MasterVoiceChannelMask);

    State->MasterVoiceDetails = MasterVoiceDetails;
    State->MasterVoiceChannelMask = MasterVoiceChannelMask;

    xaudio2_source_voice *VoiceSentinel = &State->VoiceSentinel;
    VoiceSentinel->Key = 0;
    VoiceSentinel->SourceVoice = 0;
    VoiceSentinel->Prev = VoiceSentinel;
    VoiceSentinel->Next = VoiceSentinel;

    State->Voices.Count = 31;
    State->Voices.Values = PushArray(&State->Arena, State->Voices.Count, xaudio2_source_voice);
}

dummy_internal void
XAudio2DestroySourceVoice(xaudio2_source_voice *Voice)
{
    // todo: could block! - https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2voice-destroyvoice
    Voice->SourceVoice->DestroyVoice();
    Voice->SourceVoice = 0;
}

dummy_internal void
XAudio2ProcessAudioCommands(xaudio2_state *State, audio_commands *Commands)
{
    PROFILE(State->Profiler, "XAudio2ProcessAudioCommands");

    audio_commands_settings Settings = Commands->Settings;

    State->MasterVoice->SetVolume(Settings.Volume);

    for (u32 BaseAddress = 0; BaseAddress < Commands->AudioCommandsBufferSize;)
    {
        audio_command_header *Entry = (audio_command_header *) ((u8 *) Commands->AudioCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
            case AudioCommand_Play_2D:
            {
                audio_command_play_2d *Command = (audio_command_play_2d *) Entry;

                audio_clip *AudioClip = Command->AudioClip;
                audio_play_options Params = Command->Options;

                WAVEFORMATEX wfx = {};
                wfx.wFormatTag = AudioClip->Format;
                wfx.nChannels = AudioClip->Channels;
                wfx.nSamplesPerSec = AudioClip->SamplesPerSecond;
                wfx.wBitsPerSample = AudioClip->BitsPerSample;
                wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
                wfx.nAvgBytesPerSec = wfx.nChannels * wfx.nSamplesPerSec * wfx.wBitsPerSample / 8;

                IXAudio2SourceVoice *SourceVoice;
                State->XAudio2->CreateSourceVoice(&SourceVoice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &State->VoiceCallback);

                XAUDIO2_VOICE_DETAILS VoiceDetails;
                SourceVoice->GetVoiceDetails(&VoiceDetails);

                XAUDIO2_BUFFER AudioBuffer = {};
                AudioBuffer.AudioBytes = AudioClip->AudioBytes;
                AudioBuffer.pAudioData = AudioClip->AudioData;
                AudioBuffer.Flags = XAUDIO2_END_OF_STREAM;

                if (Params.IsLooping)
                {
                    AudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
                }

                SourceVoice->SubmitSourceBuffer(&AudioBuffer);
                SourceVoice->SetVolume(Params.Volume);
                SourceVoice->Start(0);

                if (Command->Id != 0)
                {
                    xaudio2_source_voice *Voice = HashTableLookup(&State->Voices, Command->Id);

                    Voice->SourceVoice = SourceVoice;
                    Voice->VoiceDetails = VoiceDetails;
                    Voice->Key = Command->Id;

                    AddToLinkedList(&State->VoiceSentinel, Voice);
                }

                break;
            }
            case AudioCommand_Play_3D:
            {
                audio_command_play_3d *Command = (audio_command_play_3d *) Entry;

                vec3 EmitterPosition = Command->EmitterPosition;

                vec3 ListenerPosition = State->ListenerPosition;
                vec3 ListenerDirection = State->ListenerDirection;

                //
                audio_clip *AudioClip = Command->AudioClip;
                audio_play_options Params = Command->Options;

                WAVEFORMATEX wfx = {};
                wfx.wFormatTag = AudioClip->Format;
                wfx.nChannels = AudioClip->Channels;
                wfx.nSamplesPerSec = AudioClip->SamplesPerSecond;
                wfx.wBitsPerSample = AudioClip->BitsPerSample;
                wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
                wfx.nAvgBytesPerSec = wfx.nChannels * wfx.nSamplesPerSec * wfx.wBitsPerSample / 8;

                IXAudio2SourceVoice *SourceVoice;
                State->XAudio2->CreateSourceVoice(&SourceVoice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &State->VoiceCallback);

                XAUDIO2_VOICE_DETAILS VoiceDetails;
                SourceVoice->GetVoiceDetails(&VoiceDetails);

                XAUDIO2_BUFFER AudioBuffer = {};
                AudioBuffer.AudioBytes = AudioClip->AudioBytes;
                AudioBuffer.pAudioData = AudioClip->AudioData;
                AudioBuffer.Flags = XAUDIO2_END_OF_STREAM;

                if (Params.IsLooping)
                {
                    AudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
                }

                SourceVoice->SubmitSourceBuffer(&AudioBuffer);
                //

                // todo: https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--integrate-x3daudio-with-xaudio2 ?
                f32 Volume = XAudio2CalculateVoiceVolume(EmitterPosition, ListenerPosition);

                f32 OutputMatrix[8] = {};
                XAudio2CalculateOutputMatrix(State, EmitterPosition, ListenerPosition, ListenerDirection, OutputMatrix);

                SourceVoice->SetVolume(Volume * Params.Volume);
                SourceVoice->SetOutputMatrix(0, VoiceDetails.InputChannels, State->MasterVoiceDetails.InputChannels, OutputMatrix);
                SourceVoice->Start(0);

                if (Command->Id != 0)
                {
                    xaudio2_source_voice *Voice = HashTableLookup(&State->Voices, Command->Id);

                    Voice->SourceVoice = SourceVoice;
                    Voice->VoiceDetails = VoiceDetails;
                    Voice->Key = Command->Id;

                    AddToLinkedList(&State->VoiceSentinel, Voice);
                }

                break;
            }
            case AudioCommand_SetListener:
            {
                audio_command_set_listener *Command = (audio_command_set_listener *) Entry;

                State->ListenerPosition = Command->ListenerPosition;
                State->ListenerDirection = Command->ListenerDirection;

                break;
            }
            case AudioCommand_SetEmitter:
            {
                audio_command_set_emitter *Command = (audio_command_set_emitter *) Entry;

                vec3 ListenerPosition = State->ListenerPosition;
                vec3 ListenerDirection = State->ListenerDirection;

                Assert(Command->Id != 0);

                xaudio2_source_voice *Voice = HashTableLookup(&State->Voices, Command->Id);

                Assert(Voice->SourceVoice);

                f32 Volume = XAudio2CalculateVoiceVolume(Command->EmitterPosition, ListenerPosition);

                f32 OutputMatrix[8] = {};
                XAudio2CalculateOutputMatrix(State, Command->EmitterPosition, ListenerPosition, ListenerDirection, OutputMatrix);

                Voice->SourceVoice->SetVolume(Volume);
                Voice->SourceVoice->SetOutputMatrix(0, Voice->VoiceDetails.InputChannels, State->MasterVoiceDetails.InputChannels, OutputMatrix);

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
                Assert(!"Audio command is not supported");
            }
        }

        BaseAddress += Entry->Size;
    }

    HANDLE Events[] = { State->EngineCallback.CriticalError, State->VoiceCallback.BufferEnd };

    switch (WaitForMultipleObjectsEx(ArrayCount(Events), Events, FALSE, 0, FALSE))
    {
        // CriticalError
        case WAIT_OBJECT_0:
        {
            Assert(!"Critical Error");

            break;
        }
        // BufferEnd
        case WAIT_OBJECT_0 + 1:
        {
            xaudio2_source_voice *Voice = State->VoiceSentinel.Next;

            while (Voice->SourceVoice)
            {
                XAUDIO2_VOICE_STATE VoiceState;
                Voice->SourceVoice->GetState(&VoiceState, XAUDIO2_VOICE_NOSAMPLESPLAYED);

                if (VoiceState.BuffersQueued == 0)
                {
                    XAudio2DestroySourceVoice(Voice);
                    RemoveFromHashTable(&Voice->Key);
                    RemoveFromLinkedList(Voice);
                }

                Voice = Voice->Next;
            }

            break;
        }
    }
}

inline void
Win32XAudio2Shutdown(xaudio2_state *State)
{
    State->XAudio2->UnregisterForCallbacks(&State->EngineCallback);
    State->XAudio2->StopEngine();

    State->MasterVoice->DestroyVoice();

    xaudio2_source_voice *Voice = State->VoiceSentinel.Next;

    while (Voice->SourceVoice)
    {
        XAudio2DestroySourceVoice(Voice);
        RemoveFromLinkedList(Voice);

        Voice = Voice->Next;
    }
}
