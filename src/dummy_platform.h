#pragma once

enum mouse_mode
{
    MouseMode_Navigation,
    MouseMode_Cursor
};

struct read_file_result
{
    u32 Size;
    void *Contents;
};

struct read_file_options
{
    bool32 ReadAsText;
};

inline read_file_options
ReadBinary()
{
    read_file_options Result = {};
    return Result;
}

inline read_file_options
ReadText()
{
    read_file_options Result = {};

    Result.ReadAsText = true;

    return Result;
}

struct platform_file
{
    wchar FileName[256];
    u64 FileSize;
    u64 FileDate;
};

struct get_files_result
{
    u32 FileCount;
    platform_file *Files;
};

#define PLATFORM_READ_FILE(name) read_file_result name(char *FileName, memory_arena *Arena, read_file_options Options)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_WRITE_FILE(name) bool32 name(char *FileName, void *Buffer, u32 BufferSize)
typedef PLATFORM_WRITE_FILE(platform_write_file);

#define PLATFORM_GET_FILES(name) get_files_result name(wchar *Directory, memory_arena *Arena)
typedef PLATFORM_GET_FILES(platform_get_files);

#define PLATFORM_SET_MOUSE_MODE(name) void name(void *PlatformHandle, mouse_mode MouseMode)
typedef PLATFORM_SET_MOUSE_MODE(platform_set_mouse_mode);

#define PLATFORM_LOAD_FUNCTION(name) void * name(void *PlatformHandle, char *FunctionName)
typedef PLATFORM_LOAD_FUNCTION(platform_load_function);

#define PLATFORM_OPEN_FILE_DIALOG(name) void name(wchar *FilePath, u32 FilePathLength)
typedef PLATFORM_OPEN_FILE_DIALOG(platform_open_file_dialog);

#define PLATFORM_SAVE_FILE_DIALOG(name) void name(wchar *FilePath, u32 FilePathLength)
typedef PLATFORM_SAVE_FILE_DIALOG(platform_save_file_dialog);

#define PLATFORM_KICK_JOB(name) void name(job_queue *JobQueue, job Job)
typedef PLATFORM_KICK_JOB(platform_kick_job);

#define PLATFORM_KICK_JOBS(name) void name(job_queue *JobQueue, u32 JobCount, job *Jobs)
typedef PLATFORM_KICK_JOBS(platform_kick_jobs);

#define PLATFORM_KICK_JOB_AND_WAIT(name) void name(job_queue *JobQueue, job Job)
typedef PLATFORM_KICK_JOB_AND_WAIT(platform_kick_job_and_wait);

#define PLATFORM_KICK_JOBS_AND_WAIT(name) void name(job_queue *JobQueue, u32 JobCount, job *Jobs)
typedef PLATFORM_KICK_JOBS_AND_WAIT(platform_kick_jobs_and_wait);

#define PLATFORM_ENTER_CRITICAL_SECTION(name) void name(void *PlatformHandle)
typedef PLATFORM_ENTER_CRITICAL_SECTION(platform_enter_critical_section);

#define PLATFORM_LEAVE_CRITICAL_SECTION(name) void name(void *PlatformHandle)
typedef PLATFORM_LEAVE_CRITICAL_SECTION(platform_leave_critical_section);

struct platform_api
{
    void *PlatformHandle;

    platform_read_file *ReadFile;
    platform_write_file *WriteFile;
    platform_get_files *GetFiles;

    platform_set_mouse_mode *SetMouseMode;
    platform_load_function *LoadFunction;
    platform_open_file_dialog *OpenFileDialog;
    platform_save_file_dialog *SaveFileDialog;

    platform_kick_job *KickJob;
    platform_kick_jobs *KickJobs;
    platform_kick_job_and_wait *KickJobAndWait;
    platform_kick_jobs_and_wait *KickJobsAndWait;

    platform_enter_critical_section *EnterCriticalSection;
    platform_leave_critical_section *LeaveCriticalSection;
};

struct game_memory
{
    umm PermanentStorageSize;
    void *PermanentStorage;

    umm TransientStorageSize;
    void *TransientStorage;

    umm RenderCommandsStorageSize;
    void *RenderCommandsStorage;

    umm AudioCommandsStorageSize;
    void *AudioCommandsStorage;

    platform_api *Platform;
    platform_profiler *Profiler;
    job_queue *JobQueue;
};

inline game_state *
GetGameState(game_memory *Memory)
{
    game_state *GameState =(game_state *) Memory->PermanentStorage;
    return GameState;
}

inline render_commands *
GetRenderCommands(game_memory *Memory)
{
    render_commands *RenderCommands = (render_commands *) Memory->RenderCommandsStorage;
    return RenderCommands;
}

inline void
ClearRenderCommands(game_memory *Memory)
{
    render_commands *RenderCommands = (render_commands *) Memory->RenderCommandsStorage;
    RenderCommands->MaxRenderCommandsBufferSize = (u32) (Memory->RenderCommandsStorageSize - sizeof(render_commands));
    RenderCommands->RenderCommandsBufferSize = 0;
    RenderCommands->RenderCommandsBuffer = (u8 *) Memory->RenderCommandsStorage + sizeof(render_commands);
}

inline audio_commands *
GetAudioCommands(game_memory *Memory)
{
    audio_commands *AudioCommands = (audio_commands *) Memory->AudioCommandsStorage;
    return AudioCommands;
}

inline void
ClearAudioCommands(game_memory *Memory)
{
    audio_commands *AudioCommands = (audio_commands *) Memory->AudioCommandsStorage;
    AudioCommands->MaxAudioCommandsBufferSize = (u32) (Memory->AudioCommandsStorageSize - sizeof(audio_commands));
    AudioCommands->AudioCommandsBufferSize = 0;
    AudioCommands->AudioCommandsBuffer = (u8 *) Memory->AudioCommandsStorage + sizeof(audio_commands);
}

//
template <typename T>
struct value_state
{
    T Value;
    T PrevValue;
};

template <typename T>
inline void
SavePrevValueState(value_state<T> *State)
{
    State->PrevValue = State->Value;
}

template <typename T>
inline bool32
Changed(value_state<T> ValueState)
{
    bool32 Result = ValueState.Value != ValueState.PrevValue;
    return Result;
}
//

struct game_parameters
{
    u32 WindowWidth;
    u32 WindowHeight;
    u32 Samples;

    f32 Time;
    f32 Delta;
    f32 UnscaledTime;
    f32 UnscaledDelta;
    f32 PrevTimeScale;
    f32 TimeScale;
    f32 UpdateRate;
    f32 UpdateLag;
};

#define GAME_INIT(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_INIT(game_init);

#define GAME_RELOAD(name) void name(game_memory *Memory)
typedef GAME_RELOAD(game_reload);

#define GAME_PROCESS_INPUT(name) void name(game_memory *Memory, game_parameters *Parameters, game_input *Input)
typedef GAME_PROCESS_INPUT(game_process_input);

#define GAME_UPDATE(name) void name(game_memory *Memory, game_parameters *Parameters, game_input *Input)
typedef GAME_UPDATE(game_update);

#define GAME_RENDER(name) void name(game_memory *Memory, game_parameters *Parameters, game_input *Input)
typedef GAME_RENDER(game_render);
