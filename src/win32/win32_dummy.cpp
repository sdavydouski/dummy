#include <windows.h>

#include "dummy_defs.h"
#include "dummy_platform.h"
#include "dummy_string.h"
#include "dummy_math.h"
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

#include "win32_dummy_opengl.cpp"
#include "dummy_opengl.cpp"

#if 1
#include "dummy_debug.cpp"
#endif

inline void *
Win32AllocateMemory(void *BaseAddress, umm Size)
{
    void *Result = VirtualAlloc(BaseAddress, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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

    i32 CharsWritten = vsnprintf(Buffer, MAX_CHARS, Format, Args);

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
            Result.ProcessInput = (game_process_input *)GetProcAddress(Result.GameDLL, "GameProcessInput");
            Result.Update = (game_update *)GetProcAddress(Result.GameDLL, "GameUpdate");
            Result.Render = (game_render *)GetProcAddress(Result.GameDLL, "GameRender");

            if (Result.Init && Result.ProcessInput && Result.Update && Result.Render)
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
    WIN32_IMGUI_WND_PROC_HANDLER;

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
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
    SavePrevButtonState(&XboxControllerInput->Start);

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

            XboxControllerInput->Start.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START;
            XboxControllerInput->Back.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);

            PrevControllerState = CurrentControllerState;
        }
    }
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

            SetCursorPos(PlatformState->WindowPositionX + WindowCenterX, PlatformState->WindowPositionY + WindowCenterY);

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

    EndProcessMouseInput(MouseInput);
    EndProcessKeyboardInput(KeyboardInput);
}

PLATFORM_SET_MOUSE_MODE(Win32SetMouseMode)
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

PLATFORM_READ_FILE(Win32ReadFile)
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

//
#include <intrin.h>

#define WriteBarrier _WriteBarrier(); _mm_sfence();
#define ReadBarrier _ReadBarrier();

struct win32_thread_proc_parameter
{
    u32 ThreadIndex;
    HANDLE Semaphore;
};

struct work_queue_entry
{
    u32 Data;
};

#define MAX_ENTRY_COUNT 32

global u32 volatile CompletedEntryIndex;
global u32 volatile NextEntryIndex;
global u32 volatile CurrentEntryIndex;
work_queue_entry Entries[MAX_ENTRY_COUNT];

internal void
PushEntry(u32 Data, HANDLE Semaphore)
{
    Assert(CurrentEntryIndex < MAX_ENTRY_COUNT);

    work_queue_entry *Entry = Entries + CurrentEntryIndex;
    Entry->Data = Data;

    WriteBarrier;

    ++CurrentEntryIndex;

    ReleaseSemaphore(Semaphore, 1, 0);
}

DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
{
    win32_thread_proc_parameter *Parameter = (win32_thread_proc_parameter *)lpParameter;

    while (true)
    {
        // todo: InterlockedCompareExchange
        if (NextEntryIndex < CurrentEntryIndex)
        {
            u32 EntryIndex = InterlockedIncrement((LONG volatile *)&NextEntryIndex) - 1;

            ReadBarrier;

            work_queue_entry *Entry = Entries + EntryIndex;

            Win32DebugPrintString("Thread %d: Work #%d\n", Parameter->ThreadIndex, Entry->Data);

            InterlockedIncrement((LONG volatile *)&CompletedEntryIndex);
        }
        else
        {
            WaitForSingleObject(Parameter->Semaphore, INFINITE);
        }
    }

    return 0;
}
//

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#if 0
    // Multithreading playground 
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    u32 ThreadCount = SystemInfo.dwNumberOfProcessors - 1;

    win32_thread_proc_parameter ThreadParameters[32];

    HANDLE Semaphore = CreateSemaphore(0, 0, ThreadCount, 0);

    for (u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex)
    {
        win32_thread_proc_parameter *ThreadParameter = ThreadParameters + ThreadIndex;
        ThreadParameter->ThreadIndex = ThreadIndex;
        ThreadParameter->Semaphore = Semaphore;

        DWORD ThreadId;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, ThreadParameter, 0, &ThreadId);
    }

    for (u32 EntryIndex = 0; EntryIndex < 10; ++EntryIndex)
    {
        work_queue_entry *Entry = Entries + EntryIndex;

        PushEntry(EntryIndex, Semaphore);
    }

    //Sleep(1000);

    for (u32 EntryIndex = 0; EntryIndex < 10; ++EntryIndex)
    {
        work_queue_entry *Entry = Entries + EntryIndex;

        PushEntry(10 + EntryIndex, Semaphore);
    }

    while (CurrentEntryIndex != CompletedEntryIndex);

    Win32DebugPrintString("Work has been completed.\n");
    //
#endif
    SetProcessDPIAware();

    win32_platform_state PlatformState = {};
    PlatformState.WindowWidth = 3200;
    PlatformState.WindowHeight = 1800;
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

    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(256);
    GameMemory.TransientStorageSize = Megabytes(256);
    GameMemory.RenderCommandsStorageSize = Megabytes(4);
    GameMemory.Platform = &PlatformApi;

    // todo: ?
    void *BaseAddress = (void *)Terabytes(2);

    PlatformState.GameMemoryBlockSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize + GameMemory.RenderCommandsStorageSize;
    PlatformState.GameMemoryBlock = Win32AllocateMemory(0, PlatformState.GameMemoryBlockSize);

    GameMemory.PermanentStorage = PlatformState.GameMemoryBlock;
    GameMemory.TransientStorage = (u8 *)PlatformState.GameMemoryBlock + GameMemory.PermanentStorageSize;
    GameMemory.RenderCommandsStorage = (u8 *)PlatformState.GameMemoryBlock + GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;

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

    RegisterClass(&WindowClass);

    PlatformState.WindowStyles = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    RECT Rect = { 0, 0, PlatformState.WindowWidth, PlatformState.WindowHeight };
    AdjustWindowRectEx(&Rect, PlatformState.WindowStyles, false, 0);

    u32 FullWindowWidth = Rect.right - Rect.left;
    u32 FullWindowHeight = Rect.bottom - Rect.top;

    PlatformState.WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, L"Dummy", PlatformState.WindowStyles,
        CW_USEDEFAULT, CW_USEDEFAULT, FullWindowWidth, FullWindowHeight, 0, 0, hInstance, &PlatformState
    );

    if (PlatformState.WindowHandle)
    {
        HDC WindowDC = GetDC(PlatformState.WindowHandle);

        win32_opengl_state Win32OpenGLState = {};

        umm RendererArenaSize = Megabytes(32);
        InitMemoryArena(&Win32OpenGLState.OpenGL.Arena, Win32AllocateMemory(0, RendererArenaSize), RendererArenaSize);

        Win32OpenGLState.OpenGL.Platform = &PlatformApi;

        Win32InitOpenGL(&Win32OpenGLState, hInstance, PlatformState.WindowHandle);
        Win32OpenGLSetVSync(&Win32OpenGLState, PlatformState.VSync);

        PlatformState.WindowPositionX = PlatformState.ScreenWidth / 2 - PlatformState.WindowWidth / 2;
        PlatformState.WindowPositionY = PlatformState.ScreenHeight / 2 - PlatformState.WindowHeight / 2;

        // Center window on screen
        SetWindowPos(PlatformState.WindowHandle, 0, 
            PlatformState.WindowPositionX, PlatformState.WindowPositionY,
            FullWindowWidth, FullWindowHeight, 0
        );

        ShowWindow(PlatformState.WindowHandle, nShowCmd);
        
        Win32SetMouseMode((void *)&PlatformState, MouseMode_Cursor);

        Win32InitImGui(&PlatformState);

        game_parameters GameParameters = {};
        game_input GameInput = {};

        platform_input_keyboard KeyboardInput = {};
        platform_input_mouse MouseInput = {};
        platform_input_xbox_controller XboxControllerInput = {};

        GameCode.Init(&GameMemory);

        LARGE_INTEGER LastPerformanceCounter;
        QueryPerformanceCounter(&LastPerformanceCounter);

        GameParameters.UpdateRate = 1.f / 30.f;
        u32 UpdateCount = 0;
        u32 MaxUpdateCount = 5;

        PlatformState.IsGameRunning = true;

        // Game Loop
        while (PlatformState.IsGameRunning)
        {
            ImGuiIO &DebugInput = ImGui::GetIO();

            Win32ProcessWindowMessages(&PlatformState, &KeyboardInput, &MouseInput);
            Win32ProcessXboxControllerInput(&PlatformState, &XboxControllerInput);

            FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
            if (CompareFileTime(&NewDLLWriteTime, &GameCode.LastWriteTime) != 0)
            {
                Win32UnloadGameCode(&GameCode);
                GameCode = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, GameCodeLockFullPath);
            }

            if (GameCode.IsValid)
            {
                GameParameters.WindowWidth = PlatformState.WindowWidth;
                GameParameters.WindowHeight = PlatformState.WindowHeight;

                // Input
                GameInput = {};

                XboxControllerInput2GameInput(&XboxControllerInput, &GameInput);

                if (!DebugInput.WantCaptureKeyboard)
                {
                    KeyboardInput2GameInput(&KeyboardInput, &GameInput);
                }

                if (!DebugInput.WantCaptureMouse)
                {
                    MouseInput2GameInput(&MouseInput, &GameInput, GameParameters.Delta);
                }

                GameCode.ProcessInput(&GameMemory, &GameParameters, &GameInput);

                // Fixed Update
                GameParameters.UpdateLag += GameParameters.Delta;
                UpdateCount = 0;

                while (GameParameters.UpdateLag >= GameParameters.UpdateRate && UpdateCount < MaxUpdateCount)
                {
                    GameCode.Update(&GameMemory, &GameParameters);

                    GameParameters.UpdateLag -= GameParameters.UpdateRate;
                    UpdateCount++;
                }

                GameParameters.UpdateLag = Min(GameParameters.UpdateLag, GameParameters.UpdateRate);

                // Render
                GameCode.Render(&GameMemory, &GameParameters);

                render_commands *RenderCommands = GetRenderCommands(&GameMemory);
                OpenGLProcessRenderCommands(&Win32OpenGLState.OpenGL, RenderCommands);
                ClearRenderCommands(&GameMemory);
            }

            win32_platform_state LastPlatformState = PlatformState;
#if 1
            RenderDebugInfo(&PlatformState, &GameMemory, &GameParameters);
#endif
            if (LastPlatformState.IsFullScreen != PlatformState.IsFullScreen)
            {
                Win32ToggleFullScreen(&PlatformState);
            }
            if (LastPlatformState.VSync != PlatformState.VSync)
            {
                Win32OpenGLSetVSync(&Win32OpenGLState, PlatformState.VSync);
            }

            SwapBuffers(WindowDC);

            // Counting
            LARGE_INTEGER CurrentPerformanceCounter;
            QueryPerformanceCounter(&CurrentPerformanceCounter);

            if (!DebugInput.WantCaptureKeyboard)
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
            f32 Delta = (f32)DeltaPerformanceCounter / (f32)PlatformState.PerformanceFrequency;
            // ?
            Delta = Min(Delta, 1.f);

            GameParameters.Delta = PlatformState.TimeRate * Delta;
            GameParameters.Time += GameParameters.Delta;

            LastPerformanceCounter = CurrentPerformanceCounter;
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        Win32DeallocateMemory(PlatformState.GameMemoryBlock);

        DestroyWindow(PlatformState.WindowHandle);
        UnregisterClass(WindowClass.lpszClassName, hInstance);
    }

    return 0;
}
