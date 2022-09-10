#pragma once

struct game_state;

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

#define PLATFORM_SET_MOUSE_MODE(name) void name(void *PlatformHandle, mouse_mode MouseMode)
typedef PLATFORM_SET_MOUSE_MODE(platform_set_mouse_mode);

#define PLATFORM_READ_FILE(name) read_file_result name(char *FileName, memory_arena *Arena, b32 Text)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_DEBUG_PRINT_STRING(name) i32 name(const char *String, ...)
typedef PLATFORM_DEBUG_PRINT_STRING(platform_debug_print_string);

#define PLATFORM_LOAD_FUNCTION(name) void * name(void *PlatformHandle, char *FunctionName)
typedef PLATFORM_LOAD_FUNCTION(platform_load_function);

#define PLATFORM_KICK_JOB(name) void name(job_queue *JobQueue, job Job)
typedef PLATFORM_KICK_JOB(platform_kick_job);

#define PLATFORM_KICK_JOBS(name) void name(job_queue *JobQueue, u32 JobCount, job *Jobs)
typedef PLATFORM_KICK_JOBS(platform_kick_jobs);

#define PLATFORM_KICK_JOB_AND_WAIT(name) void name(job_queue *JobQueue, job Job)
typedef PLATFORM_KICK_JOB_AND_WAIT(platform_kick_job_and_wait);

#define PLATFORM_KICK_JOBS_AND_WAIT(name) void name(job_queue *JobQueue, u32 JobCount, job *Jobs)
typedef PLATFORM_KICK_JOBS_AND_WAIT(platform_kick_jobs_and_wait);

struct platform_api
{
    void *PlatformHandle;
    platform_set_mouse_mode *SetMouseMode;
    platform_read_file *ReadFile;
    platform_debug_print_string *DebugPrintString;
    platform_load_function *LoadFunction;

    platform_kick_job *KickJob;
    platform_kick_jobs *KickJobs;
    platform_kick_job_and_wait *KickJobAndWait;
    platform_kick_jobs_and_wait *KickJobsAndWait;
};

struct game_memory
{
    umm PermanentStorageSize;
    void *PermanentStorage;

    umm TransientStorageSize;
    void *TransientStorage;

    // todo: use TransientStorage for this?
    umm RenderCommandsStorageSize;
    void *RenderCommandsStorage;

    platform_api *Platform;
    job_queue *JobQueue;
};

inline game_state *
GetGameState(game_memory *Memory)
{
    game_state *GameState =(game_state *)Memory->PermanentStorage;
    return GameState;
}

inline render_commands *
GetRenderCommands(game_memory *Memory)
{
    render_commands *RenderCommands = (render_commands *)Memory->RenderCommandsStorage;
    return RenderCommands;
}

inline void
ClearRenderCommands(game_memory *Memory)
{
    render_commands *RenderCommands = (render_commands *)Memory->RenderCommandsStorage;
    RenderCommands->MaxRenderCommandsBufferSize = (u32)(Memory->RenderCommandsStorageSize - sizeof(render_commands));
    RenderCommands->RenderCommandsBufferSize = 0;
    RenderCommands->RenderCommandsBuffer = (u8 *)Memory->RenderCommandsStorage + sizeof(render_commands);
}

struct game_parameters
{
    u32 WindowWidth;
    u32 WindowHeight;

    f32 Time;
    f32 Delta;
    f32 UpdateRate;
    f32 UpdateLag;
};

#define GAME_INIT(name) void name(game_memory *Memory)
typedef GAME_INIT(game_init);

#define GAME_RELOAD(name) void name(game_memory *Memory)
typedef GAME_RELOAD(game_reload);

#define GAME_PROCESS_INPUT(name) void name(game_memory *Memory, game_parameters *Parameters, game_input *Input)
typedef GAME_PROCESS_INPUT(game_process_input);

#define GAME_UPDATE(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_UPDATE(game_update);

#define GAME_RENDER(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_RENDER(game_render);
