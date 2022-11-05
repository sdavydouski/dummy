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
Play2D(audio_commands *Commands, audio_clip *AudioClip, b32 IsLooping = false, u32 Id = 0)
{
    audio_command_play_2d *Command = PushAudioCommand(Commands, audio_command_play_2d, AudioCommand_Play_2D);
    Command->AudioClip = AudioClip;
    Command->IsLooping = IsLooping;
    Command->Id = Id;
}

inline void
Play3D(audio_commands *Commands, audio_clip *AudioClip, vec3 EmitterPosition, u32 Id = 0)
{
    audio_command_play_3d *Command = PushAudioCommand(Commands, audio_command_play_3d, AudioCommand_Play_3D);
    Command->AudioClip = AudioClip;
    Command->EmitterPosition = EmitterPosition;
    Command->Id = Id;
}

inline void
SetListener(audio_commands *Commands, vec3 ListenerPosition, vec3 ListenerDirection)
{
    audio_command_set_listener *Command = PushAudioCommand(Commands, audio_command_set_listener, AudioCommand_SetListener);
    Command->ListenerPosition = ListenerPosition;
    Command->ListenerDirection = ListenerDirection;
}

inline void
SetEmitter(audio_commands *Commands, u32 Id, vec3 EmitterPosition)
{
    audio_command_set_emitter *Command = PushAudioCommand(Commands, audio_command_set_emitter, AudioCommand_SetEmitter);
    Command->Id = Id;
    Command->EmitterPosition = EmitterPosition;
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
