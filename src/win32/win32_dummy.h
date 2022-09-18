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
    HWND WindowHandle;
    WINDOWPLACEMENT WindowPlacement;
    DWORD WindowStyles;
    
    u64 PerformanceFrequency;

    i32 WindowWidth;
    i32 WindowHeight;

    i32 WindowPositionX;
    i32 WindowPositionY;
    
    i32 ScreenWidth;
    i32 ScreenHeight;

    b32 IsWindowActive;
    b32 IsFullScreen;
    b32 VSync;

    wchar EXEDirectoryFullPath[WIN32_FILE_PATH];

    umm GameMemoryBlockSize;
    void *GameMemoryBlock;

    b32 IsGameRunning;
    f32 TimeRate;

    mouse_mode MouseMode;

    win32_job_queue_sync JobQueueSync;
};

struct win32_game_code
{
    HMODULE GameDLL;
    FILETIME LastWriteTime;

    game_init *Init;
    game_reload *Reload;
    game_process_input *ProcessInput;
    game_update *Update;
    game_render *Render;

    b32 IsValid;
};

struct win32_file_attributes
{
    FILETIME LastWriteTime;
};

struct win32_worker_thread
{
    job_queue *JobQueue;
};
