inline audio_command_header *
PushAudioCommand_(audio_commands *Commands, u32 Size, audio_command_type Type)
{
    Assert(Commands->AudioCommandsBufferSize + Size < Commands->MaxAudioCommandsBufferSize);

    audio_command_header *Result = (audio_command_header *) ((u8 *) Commands->AudioCommandsBuffer + Commands->AudioCommandsBufferSize);
    Result->Type = Type;
    Result->Size = Size;

    Commands->AudioCommandsBufferSize += Size;

    return Result;
}

#define PushAudioCommand(Buffer, Struct, Type) (Struct *) PushAudioCommand_(Buffer, sizeof(Struct), Type)

inline void
Play(audio_commands *Commands, u32 Id, audio_clip *AudioClip, b32 IsLooping = false)
{
    audio_command_play *Command = PushAudioCommand(Commands, audio_command_play, AudioCommand_Play);
    Command->Id = Id;
    Command->AudioClip = AudioClip;
    Command->IsLooping = IsLooping;
}

inline void
Pause(audio_commands *Commands, u32 Id)
{
    audio_command_pause *Command = PushAudioCommand(Commands, audio_command_pause, AudioCommand_Pause);
    Command->Id = Id;
}

inline void
Resume(audio_commands *Commands, u32 Id)
{
    audio_command_resume *Command = PushAudioCommand(Commands, audio_command_resume, AudioCommand_Resume);
    Command->Id = Id;
}

inline void
Stop(audio_commands *Commands, u32 Id)
{
    audio_command_stop *Command = PushAudioCommand(Commands, audio_command_stop, AudioCommand_Stop);
    Command->Id = Id;
}
