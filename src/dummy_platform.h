#pragma once

struct profiler_sample
{
    char Name[64];
    u64 ElapsedTicks;
    f32 ElapsedMilliseconds;
};

struct profiler_frame_samples
{
    u32 SampleCount;
    profiler_sample Samples[64];
};

struct platform_profiler
{
    u64 TicksPerSecond;
    i32 CurrentFrameSampleIndex;
    u32 MaxFrameSampleCount;
    profiler_frame_samples *FrameSamples;
};

inline profiler_frame_samples *
ProfilerGetCurrentFrameSamples(platform_profiler *Profiler)
{
    profiler_frame_samples *Result = Profiler->FrameSamples + Profiler->CurrentFrameSampleIndex;
    return Result;
}

inline profiler_frame_samples *
ProfilerGetPreviousFrameSamples(platform_profiler *Profiler)
{
    i32 PreviousFrameSampleIndex = Profiler->CurrentFrameSampleIndex - 1;
    if (PreviousFrameSampleIndex < 0)
    {
        PreviousFrameSampleIndex = Profiler->MaxFrameSampleCount - 1;
    }

    profiler_frame_samples *Result = Profiler->FrameSamples + PreviousFrameSampleIndex;
    return Result;
}

inline void
StoreProfileSample(platform_profiler *Profiler, char *Name, u64 ElapsedTicks)
{
    profiler_sample Sample = {};
    CopyString(Name, Sample.Name);
    Sample.ElapsedTicks = ElapsedTicks;
    Sample.ElapsedMilliseconds = ((f32) ElapsedTicks / (f32) Profiler->TicksPerSecond) * 1000.f;

    profiler_frame_samples *FrameSamples = ProfilerGetCurrentFrameSamples(Profiler);

    FrameSamples->Samples[FrameSamples->SampleCount++] = Sample;
    Assert(FrameSamples->SampleCount < ArrayCount(FrameSamples->Samples));
}

inline void
ProfilerStartFrame(platform_profiler *Profiler)
{
    Profiler->CurrentFrameSampleIndex = (Profiler->CurrentFrameSampleIndex + 1) % Profiler->MaxFrameSampleCount;
    profiler_frame_samples *FrameSamples = ProfilerGetCurrentFrameSamples(Profiler);
    FrameSamples->SampleCount = 0;
}

#ifdef _WIN32
#include <windows.h>

struct win32_auto_profiler
{
    platform_profiler *Profiler;
    char *Name;
    LARGE_INTEGER StartTime;

    win32_auto_profiler(platform_profiler *Profiler, char *Name) : Profiler(Profiler), Name(Name)
    {
        QueryPerformanceCounter(&StartTime);
    }

    ~win32_auto_profiler()
    {
        LARGE_INTEGER EndTime;
        QueryPerformanceCounter(&EndTime);

        u64 ElapsedTicks = EndTime.QuadPart - StartTime.QuadPart;

        StoreProfileSample(Profiler, Name, ElapsedTicks);
    }
};

#define PROFILE(Profiler, Name) win32_auto_profiler Profile(Profiler, (char *) Name)
#else
#error Not implemented
#endif

#define PROFILER_START_FRAME(Profiler) ProfilerStartFrame(Profiler) 

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

    umm RenderCommandsStorageSize;
    void *RenderCommandsStorage;

    platform_api *Platform;
    platform_profiler *Profiler;
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
