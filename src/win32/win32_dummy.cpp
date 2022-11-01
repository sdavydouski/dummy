#include <windows.h>
#include <xinput.h>

#include "dummy_defs.h"
#include "dummy_math.h"
#include "dummy_random.h"
#include "dummy_memory.h"
#include "dummy_string.h"
#include "dummy_container.h"
#include "dummy_input.h"
#include "dummy_collision.h"
#include "dummy_physics.h"
#include "dummy_visibility.h"
#include "dummy_spatial.h"
#include "dummy_animation.h"
#include "dummy_assets.h"
#include "dummy_audio.h"
#include "dummy_renderer.h"
#include "dummy_job.h"
#include "dummy_platform.h"
#include "win32_dummy.h"

inline FILETIME
Win32GetLastWriteTime(char *FileName)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA FileInformation;
    if (GetFileAttributesExA(FileName, GetFileExInfoStandard, &FileInformation))
    {
        LastWriteTime = FileInformation.ftLastWriteTime;
    }

    return LastWriteTime;
}

#include "win32_dummy_xaudio2.cpp"
#include "dummy_text.cpp"
#include "win32_dummy_opengl.cpp"
#include "dummy_opengl.cpp"

#define DEBUG_UI 1

#if DEBUG_UI
#include "dummy.h"
#include "dummy_debug.cpp"

#define DEBUG_UI_INIT(...) Win32InitImGui(__VA_ARGS__)
#define DEBUG_UI_RENDER(...) Win32RenderDebugInfo(__VA_ARGS__)
#define DEBUG_UI_SHUTDOWN(...) Win32ShutdownImGui(__VA_ARGS__)

#define DEBUG_UI_WND_PROC_HANDLER(...) if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) { return true; }
#define DEBUG_UI_REMOVE_FOCUS(...) ImGui::FocusWindow(0)
#define DEBUG_UI_CAPTURE_KEYBOARD ImGui::GetIO().WantCaptureKeyboard
#define DEBUG_UI_CAPTURE_MOUSE ImGui::GetIO().WantCaptureMouse
#else
#define DEBUG_UI_INIT(...)
#define DEBUG_UI_RENDER(...)
#define DEBUG_UI_SHUTDOWN(...)
#define DEBUG_UI_WND_PROC_HANDLER(...)
#define DEBUG_UI_REMOVE_FOCUS(...)
#define DEBUG_UI_CAPTURE_KEYBOARD false
#define DEBUG_UI_CAPTURE_MOUSE false
#endif

inline void *
Win32AllocateMemory(void *BaseAddress, umm Bytes)
{
    void *Result = VirtualAlloc(BaseAddress, Bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (!Result)
    {
        DWORD Error = GetLastError();
        Assert(!"VirtualAlloc failed");
    }

    return Result;
}

template <typename T>
inline T *
Win32AllocateMemory(umm Count = 1)
{
    T *Result = (T *) Win32AllocateMemory(0, sizeof(T) * Count);
    return Result;
}

inline void
Win32DeallocateMemory(void *Address)
{
    VirtualFree(Address, 0, MEM_RELEASE);
}

inline i32
Win32VDebugPrintString(const char *Format, va_list Args)
{
    const u32 MAX_CHARS = 256;
    char Buffer[MAX_CHARS];

    i32 CharsWritten = vsnprintf_s(Buffer, MAX_CHARS, Format, Args);

    OutputDebugStringA(Buffer);

    return CharsWritten;
}

inline i32
Win32DebugPrintString(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);

    i32 CharsWritten = Win32VDebugPrintString(Format, Args);

    va_end(Args);

    return CharsWritten;
}

inline i32
Win32VDebugPrintString(const wchar *Format, va_list Args)
{
    const u32 MAX_CHARS = 256;
    wchar Buffer[MAX_CHARS];

    i32 CharsWritten = _vsnwprintf_s(Buffer, MAX_CHARS, Format, Args);

    OutputDebugStringW(Buffer);

    return CharsWritten;
}

inline i32
Win32DebugPrintString(const wchar *Format, ...)
{
    va_list Args;
    va_start(Args, Format);

    i32 CharsWritten = Win32VDebugPrintString(Format, Args);

    va_end(Args);

    return CharsWritten;
}

inline win32_platform_state *
GetWin32PlatformState(HWND WindowHandle) {
    win32_platform_state *Result = (win32_platform_state *)GetWindowLongPtr(WindowHandle, GWLP_USERDATA);
    return Result;
}

internal void
Win32GetFullPathToEXEDirectory(wchar *EXEDirectoryFullPath)
{
    wchar EXEFullPath[WIN32_FILE_PATH];
    GetModuleFileName(0, EXEFullPath, ArrayCount(EXEFullPath));

    wchar *OnePastLastEXEFullPathSlash = GetLastAfterDelimiter(EXEFullPath, L'\\');

    for (wchar Index = 0; Index < OnePastLastEXEFullPathSlash - EXEFullPath; ++Index)
    {
        EXEDirectoryFullPath[Index] = EXEFullPath[Index];
    }
}

inline b32
Win32FileExists(wchar *FileName)
{
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    b32 Exists = GetFileAttributesEx(FileName, GetFileExInfoStandard, &Ignored);

    return Exists;
}

inline FILETIME
Win32GetLastWriteTime(wchar *FileName)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA FileInformation;
    if (GetFileAttributesEx(FileName, GetFileExInfoStandard, &FileInformation))
    {
        LastWriteTime = FileInformation.ftLastWriteTime;
    }

    return LastWriteTime;
}

inline void
Win32HideMouseCursor()
{
    i32 DisplayCounter = ShowCursor(false);
    while (DisplayCounter >= 0)
    {
        DisplayCounter = ShowCursor(false);
    }
}

internal PLATFORM_LOAD_FUNCTION(Win32LoadFunction)
{
    win32_platform_state *PlatformState = (win32_platform_state *)PlatformHandle;

    wchar GameCodeDLLFullPath[WIN32_FILE_PATH];
    ConcatenateStrings(PlatformState->EXEDirectoryFullPath, L"dummy_temp.dll", GameCodeDLLFullPath);

    HMODULE GameDLL = GetModuleHandle(GameCodeDLLFullPath);
    void *Result = GetProcAddress(GameDLL, FunctionName);

    Assert(Result);

    return Result;
}

internal win32_game_code
Win32LoadGameCode(wchar *SourceDLLName, wchar *TempDLLName, wchar *LockFileName)
{
    win32_game_code Result = {};

    if (!Win32FileExists(LockFileName))
    {
        Result.LastWriteTime = Win32GetLastWriteTime(SourceDLLName);

        CopyFile(SourceDLLName, TempDLLName, false);

        Result.GameDLL = LoadLibrary(TempDLLName);

        if (Result.GameDLL)
        {
            Result.Init = (game_init *)GetProcAddress(Result.GameDLL, "GameInit");
            Result.Reload = (game_reload *)GetProcAddress(Result.GameDLL, "GameReload");
            Result.ProcessInput = (game_process_input *)GetProcAddress(Result.GameDLL, "GameProcessInput");
            Result.Update = (game_update *)GetProcAddress(Result.GameDLL, "GameUpdate");
            Result.Render = (game_render *)GetProcAddress(Result.GameDLL, "GameRender");

            if (Result.Init && Result.Reload && Result.ProcessInput && Result.Update && Result.Render)
            {
                Result.IsValid = true;
            }
        }
    }

    return Result;
}

inline void
Win32UnloadGameCode(win32_game_code *GameCode)
{
    if (GameCode->GameDLL)
    {
        FreeLibrary(GameCode->GameDLL);
        GameCode->GameDLL = 0;
    }

    GameCode->IsValid = false;
    GameCode->Init = 0;
    GameCode->Reload = 0;
    GameCode->ProcessInput = 0;
    GameCode->Update = 0;
    GameCode->Render = 0;
}

inline RECT
GetCursorClipRect(win32_platform_state *PlatformState)
{
    RECT Rect;

    Rect.left = PlatformState->WindowPositionX;
    Rect.right = Rect.left + PlatformState->WindowWidth;
    Rect.top = PlatformState->WindowPositionY;
    Rect.bottom = Rect.top + PlatformState->WindowHeight;

    return Rect;
}

internal void
Win32ToggleFullScreen(win32_platform_state *PlatformState)
{
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353

    DWORD Style = GetWindowLong(PlatformState->WindowHandle, GWL_STYLE);
    if (Style & PlatformState->WindowStyles)
    {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        if (GetWindowPlacement(PlatformState->WindowHandle, &PlatformState->WindowPlacement) &&
            GetMonitorInfo(MonitorFromWindow(PlatformState->WindowHandle, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(PlatformState->WindowHandle, GWL_STYLE, Style & ~PlatformState->WindowStyles);
            SetWindowPos(PlatformState->WindowHandle, HWND_TOP,
                MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED
            );
        }
    }
    else
    {
        SetWindowLong(PlatformState->WindowHandle, GWL_STYLE, Style | PlatformState->WindowStyles);

        SetWindowPlacement(PlatformState->WindowHandle, &PlatformState->WindowPlacement);
        SetWindowPos(PlatformState->WindowHandle, 0, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED
        );
    }

#if 0
    RECT Rect = GetCursorClipRect(PlatformState);
    ClipCursor(&Rect);
#endif
}

LRESULT CALLBACK WindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    DEBUG_UI_WND_PROC_HANDLER();

    switch (uMsg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
            win32_platform_state *PlatformState = (win32_platform_state *)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)PlatformState);
            break;
        }
        case WM_CLOSE:
        {
            win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);
            PlatformState->IsGameRunning = false;
            break;
        }
        case WM_DESTROY:
        {
            win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);
            PlatformState->IsGameRunning = false;
            break;
        }
        case WM_ACTIVATE:
        {
            win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);

            i32 IsActive = LOWORD(wParam);
            if (IsActive == WA_ACTIVE || IsActive == WA_CLICKACTIVE)
            {
                PlatformState->IsWindowActive = true;
            }
            else if (IsActive == WA_INACTIVE)
            {
                PlatformState->IsWindowActive = false;
            }

            break;
        }
        case WM_MOVE:
        {
            win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);

            PlatformState->WindowPositionX = (i32)LOWORD(lParam);
            PlatformState->WindowPositionY = (i32)HIWORD(lParam);

            break;
        }
        case WM_SIZE :
        {
            win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);

            PlatformState->WindowWidth = LOWORD(lParam);
            PlatformState->WindowHeight = HIWORD(lParam);

            break;
        }
#if 1
        case WM_EXITSIZEMOVE:
        {
            win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);

            RECT Rect = GetCursorClipRect(PlatformState);
            ClipCursor(&Rect);

            break;
        }
#endif
        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    
    return 0;
}

inline vec2
Win32ProcessXboxControllerStick(SHORT sThumbX, SHORT sThumbY, f32 DeadZone)
{
    vec2 StickValue = vec2(0.f);

    if (sThumbX < -DeadZone)
    {
        StickValue.x = (f32)sThumbX / 32768.f;
    }
    else if (sThumbX > DeadZone)
    {
        StickValue.x = (f32)sThumbX / 32767.f;
    }

    if (sThumbY < -DeadZone)
    {
        StickValue.y = (f32)sThumbY / 32768.f;
    }
    else if (sThumbY > DeadZone)
    {
        StickValue.y = (f32)sThumbY / 32767.f;
    }

    return StickValue;
}

inline f32
Win32ProcessXboxControllerTrigger(BYTE Trigger)
{
    f32 Result = 0.f;

    if (Trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
    {
        Result = (f32)Trigger / 255.f;
    }

    return Result;
}

internal void
Win32ProcessXboxControllerInput(win32_platform_state *PlatformState, platform_input_xbox_controller *XboxControllerInput)
{
    BeginProcessXboxControllerInput(XboxControllerInput);

    XINPUT_STATE PrevControllerState = {};
    XINPUT_STATE CurrentControllerState = {};

    if (XInputGetState(0, &CurrentControllerState) == ERROR_SUCCESS)
    {
        if (PrevControllerState.dwPacketNumber != CurrentControllerState.dwPacketNumber)
        {
            XboxControllerInput->LeftStick = Win32ProcessXboxControllerStick(
                CurrentControllerState.Gamepad.sThumbLX, 
                CurrentControllerState.Gamepad.sThumbLY, 
                XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
            );
            XboxControllerInput->RightStick = Win32ProcessXboxControllerStick(
                CurrentControllerState.Gamepad.sThumbRX, 
                CurrentControllerState.Gamepad.sThumbRY, 
                XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
            );

            XboxControllerInput->LeftTrigger = Win32ProcessXboxControllerTrigger(CurrentControllerState.Gamepad.bLeftTrigger);
            XboxControllerInput->RightTrigger = Win32ProcessXboxControllerTrigger(CurrentControllerState.Gamepad.bRightTrigger);

            XboxControllerInput->DradUp.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
            XboxControllerInput->DradDown.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            XboxControllerInput->DradLeft.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            XboxControllerInput->DradRight.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

            XboxControllerInput->Start.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START);
            XboxControllerInput->Back.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);

            XboxControllerInput->LeftThumb.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            XboxControllerInput->RightThumb.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

            XboxControllerInput->A.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A);
            XboxControllerInput->B.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B);
            XboxControllerInput->X.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X);
            XboxControllerInput->Y.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y);

            PrevControllerState = CurrentControllerState;
        }
    }

    EndProcessXboxControllerInput(XboxControllerInput);
}

internal void
Win32ProcessKeyboardInput(platform_input_keyboard *KeyboardInput, win32_platform_state *PlatformState, MSG *WindowMessage)
{
    u32 VKeyCode = (u32)WindowMessage->wParam;
    b32 WasKeyPressed = (WindowMessage->lParam & (1u << 30)) != 0;
    b32 IsKeyPressed = (WindowMessage->lParam & (1u << 31)) == 0;

    if (IsKeyPressed != WasKeyPressed)
    {
        switch (VKeyCode)
        {
            case 'W':
            case VK_UP:
            {
                KeyboardInput->Up.IsPressed = IsKeyPressed;
                break;
            }
            case 'S':
            case VK_DOWN:
            {
                KeyboardInput->Down.IsPressed = IsKeyPressed;
                break;
            }
            case 'A':
            case VK_LEFT:
            {
                KeyboardInput->Left.IsPressed = IsKeyPressed;
                break;
            }
            case 'D':
            case VK_RIGHT:
            {
                KeyboardInput->Right.IsPressed = IsKeyPressed;
                break;
            }
            case 'C':
            {
                KeyboardInput->C.IsPressed = IsKeyPressed;
                break;
            }
            case 'Z':
            {
                KeyboardInput->Z.IsPressed = IsKeyPressed;
                break;
            }
            case 'E':
            {
                KeyboardInput->E.IsPressed = IsKeyPressed;
                break;
            }
            case 'R':
            {
                KeyboardInput->R.IsPressed = IsKeyPressed;
                break;
            }
            case VK_TAB:
            {
                KeyboardInput->Tab.IsPressed = IsKeyPressed;
                break;
            }
            case VK_CONTROL:
            {
                KeyboardInput->Ctrl.IsPressed = IsKeyPressed;
                break;
            }
            case VK_SPACE:
            {
                KeyboardInput->Space.IsPressed = IsKeyPressed;
                break;
            }
            case VK_RETURN:
            {
                KeyboardInput->Enter.IsPressed = IsKeyPressed;
                break;
            }
            case VK_SHIFT:
            {
                KeyboardInput->Shift.IsPressed = IsKeyPressed;
                break;
            }
            case VK_OEM_PLUS:
            case VK_ADD:
            {
                KeyboardInput->Plus.IsPressed = IsKeyPressed;
                break;
            }
            case VK_OEM_MINUS:
            case VK_SUBTRACT:
            {
                KeyboardInput->Minus.IsPressed = IsKeyPressed;
                break;
            }
            case '1':
            case VK_NUMPAD1:
            {
                KeyboardInput->One.IsPressed = IsKeyPressed;
                break;
            }
            case '2':
            case VK_NUMPAD2:
            {
                KeyboardInput->Two.IsPressed = IsKeyPressed;
                break;
            }
            case '3':
            case VK_NUMPAD3:
            {
                KeyboardInput->Three.IsPressed = IsKeyPressed;
                break;
            }
            case '0':
            case VK_NUMPAD0:
            {
                KeyboardInput->Zero.IsPressed = IsKeyPressed;
                break;
            }
            case VK_ESCAPE:
            {
                PlatformState->IsGameRunning = false;
                break;
            }
        }
    }
}

inline void
Win32ProcessMouseInput(platform_input_mouse *MouseInput, win32_platform_state *PlatformState, MSG *WindowMessage)
{
    i32 MouseCursorX = GET_MOUSE_CURSOR_X(WindowMessage->lParam);
    i32 MouseCursorY = GET_MOUSE_CURSOR_Y(WindowMessage->lParam);

    switch (PlatformState->MouseMode)
    {
        case MouseMode_Navigation:
        {
            i32 WindowCenterX = PlatformState->WindowWidth / 2;
            i32 WindowCenterY = PlatformState->WindowHeight / 2;

            MouseInput->dx = MouseCursorX - WindowCenterX;
            MouseInput->dy = MouseCursorY - WindowCenterY;

            break;
        }
        case MouseMode_Cursor:
        {
            MouseInput->x = MouseCursorX;
            MouseInput->y = MouseCursorY;

            break;
        }
        default:
        {
            Assert(!"Mouse mode is not supported");
            break;
        }
    }
}

internal void
Win32ProcessWindowMessages(win32_platform_state *PlatformState, platform_input_keyboard *KeyboardInput, platform_input_mouse *MouseInput)
{
    BeginProcessKeyboardInput(KeyboardInput);
    BeginProcessMouseInput(MouseInput);

    MSG WindowMessage = {};
    while (PeekMessage(&WindowMessage, 0, 0, 0, PM_REMOVE))
    {
        switch (WindowMessage.message)
        {
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                if (PlatformState->IsWindowActive)
                {
                    Win32ProcessKeyboardInput(KeyboardInput, PlatformState, &WindowMessage);
                }

                // pass event to imgui
                TranslateMessage(&WindowMessage);
                DispatchMessage(&WindowMessage);

                break;
            }
            case WM_MOUSEMOVE:
            {
                if (PlatformState->IsWindowActive)
                {
                    Win32ProcessMouseInput(MouseInput, PlatformState, &WindowMessage);
                }

                TranslateMessage(&WindowMessage);
                DispatchMessage(&WindowMessage);

                break;
            }
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            {
                if (PlatformState->IsWindowActive)
                {
                    // todo: mouse input as well
                    b32 IsLeftMouseDown = WindowMessage.wParam == MK_LBUTTON;
                    b32 IsRightMouseDown = WindowMessage.wParam == MK_RBUTTON;

                    MouseInput->LeftButton.IsPressed = IsLeftMouseDown;
                    MouseInput->RightButton.IsPressed = IsRightMouseDown;

                    i32 WindowCenterX = PlatformState->WindowWidth / 2;
                    i32 WindowCenterY = PlatformState->WindowHeight / 2;

                    MouseInput->dx = 0;
                    MouseInput->dy = 0;
                }

                // pass event to imgui
                TranslateMessage(&WindowMessage);
                DispatchMessage(&WindowMessage);

                break;
            }
            case WM_MOUSEWHEEL:
            {
                if (PlatformState->IsWindowActive)
                {
                    MouseInput->WheelDelta = GET_WHEEL_DELTA_WPARAM(WindowMessage.wParam) / WHEEL_DELTA;
                }

                break;
            }
            default:
            {
                TranslateMessage(&WindowMessage);
                DispatchMessage(&WindowMessage);
                break;
            }
        }
    }

    //
    if (PlatformState->MouseMode == MouseMode_Navigation)
    {
        i32 WindowCenterX = PlatformState->WindowWidth / 2;
        i32 WindowCenterY = PlatformState->WindowHeight / 2;

        if (Abs(MouseInput->dx) > 0 || Abs(MouseInput->dy) > 0)
        {
            SetCursorPos(PlatformState->WindowPositionX + WindowCenterX, PlatformState->WindowPositionY + WindowCenterY);
        }
    }
    //

    EndProcessMouseInput(MouseInput);
    EndProcessKeyboardInput(KeyboardInput);
}

internal PLATFORM_SET_MOUSE_MODE(Win32SetMouseMode)
{
    win32_platform_state *PlatformState = (win32_platform_state *)PlatformHandle;

    if (MouseMode != PlatformState->MouseMode)
    {
        PlatformState->MouseMode = MouseMode;

        switch (PlatformState->MouseMode)
        {
            case MouseMode_Navigation:
            {
                Win32HideMouseCursor();
                SetCursorPos(PlatformState->WindowPositionX + PlatformState->WindowWidth / 2, PlatformState->WindowPositionY + PlatformState->WindowHeight / 2);
                RECT ClipRegion = GetCursorClipRect(PlatformState);
                ClipCursor(&ClipRegion);

                DEBUG_UI_REMOVE_FOCUS();

                break;
            }
            case MouseMode_Cursor:
            {
                ShowCursor(true);
                ClipCursor(0);
                break;
            }
            default:
            {
                Assert(!"Mouse mode is not supported");
                break;
            }
        }
    }
}

internal PLATFORM_READ_FILE(Win32ReadFile)
{
    read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = (u32)FileSize.QuadPart;
            // Save room for the terminating NULL character. 
            u32 BufferSize = Text ? FileSize32 + 1 : FileSize32;

            Result.Contents = PushSize(Arena, BufferSize);

            DWORD BytesRead;
            if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && BytesRead == FileSize32)
            {
                Result.Size = FileSize32;
                
                if (Text)
                {
                    u8 *NullTerminator = (u8 *)Result.Contents + BytesRead;
                    *NullTerminator = 0;
                }
            }
            else
            {
                DWORD Error = GetLastError();
                Assert(!"ReadFile failed");
            }
        }
        else
        {
            DWORD Error = GetLastError();
            Assert(!"GetFileSizeEx failed");
        }

        CloseHandle(FileHandle);
    }
    else
    {
        DWORD Error = GetLastError();
        Assert(!"CreateFileA failed");
    }

    return Result;
}

PLATFORM_KICK_JOB(Win32KickJob)
{
    CRITICAL_SECTION *CriticalSection = (CRITICAL_SECTION *) JobQueue->CriticalSection;

    EnterCriticalSection(CriticalSection);

    PutJobIntoQueue(JobQueue, &Job);

    // dear compiler: please don't reorder instructions across this barrier!
    _ReadWriteBarrier();

    CONDITION_VARIABLE *QueueNotEmpty = (CONDITION_VARIABLE *) JobQueue->QueueNotEmpty;
    WakeConditionVariable(QueueNotEmpty);

    LeaveCriticalSection(CriticalSection);
}

PLATFORM_KICK_JOBS(Win32KickJobs)
{
    CRITICAL_SECTION *CriticalSection = (CRITICAL_SECTION *) JobQueue->CriticalSection;

    EnterCriticalSection(CriticalSection);

    PutJobsIntoQueue(JobQueue, JobCount, Jobs);

    // dear compiler: please don't reorder instructions across this barrier!
    _ReadWriteBarrier();

    CONDITION_VARIABLE *QueueNotEmpty = (CONDITION_VARIABLE *) JobQueue->QueueNotEmpty;
    WakeAllConditionVariable(QueueNotEmpty);

    LeaveCriticalSection(CriticalSection);
}

PLATFORM_KICK_JOB_AND_WAIT(Win32KickJobAndWait)
{
    Win32KickJob(JobQueue, Job);

    while (JobQueue->CurrentJobCount > 0) {}
}

PLATFORM_KICK_JOBS_AND_WAIT(Win32KickJobsAndWait)
{
    Win32KickJobs(JobQueue, JobCount, Jobs);

    while (JobQueue->CurrentJobCount > 0) {}
}

DWORD WINAPI WorkerThreadProc(LPVOID lpParam)
{
    Win32DebugPrintString("Worker Thread Id: 0x%x\n", GetCurrentThreadId());

    win32_worker_thread *Thread = (win32_worker_thread *) lpParam;

    job_queue *JobQueue = Thread->JobQueue;

    CRITICAL_SECTION *CriticalSection = (CRITICAL_SECTION *) JobQueue->CriticalSection;
    CONDITION_VARIABLE *QueueNotEmpty = (CONDITION_VARIABLE *) JobQueue->QueueNotEmpty;

    while (true)
    {
        EnterCriticalSection(CriticalSection);

        while (JobQueue->CurrentJobIndex == -1)
        {
            SleepConditionVariableCS(QueueNotEmpty, CriticalSection, INFINITE);
        }

        job *Job = GetNextJobFromQueue(JobQueue);

        LeaveCriticalSection(CriticalSection);

        Job->EntryPoint(JobQueue, Job->Parameters);

        InterlockedDecrement((LONG *) &JobQueue->CurrentJobCount);

        Assert(JobQueue->CurrentJobCount >= 0);
    }

    return 0;
}

internal void
Win32MakeJobQueue(job_queue *JobQueue, u32 WorkerThreadCount, win32_job_queue_sync *JobQueueSync)
{
    win32_worker_thread *WorkerThreads = Win32AllocateMemory<win32_worker_thread>(WorkerThreadCount);

    InitializeCriticalSectionAndSpinCount(&JobQueueSync->CriticalSection, 0x00000400);
    InitializeConditionVariable(&JobQueueSync->QueueNotEmpty);

    JobQueue->CurrentJobIndex = -1;
    JobQueue->CurrentJobCount = 0;
    JobQueue->CriticalSection = &JobQueueSync->CriticalSection;
    JobQueue->QueueNotEmpty = &JobQueueSync->QueueNotEmpty;

    for (u32 WorkerThreadIndex = 0; WorkerThreadIndex < WorkerThreadCount; ++WorkerThreadIndex)
    {
        win32_worker_thread *WorkerThread = WorkerThreads + WorkerThreadIndex;

        WorkerThread->JobQueue = JobQueue;

        HANDLE ThreadHandle = CreateThread(0, 0, WorkerThreadProc, WorkerThread, 0, 0);
        CloseHandle(ThreadHandle);
    }
}

PLATFORM_GET_TIMESTAMP(Win32GetTimeStamp)
{
    LARGE_INTEGER PerformanceCounter;
    QueryPerformanceCounter(&PerformanceCounter);

    u64 Result = PerformanceCounter.QuadPart;

    return Result;
}

inline void
Win32InitProfiler(platform_profiler *Profiler)
{
    LARGE_INTEGER PerformanceFrequency;
    QueryPerformanceFrequency(&PerformanceFrequency);

    Profiler->TicksPerSecond = PerformanceFrequency.QuadPart;
    Profiler->CurrentFrameSampleIndex = 0;
    Profiler->MaxFrameSampleCount = 256;
    Profiler->FrameSamples = Win32AllocateMemory<profiler_frame_samples>(Profiler->MaxFrameSampleCount);
    Profiler->GetTimestamp = Win32GetTimeStamp;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    SetProcessDPIAware();

    win32_platform_state PlatformState = {};
    
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    u32 MaxWorkerThreadCount = SystemInfo.dwNumberOfProcessors - 1;

    job_queue JobQueue = {};
    Win32MakeJobQueue(&JobQueue, MaxWorkerThreadCount, &PlatformState.JobQueueSync);

    HANDLE CurrentThread = GetCurrentThread();
    u32 CurrentProcessorNumber = GetCurrentProcessorNumber();
    SetThreadAffinityMask(CurrentThread, (umm) 1 << CurrentProcessorNumber);

#if 1
    PlatformState.WindowWidth = 3200;
    PlatformState.WindowHeight = 1800;
#else
    PlatformState.WindowWidth = 1600;
    PlatformState.WindowHeight = 900;
#endif
    PlatformState.ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    PlatformState.ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    PlatformState.WindowPlacement = {sizeof(WINDOWPLACEMENT)};
    PlatformState.VSync = false;
    PlatformState.TimeRate = 1.f;

    LARGE_INTEGER PerformanceFrequency;
    QueryPerformanceFrequency(&PerformanceFrequency);
    PlatformState.PerformanceFrequency = PerformanceFrequency.QuadPart;

    platform_api PlatformApi = {};
    PlatformApi.PlatformHandle = (void *)&PlatformState;
    PlatformApi.SetMouseMode = Win32SetMouseMode;
    PlatformApi.ReadFile = Win32ReadFile;
    PlatformApi.DebugPrintString = Win32DebugPrintString;
    PlatformApi.LoadFunction = Win32LoadFunction;
    PlatformApi.KickJob = Win32KickJob;
    PlatformApi.KickJobs = Win32KickJobs;
    PlatformApi.KickJobAndWait = Win32KickJobAndWait;
    PlatformApi.KickJobsAndWait = Win32KickJobsAndWait;

    platform_profiler PlatformProfiler = {};
    Win32InitProfiler(&PlatformProfiler);

    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(256);
    GameMemory.TransientStorageSize = Megabytes(256);
    GameMemory.RenderCommandsStorageSize = Megabytes(16);
    GameMemory.AudioCommandsStorageSize = Megabytes(16);
    GameMemory.Platform = &PlatformApi;
    GameMemory.Profiler = &PlatformProfiler;
    GameMemory.JobQueue = &JobQueue;

    void *BaseAddress = 0;
    PlatformState.GameMemoryBlockSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize + GameMemory.RenderCommandsStorageSize + GameMemory.AudioCommandsStorageSize;
    PlatformState.GameMemoryBlock = Win32AllocateMemory(BaseAddress, PlatformState.GameMemoryBlockSize);

    GameMemory.PermanentStorage = PlatformState.GameMemoryBlock;
    GameMemory.TransientStorage = (u8 *) GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;
    GameMemory.RenderCommandsStorage = (u8 *) GameMemory.TransientStorage + GameMemory.TransientStorageSize;
    GameMemory.AudioCommandsStorage = (u8 *) GameMemory.RenderCommandsStorage + GameMemory.RenderCommandsStorageSize;

    Win32GetFullPathToEXEDirectory(PlatformState.EXEDirectoryFullPath);

    wchar SourceGameCodeDLLFullPath[WIN32_FILE_PATH];
    ConcatenateStrings(PlatformState.EXEDirectoryFullPath, L"dummy.dll", SourceGameCodeDLLFullPath);

    wchar TempGameCodeDLLFullPath[WIN32_FILE_PATH];
    ConcatenateStrings(PlatformState.EXEDirectoryFullPath, L"dummy_temp.dll", TempGameCodeDLLFullPath);

    wchar GameCodeLockFullPath[WIN32_FILE_PATH];
    ConcatenateStrings(PlatformState.EXEDirectoryFullPath, L"dummy_lock.tmp", GameCodeLockFullPath);

    win32_game_code GameCode = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, GameCodeLockFullPath);

    Assert(GameCode.IsValid);

    WNDCLASS WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = L"Dummy Window Class";
    WindowClass.hCursor = LoadCursor(hInstance, IDC_ARROW);
    WindowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    RegisterClass(&WindowClass);

    PlatformState.WindowStyles = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    RECT Rect = { 0, 0, PlatformState.WindowWidth, PlatformState.WindowHeight };
    AdjustWindowRectEx(&Rect, PlatformState.WindowStyles, false, 0);

    u32 FullWindowWidth = Rect.right - Rect.left;
    u32 FullWindowHeight = Rect.bottom - Rect.top;

    PlatformState.WindowHandle = CreateWindowEx(
        0, WindowClass.lpszClassName, L"Dummy", PlatformState.WindowStyles,
        CW_USEDEFAULT, CW_USEDEFAULT, FullWindowWidth, FullWindowHeight, 0, 0, hInstance, &PlatformState
    );

    if (PlatformState.WindowHandle)
    {
        HDC WindowDC = GetDC(PlatformState.WindowHandle);

        // OpenGL
        win32_opengl_state Win32OpenGLState = {};

        umm RendererArenaSize = Megabytes(32);
        InitMemoryArena(&Win32OpenGLState.OpenGL.Arena, Win32AllocateMemory(0, RendererArenaSize), RendererArenaSize);

        Win32OpenGLState.OpenGL.Platform = &PlatformApi;
        Win32OpenGLState.OpenGL.Profiler = &PlatformProfiler;

        Win32InitOpenGL(&Win32OpenGLState, &PlatformState, hInstance);
        Win32OpenGLSetVSync(&Win32OpenGLState, PlatformState.VSync);

        // XAudio2
        xaudio2_state XAudio2State = {};

        umm XAudio2ArenaSize = Megabytes(32);
        InitMemoryArena(&XAudio2State.Arena, Win32AllocateMemory(0, XAudio2ArenaSize), XAudio2ArenaSize);

        Win32InitXAudio2(&XAudio2State);

        PlatformState.WindowPositionX = PlatformState.ScreenWidth / 2 - PlatformState.WindowWidth / 2;
        PlatformState.WindowPositionY = PlatformState.ScreenHeight / 2 - PlatformState.WindowHeight / 2;

        // Center window on screen
        SetWindowPos(PlatformState.WindowHandle, 0, 
            PlatformState.WindowPositionX, PlatformState.WindowPositionY,
            FullWindowWidth, FullWindowHeight, 0
        );

        ShowWindow(PlatformState.WindowHandle, nShowCmd);
        
        Win32SetMouseMode((void *)&PlatformState, MouseMode_Cursor);

        umm DebugArenaSize = Megabytes(32);
        memory_arena DebugArena;
        InitMemoryArena(&DebugArena, Win32AllocateMemory(0, DebugArenaSize), DebugArenaSize);

        DEBUG_UI_INIT(&PlatformState);

        game_parameters GameParameters = {};
        game_input GameInput = {};

        platform_input_keyboard KeyboardInput = {};
        platform_input_mouse MouseInput = {};
        platform_input_xbox_controller XboxControllerInput = {};

        LARGE_INTEGER LastPerformanceCounter;
        QueryPerformanceCounter(&LastPerformanceCounter);

        GameParameters.WindowWidth = PlatformState.WindowWidth;
        GameParameters.WindowHeight = PlatformState.WindowHeight;
        GameParameters.UpdateRate = 1.f / 20.f;

        u32 UpdateCount = 0;
        u32 MaxUpdateCount = 5;

        PlatformState.IsGameRunning = true;

        GameCode.Init(&GameMemory, &GameParameters);

        // Game Loop
        while (PlatformState.IsGameRunning)
        {
            PROFILER_START_FRAME(&PlatformProfiler);

            char WindowTitle[64];
            FormatString(WindowTitle, "Dummy | %.3f ms, %.1f fps", GameParameters.Delta * 1000.f, 1.f / GameParameters.Delta);
            SetWindowTextA(PlatformState.WindowHandle, WindowTitle);

            {
                PROFILE(&PlatformProfiler, "Win32ProcessWindowMessages");
                Win32ProcessWindowMessages(&PlatformState, &KeyboardInput, &MouseInput);
            }
            {
                PROFILE(&PlatformProfiler, "Win32ProcessXboxControllerInput");
                Win32ProcessXboxControllerInput(&PlatformState, &XboxControllerInput);
            }

            {
                PROFILE(&PlatformProfiler, "Win32CheckAndCompareLastDLLWriteTime");

                FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
                if (CompareFileTime(&NewDLLWriteTime, &GameCode.LastWriteTime) != 0)
                {
                    Win32UnloadGameCode(&GameCode);
                    GameCode = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, GameCodeLockFullPath);

                    if (GameCode.IsValid)
                    {
                        GameCode.Reload(&GameMemory);
                    }
                }
            }

            if (GameCode.IsValid)
            {
                GameParameters.WindowWidth = PlatformState.WindowWidth;
                GameParameters.WindowHeight = PlatformState.WindowHeight;

                // Input
                GameInput = {};

                {
                    PROFILE(&PlatformProfiler, "ProcessInput");

                    XboxControllerInput2GameInput(&XboxControllerInput, &GameInput);

                    if (!DEBUG_UI_CAPTURE_KEYBOARD)
                    {
                        KeyboardInput2GameInput(&KeyboardInput, &GameInput);
                    }

                    if (!DEBUG_UI_CAPTURE_MOUSE)
                    {
                        MouseInput2GameInput(&MouseInput, &GameInput);
                    }

                    GameCode.ProcessInput(&GameMemory, &GameParameters, &GameInput);
                }

                // Fixed Update
                {
                    PROFILE(&PlatformProfiler, "FixedUpdate");

                    GameParameters.UpdateLag += GameParameters.Delta;
                    UpdateCount = 0;

                    while (GameParameters.UpdateLag >= GameParameters.UpdateRate && UpdateCount < MaxUpdateCount)
                    {
                        GameCode.Update(&GameMemory, &GameParameters);

                        GameParameters.UpdateLag -= GameParameters.UpdateRate;
                        UpdateCount++;
                    }

                    GameParameters.UpdateLag = Min(GameParameters.UpdateLag, GameParameters.UpdateRate);
                }

                // Render
                GameCode.Render(&GameMemory, &GameParameters);

                audio_commands *AudioCommands = GetAudioCommands(&GameMemory);
                XAudio2ProcessAudioCommands(&XAudio2State, AudioCommands);
                ClearAudioCommands(&GameMemory);

                render_commands *RenderCommands = GetRenderCommands(&GameMemory);
                OpenGLProcessRenderCommands(&Win32OpenGLState.OpenGL, RenderCommands);
                ClearRenderCommands(&GameMemory);
            }

            win32_platform_state LastPlatformState = PlatformState;

            {
                PROFILE(&PlatformProfiler, "DEBUG_UI_RENDER");
                DEBUG_UI_RENDER(&PlatformState, &Win32OpenGLState.OpenGL, &GameMemory, &GameParameters, &DebugArena);
            }

            if (LastPlatformState.IsFullScreen != PlatformState.IsFullScreen)
            {
                Win32ToggleFullScreen(&PlatformState);
            }
            if (LastPlatformState.VSync != PlatformState.VSync)
            {
                Win32OpenGLSetVSync(&Win32OpenGLState, PlatformState.VSync);
            }

            {
                PROFILE(&PlatformProfiler, "SwapBuffers");
                SwapBuffers(WindowDC);
            }

            LARGE_INTEGER CurrentPerformanceCounter;
            QueryPerformanceCounter(&CurrentPerformanceCounter);

            //if (!DEBUG_UI_CAPTURE_KEYBOARD)
            {
                if (IsButtonActivated(&KeyboardInput.Plus))
                {
                    PlatformState.TimeRate *= 2.f;
                }
                if (IsButtonActivated(&KeyboardInput.Minus))
                {
                    PlatformState.TimeRate *= 0.5f;
                }
            }

            Clamp(&PlatformState.TimeRate, 0.125f, 2.f);

            u64 DeltaPerformanceCounter = CurrentPerformanceCounter.QuadPart - LastPerformanceCounter.QuadPart;
            f32 Delta = (f32) DeltaPerformanceCounter / (f32) PlatformState.PerformanceFrequency;
            // ?
            Delta = Min(Delta, 1.f);

            GameParameters.Delta = PlatformState.TimeRate * Delta;
            GameParameters.Time += GameParameters.Delta;

            LastPerformanceCounter = CurrentPerformanceCounter;
        }

        // Cleanup
        DEBUG_UI_SHUTDOWN();

        // todo:
        XAudio2State.XAudio2->StopEngine();

        Win32DeallocateMemory(PlatformState.GameMemoryBlock);

        DestroyWindow(PlatformState.WindowHandle);
        UnregisterClass(WindowClass.lpszClassName, hInstance);
    }

    return 0;
}
