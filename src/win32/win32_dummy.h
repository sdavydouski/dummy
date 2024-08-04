#pragma once

#define WIN32_FILE_PATH MAX_PATH

#define GET_MOUSE_CURSOR_X(lParam) (i32)(i16)((lParam) & 0xFFFF)
#define GET_MOUSE_CURSOR_Y(lParam) (i32)(i16)((lParam) >> 16)

struct win32_job_queue_sync
{
    CRITICAL_SECTION CriticalSection;
    CONDITION_VARIABLE QueueNotEmpty;
};

struct win32_platform_state
{
    HINSTANCE hInstance;
    HWND WindowHandle;
    WINDOWPLACEMENT WindowPlacement;
    DWORD WindowStyles;
    CRITICAL_SECTION CriticalSection;
    
    u64 PerformanceFrequency;

    i32 WindowWidth;
    i32 WindowHeight;

    i32 WindowPositionX;
    i32 WindowPositionY;

    i32 GameWindowWidth;
    i32 GameWindowHeight;

    i32 GameWindowPositionX;
    i32 GameWindowPositionY;
    
    i32 ScreenWidth;
    i32 ScreenHeight;

    u32 Samples;

    bool32 IsWindowActive;
    bool32 VSync;

    bool32_state IsFullScreen;

    wchar EXEDirectoryFullPath[WIN32_FILE_PATH];

    umm GameMemoryBlockSize;
    void *GameMemoryBlock;

    bool32 IsGameRunning;

    mouse_mode MouseMode;

    win32_job_queue_sync JobQueueSync;

    memory_arena Arena;
    stream Stream;
};

struct win32_game_code
{
    HMODULE GameDLL;
    FILETIME LastWriteTime;

    game_init_func *Init;
    game_reload_func *Reload;
    game_input_func *Input;
    game_update_func *Update;
    game_render_func *Render;
    game_frame_start_func *FrameStart;
    game_frame_end_func *FrameEnd;

    bool32 IsValid;
};

struct win32_file_attributes
{
    FILETIME LastWriteTime;
};

struct win32_worker_thread
{
    job_queue *JobQueue;
};

enum win32_renderer_backend
{
    Renderer_OpenGL,
    Renderer_Direct3D12
};

struct win32_renderer_state
{
    stream Stream;
    memory_arena Arena;
    platform_api *Platform;
    platform_profiler *Profiler;

    win32_renderer_backend Backend;
    union
    {
        opengl_state *OpenGL;
        d3d12_state *Direct3D12;
    };
};

enum win32_audio_backend
{
    Audio_XAudio2
};

struct win32_audio_state
{
    stream Stream;
    memory_arena Arena;
    platform_api *Platform;
    platform_profiler *Profiler;

    win32_audio_backend Backend;
    union
    {
        xaudio2_state *XAudio2;
    };
};
