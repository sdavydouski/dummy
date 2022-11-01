#pragma once

enum audio_command_type
{
    AudioCommand_Play,
    AudioCommand_Pause,
    AudioCommand_Resume,
    AudioCommand_Stop,

};

struct audio_command_header
{
    audio_command_type Type;
    u32 Size;
};

struct audio_command_play
{
    audio_command_header Header;

    u32 Id;
    audio_clip *AudioClip;
    b32 IsLooping;
};

struct audio_command_pause
{
    audio_command_header Header;
    u32 Id;
};

struct audio_command_resume
{
    audio_command_header Header;
    u32 Id;
};

struct audio_command_stop
{
    audio_command_header Header;
    u32 Id;
};

struct audio_commands_settings
{
    f32 Volume;
};

struct audio_commands
{
    u32 MaxAudioCommandsBufferSize;
    u32 AudioCommandsBufferSize;
    void *AudioCommandsBuffer;

    audio_commands_settings Settings;
};
