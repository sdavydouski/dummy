#pragma once

struct audio_play_options
{
    f32 Volume;
    b32 IsLooping;
};

enum audio_command_type
{
    AudioCommand_Play_2D,
    AudioCommand_Play_3D,
    AudioCommand_SetListener,
    AudioCommand_SetEmitter,
    AudioCommand_Pause,
    AudioCommand_Resume,
    AudioCommand_Stop,
};

struct audio_command_header
{
    audio_command_type Type;
    u32 Size;
};

struct audio_command_play_2d
{
    audio_command_header Header;

    u32 Id;
    audio_clip *AudioClip;
    audio_play_options Options;
};

struct audio_command_play_3d
{
    audio_command_header Header;

    u32 Id;
    vec3 EmitterPosition;
    audio_clip *AudioClip;
    audio_play_options Options;
};

struct audio_command_set_listener
{
    audio_command_header Header;
    vec3 ListenerPosition;
    vec3 ListenerDirection;
};

struct audio_command_set_emitter
{
    audio_command_header Header;
    u32 Id;
    vec3 EmitterPosition;
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
