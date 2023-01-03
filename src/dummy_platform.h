#pragma once

struct stream_chunk
{
    u32 Size;
    u8 *Contents;

    stream_chunk *Next;
};

struct stream
{
    stream_chunk *First;
    stream_chunk *Last;

    memory_arena Arena;
};

inline stream_chunk *
AppendChunk(stream *Stream, u32 Size, u8 *Contents)
{
    stream_chunk *Chunk = PushType(&Stream->Arena, stream_chunk);
    Chunk->Size = Size;
    Chunk->Contents = Contents;
    Chunk->Next = 0;

    if (Stream->Last)
    {
        Stream->Last->Next = Chunk;
        Stream->Last = Stream->Last->Next;
    }
    else
    {
        Stream->First = Chunk;
        Stream->Last = Stream->First;
    }

    return Chunk;
}

inline void
ClearStream(stream *Stream)
{
    ClearMemoryArena(&Stream->Arena);
    Stream->First = 0;
    Stream->Last = 0;
}

inline stream
CreateStream(memory_arena Arena)
{
    stream Result = {};

    Result.Arena = Arena;

    return Result;
}

//#define Out(...) Out_((__FILE__, __LINE__, __VA_ARGS__)

internal void
Out(stream *Dest, const char *Format, ...)
{
    va_list Args;

    char String[256];

    va_start(Args, Format);
    u32 Size = FormatStringArgs(String, ArrayCount(String), Format, Args);
    va_end(Args);

    u8 *Contents = PushArray(&Dest->Arena, Size, u8);
    CopyMemory(String, Contents, Size);

    AppendChunk(Dest, Size, Contents);
}

#define PLATFORM_GET_TIMESTAMP(name) u64 name(void)
typedef PLATFORM_GET_TIMESTAMP(platform_get_timestamp);

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

    platform_get_timestamp *GetTimestamp;
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

struct auto_profiler
{
    platform_profiler *Profiler;
    char *Name;
    u64 StartTime;

    auto_profiler(platform_profiler *Profiler, char *Name) : Profiler(Profiler), Name(Name)
    {
        StartTime = Profiler->GetTimestamp();
    }

    ~auto_profiler()
    {
        u64 EndTime = Profiler->GetTimestamp();
        u64 ElapsedTicks = EndTime - StartTime;

        StoreProfileSample(Profiler, Name, ElapsedTicks);
    }
};

#define PROFILE(Profiler, Name) auto_profiler Profile(Profiler, (char *) Name)
//#define PROFILE(...) 

#define PROFILER_START_FRAME(Profiler) ProfilerStartFrame(Profiler)
//#define PROFILER_START_FRAME(...) 

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

struct read_file_options
{
    b32 ReadAsText;
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

#define PLATFORM_READ_FILE(name) read_file_result name(char *FileName, memory_arena *Arena, read_file_options Options)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_WRITE_FILE(name) b32 name(char *FileName, void *Buffer, u32 BufferSize)
typedef PLATFORM_WRITE_FILE(platform_write_file);

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

struct game_parameters
{
    u32 WindowWidth;
    u32 WindowHeight;

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

#define GAME_UPDATE(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_UPDATE(game_update);

#define GAME_RENDER(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_RENDER(game_render);
