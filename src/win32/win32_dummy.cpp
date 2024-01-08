#define NOMINMAX
#include <windows.h>
#include <xinput.h>
#include <shobjidl.h>

#include "dummy.h"

#include "win32_dummy_opengl.h"
#include "win32_dummy_d3d12.h"
#include "win32_dummy_xaudio2.h"

#include "win32_dummy.h"
#include "win32_resource.h"

#include "win32_dummy_opengl.cpp"
#include "win32_dummy_d3d12.cpp"
#include "win32_dummy_xaudio2.cpp"

#if EDITOR
#include "win32_dummy_editor.cpp"

#define EDITOR_INIT(...) Win32InitEditor(__VA_ARGS__)
#define EDITOR_RENDER(...) Win32RenderEditor(__VA_ARGS__)
#define EDITOR_SHUTDOWN(...) Win32ShutdownEditor(__VA_ARGS__)
#define EDITOR_WND_PROC_HANDLER(...) if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) { return true; }
#define EDITOR_REMOVE_FOCUS(...) EditorRemoveFocus(__VA_ARGS__)
#define EDITOR_CAPTURE_KEYBOARD_INPUT(...) EditorCaptureKeyboardInput(__VA_ARGS__)
#define EDITOR_CAPTURE_MOUSE_INPUT(...) EditorCaptureMouseInput(__VA_ARGS__)
#else
#define EDITOR_INIT(...)
#define EDITOR_RENDER(...)
#define EDITOR_SHUTDOWN(...)
#define EDITOR_WND_PROC_HANDLER(...)
#define EDITOR_REMOVE_FOCUS(...)
#define EDITOR_CAPTURE_KEYBOARD_INPUT(...) false
#define EDITOR_CAPTURE_MOUSE_INPUT(...) false
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
GetWin32PlatformState(HWND WindowHandle)
{
    win32_platform_state *Result = (win32_platform_state *)GetWindowLongPtr(WindowHandle, GWLP_USERDATA);
    return Result;
}

dummy_internal void
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

inline bool32
Win32FileExists(wchar *FileName)
{
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    bool32 Exists = GetFileAttributesEx(FileName, GetFileExInfoStandard, &Ignored);

    return Exists;
}

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

inline u64
Win32PackHighLow(DWORD High, DWORD Low)
{
    u64 Result = (((u64) High << (u64) 32) | (u64) Low);
    return Result;
}

inline u32
Win32GetFileCount(wchar *Directory)
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFile(Directory, &FindData);

    u32 FileCount = 0;

    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // Directory
            }
            else
            {
                // File
                ++FileCount;
            }
        } 
        while (FindNextFile(FindHandle, &FindData));
    }
    else
    {
        DWORD Error = GetLastError();
        Assert(!"FindFirstFile failed");
    }

    return FileCount;
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

dummy_internal void
Win32InitRenderer(win32_renderer_state *RendererState, win32_platform_state *PlatformState, platform_api *Platform, platform_profiler *Profiler, win32_renderer_backend Backend)
{
    umm RendererArenaSize = Megabytes(32);
    InitMemoryArena(&RendererState->Arena, Win32AllocateMemory(0, RendererArenaSize), RendererArenaSize);

    RendererState->Stream = CreateStream(SubMemoryArena(&RendererState->Arena, Megabytes(4)));
    RendererState->Platform = Platform;
    RendererState->Profiler = Profiler;
    RendererState->Backend = Backend;

    switch (RendererState->Backend)
    {
        case Renderer_OpenGL:
        {
            RendererState->OpenGL = PushType(&RendererState->Arena, opengl_state);

            RendererState->OpenGL->Stream = &RendererState->Stream;
            RendererState->OpenGL->Arena = &RendererState->Arena;
            RendererState->OpenGL->Platform = RendererState->Platform;
            RendererState->OpenGL->Profiler = RendererState->Profiler;

            Win32InitOpenGL(RendererState->OpenGL, PlatformState);

            break;
        }
        case Renderer_Direct3D12:
        {
            RendererState->Direct3D12 = PushType(&RendererState->Arena, d3d12_state);

            RendererState->Direct3D12->Stream = &RendererState->Stream;
            RendererState->Direct3D12->Arena = &RendererState->Arena;
            RendererState->Direct3D12->Platform = RendererState->Platform;
            RendererState->Direct3D12->Profiler = RendererState->Profiler;

            Win32InitDirect3D12(RendererState->Direct3D12, PlatformState);

            break;
        }
    }
}

dummy_internal void
ProcessRenderCommands(win32_renderer_state *RendererState, render_commands *RenderCommands)
{
    switch (RendererState->Backend)
    {
        case Renderer_OpenGL:
        {
            OpenGLProcessRenderCommands(RendererState->OpenGL, RenderCommands);
            break;
        }
        case Renderer_Direct3D12:
        {
            Direct3D12ProcessRenderCommands(RendererState->Direct3D12, RenderCommands);
            break;
        }
    }
}

dummy_internal void
Win32PresentFrame(win32_renderer_state *RendererState, bool32 VSync)
{
    switch (RendererState->Backend)
    {
        case Renderer_OpenGL:
        {
            Win32OpenGLPresentFrame(RendererState->OpenGL, VSync);
            break;
        }
        case Renderer_Direct3D12:
        {
            Direct3D12PresentFrame(RendererState->Direct3D12, VSync);
            break;
        }
    }
}

dummy_internal void
Win32ShutdownRenderer(win32_renderer_state *RendererState)
{
    switch (RendererState->Backend)
    {
        case Renderer_OpenGL:
        {
            Win32ShutdownOpenGL(RendererState->OpenGL);
            break;
        }
        case Renderer_Direct3D12:
        {
            ShutdownDirect3D12(RendererState->Direct3D12);
            break;
        }
    }
}

dummy_internal void
Win32InitAudio(win32_audio_state *AudioState, win32_platform_state *PlatformState, platform_api *Platform, platform_profiler *Profiler, win32_audio_backend Backend)
{
    umm AudioArenaSize = Megabytes(32);
    InitMemoryArena(&AudioState->Arena, Win32AllocateMemory(0, AudioArenaSize), AudioArenaSize);

    AudioState->Stream = CreateStream(SubMemoryArena(&AudioState->Arena, Megabytes(4)));
    AudioState->Platform = Platform;
    AudioState->Profiler = Profiler;
    AudioState->Backend = Backend;

    switch (AudioState->Backend)
    {
        case Audio_XAudio2:
        {
            AudioState->XAudio2 = PushType(&AudioState->Arena, xaudio2_state);

            AudioState->XAudio2->Stream = &AudioState->Stream;
            AudioState->XAudio2->Arena = &AudioState->Arena;
            AudioState->XAudio2->Platform = AudioState->Platform;
            AudioState->XAudio2->Profiler = AudioState->Profiler;

            Win32InitXAudio2(AudioState->XAudio2);

            break;
        }
    }
}

dummy_internal void
ProcessAudioCommands(win32_audio_state *AudioState, audio_commands *AudioCommands)
{
    switch (AudioState->Backend)
    {
        case Audio_XAudio2:
        {
            XAudio2ProcessAudioCommands(AudioState->XAudio2, AudioCommands);
            break;
        }
    }
}

dummy_internal void
Win32ShutdownAudio(win32_audio_state *AudioState)
{
    switch (AudioState->Backend)
    {
        case Audio_XAudio2:
        {
            Win32ShutdownXAudio2(AudioState->XAudio2);
            break;
        }
    }
}

dummy_internal 
PLATFORM_LOAD_FUNCTION(Win32LoadFunction)
{
    win32_platform_state *PlatformState = (win32_platform_state *) PlatformHandle;

    wchar GameCodeDLLFullPath[WIN32_FILE_PATH] = L"";
    ConcatenateString(GameCodeDLLFullPath, PlatformState->EXEDirectoryFullPath);
    ConcatenateString(GameCodeDLLFullPath, L"dummy_temp.dll");

    HMODULE GameDLL = GetModuleHandle(GameCodeDLLFullPath);
    void *Result = GetProcAddress(GameDLL, FunctionName);

    Assert(Result);

    return Result;
}

dummy_internal win32_game_code
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

dummy_internal void
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
    EDITOR_WND_PROC_HANDLER();

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

#if (!EDITOR)
            PlatformState->GameWindowWidth = PlatformState->WindowWidth;
            PlatformState->GameWindowHeight = PlatformState->WindowHeight;
#endif

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

dummy_internal void
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

dummy_internal void
Win32ProcessKeyboardInput(platform_input_keyboard *KeyboardInput, win32_platform_state *PlatformState, MSG *WindowMessage)
{
    u32 VKeyCode = (u32)WindowMessage->wParam;
    bool32 WasKeyPressed = (WindowMessage->lParam & (1u << 30)) != 0;
    bool32 IsKeyPressed = (WindowMessage->lParam & (1u << 31)) == 0;

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
            case 'B':
            {
                KeyboardInput->B.IsPressed = IsKeyPressed;
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
            case VK_BACK:
            {
                KeyboardInput->Backspace.IsPressed = IsKeyPressed;
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
    i32 MouseCursorX = GET_MOUSE_CURSOR_X(WindowMessage->lParam) - PlatformState->GameWindowPositionX;
    i32 MouseCursorY = GET_MOUSE_CURSOR_Y(WindowMessage->lParam) - PlatformState->GameWindowPositionY;

    switch (PlatformState->MouseMode)
    {
        case MouseMode_Navigation:
        {
            i32 WindowCenterX = PlatformState->GameWindowWidth / 2;
            i32 WindowCenterY = PlatformState->GameWindowHeight / 2;

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

dummy_internal void
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
                    bool32 IsLeftMouseDown = WindowMessage.wParam == MK_LBUTTON;
                    bool32 IsRightMouseDown = WindowMessage.wParam == MK_RBUTTON;

                    MouseInput->LeftButton.IsPressed = IsLeftMouseDown;
                    MouseInput->RightButton.IsPressed = IsRightMouseDown;

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

                // pass event to imgui
                TranslateMessage(&WindowMessage);
                DispatchMessage(&WindowMessage);

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

    if (PlatformState->MouseMode == MouseMode_Navigation)
    {
        i32 WindowCenterX = PlatformState->GameWindowPositionX + PlatformState->GameWindowWidth / 2;
        i32 WindowCenterY = PlatformState->GameWindowPositionY + PlatformState->GameWindowHeight / 2;

        if (Abs(MouseInput->dx) > 0 || Abs(MouseInput->dy) > 0)
        {
            SetCursorPos(PlatformState->WindowPositionX + WindowCenterX, PlatformState->WindowPositionY + WindowCenterY);
        }
    }

    EndProcessMouseInput(MouseInput);
    EndProcessKeyboardInput(KeyboardInput);
}

dummy_internal 
PLATFORM_SET_MOUSE_MODE(Win32SetMouseMode)
{
    win32_platform_state *PlatformState = (win32_platform_state *) PlatformHandle;

    if (MouseMode != PlatformState->MouseMode)
    {
        PlatformState->MouseMode = MouseMode;

        switch (PlatformState->MouseMode)
        {
            case MouseMode_Navigation:
            {
                i32 WindowCenterX = PlatformState->GameWindowPositionX + PlatformState->GameWindowWidth / 2;
                i32 WindowCenterY = PlatformState->GameWindowPositionY + PlatformState->GameWindowHeight / 2;

                Win32HideMouseCursor();
                SetCursorPos(PlatformState->WindowPositionX + WindowCenterX, PlatformState->WindowPositionY + WindowCenterY);
                RECT ClipRegion = GetCursorClipRect(PlatformState);
                ClipCursor(&ClipRegion);

                EDITOR_REMOVE_FOCUS();

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

dummy_internal 
PLATFORM_READ_FILE(Win32ReadFile)
{
    read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = (u32)FileSize.QuadPart;
            // Save room for the terminating NULL character. 
            u32 BufferSize = Options.ReadAsText ? FileSize32 + 1 : FileSize32;

            Result.Contents = PushSize(Arena, BufferSize);

            DWORD BytesRead;
            if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && BytesRead == FileSize32)
            {
                Result.Size = FileSize32;
                
                if (Options.ReadAsText)
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

dummy_internal 
PLATFORM_WRITE_FILE(Win32WriteFile)
{
    bool32 Result = false;

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Buffer, BufferSize, &BytesWritten, 0))
        {
            Assert(BytesWritten == BufferSize);
            Result = true;
        }
        else
        {
            DWORD Error = GetLastError();
            Assert(!"WriteFile failed");
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

dummy_internal
PLATFORM_GET_FILES(Win32GetFiles)
{
    get_files_result Result = {};

    u32 FileCount = Win32GetFileCount(Directory);

    Result.FileCount = FileCount;
    Result.Files = PushArray(Arena, Result.FileCount, platform_file);

    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFile(Directory, &FindData);

    u32 FileIndex = 0;

    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // Directory
            }
            else
            {
                // File
                platform_file *File = Result.Files + FileIndex++;

                CopyString(FindData.cFileName, File->FileName);
                File->FileSize = Win32PackHighLow(FindData.nFileSizeHigh, FindData.nFileSizeLow);
                File->FileDate = Win32PackHighLow(FindData.ftLastWriteTime.dwHighDateTime, FindData.ftLastWriteTime.dwLowDateTime);
            }
        }
        while (FindNextFile(FindHandle, &FindData));
    }
    else
    {
        DWORD Error = GetLastError();
        Assert(!"FindFirstFile failed");
    }

    return Result;
}

dummy_internal 
PLATFORM_ENTER_CRITICAL_SECTION(Win32EnterCriticalSection)
{
    win32_platform_state *PlatformState = (win32_platform_state *) PlatformHandle;
    EnterCriticalSection(&PlatformState->CriticalSection);
}

dummy_internal 
PLATFORM_LEAVE_CRITICAL_SECTION(Win32LeaveCriticalSection)
{
    win32_platform_state *PlatformState = (win32_platform_state *) PlatformHandle;
    LeaveCriticalSection(&PlatformState->CriticalSection);
}

dummy_internal 
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

dummy_internal 
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

dummy_internal 
PLATFORM_KICK_JOB_AND_WAIT(Win32KickJobAndWait)
{
    Win32KickJob(JobQueue, Job);

    while (JobQueue->CurrentJobCount > 0) {}
}

dummy_internal 
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

dummy_internal void
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

dummy_internal
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

COMDLG_FILTERSPEC DialogFileTypes[] =
{
    { L"Dummy", L"*.dummy"},
    { L"Text", L"*.txt"},
    { L"All", L"*.*"},
};

dummy_internal
PLATFORM_OPEN_FILE_DIALOG(Win32OpenFileDialog)
{
    IFileOpenDialog *pFileOpen;

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));

    if (SUCCEEDED(hr))
    {
        pFileOpen->SetFileTypes(ArrayCount(DialogFileTypes), DialogFileTypes);

        hr = pFileOpen->Show(0);

        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);

            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                CopyString_(pszFilePath, FilePath, FilePathLength);

                pItem->Release();
            }
        }

        pFileOpen->Release();
    }
}

dummy_internal
PLATFORM_SAVE_FILE_DIALOG(Win32SaveFileDialog)
{
    IFileSaveDialog *pFileSave;

    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileSave));

    if (SUCCEEDED(hr))
    {
        pFileSave->SetFileTypes(ArrayCount(DialogFileTypes), DialogFileTypes);
        pFileSave->SetDefaultExtension(L"*.area");

        hr = pFileSave->Show(0);

        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileSave->GetResult(&pItem);

            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                CopyString_(pszFilePath, FilePath, FilePathLength);

                pItem->Release();
            }
        }

        pFileSave->Release();
    }
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    SetProcessDPIAware();
    CoInitializeEx(0, COINIT_MULTITHREADED);

    win32_platform_state PlatformState = {};

    umm PlatformStateArenaSize = Megabytes(4);
    InitMemoryArena(&PlatformState.Arena, Win32AllocateMemory(0, PlatformStateArenaSize), PlatformStateArenaSize);
    PlatformState.Stream = CreateStream(SubMemoryArena(&PlatformState.Arena, Kilobytes(64)));
    
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

#if 1
    u32 MaxWorkerThreadCount = SystemInfo.dwNumberOfProcessors - 1;
#else
    u32 MaxWorkerThreadCount = 1;
#endif

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
    PlatformState.GameWindowWidth = PlatformState.WindowWidth;
    PlatformState.GameWindowHeight = PlatformState.WindowHeight;
    PlatformState.ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    PlatformState.ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    PlatformState.Samples = 4;
    PlatformState.WindowPlacement = {sizeof(WINDOWPLACEMENT)};
    PlatformState.hInstance = hInstance;
    PlatformState.VSync = true;
    InitValueState(&PlatformState.IsFullScreen, (bool32) true);

    Out(&PlatformState.Stream, "Platform::Worker Thread Count: %d", MaxWorkerThreadCount);
    Out(&PlatformState.Stream, "Platform::Window Size: %d, %d", PlatformState.WindowWidth, PlatformState.WindowHeight);
    Out(&PlatformState.Stream, "Platform::Screen Size: %d, %d", PlatformState.ScreenWidth, PlatformState.ScreenHeight);

    LARGE_INTEGER PerformanceFrequency;
    QueryPerformanceFrequency(&PerformanceFrequency);
    PlatformState.PerformanceFrequency = PerformanceFrequency.QuadPart;

    InitializeCriticalSectionAndSpinCount(&PlatformState.CriticalSection, 0x00000400);

    platform_api PlatformApi = {};
    PlatformApi.PlatformHandle = (void *)&PlatformState;
    PlatformApi.SetMouseMode = Win32SetMouseMode;
    PlatformApi.ReadFile = Win32ReadFile;
    PlatformApi.WriteFile = Win32WriteFile;
    PlatformApi.GetFiles = Win32GetFiles;
    PlatformApi.OpenFileDialog = Win32OpenFileDialog;
    PlatformApi.SaveFileDialog = Win32SaveFileDialog;
    PlatformApi.LoadFunction = Win32LoadFunction;
    PlatformApi.EnterCriticalSection = Win32EnterCriticalSection;
    PlatformApi.LeaveCriticalSection = Win32LeaveCriticalSection;
    PlatformApi.KickJob = Win32KickJob;
    PlatformApi.KickJobs = Win32KickJobs;
    PlatformApi.KickJobAndWait = Win32KickJobAndWait;
    PlatformApi.KickJobsAndWait = Win32KickJobsAndWait;

    platform_profiler PlatformProfiler = {};
    Win32InitProfiler(&PlatformProfiler);

    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(256);
    GameMemory.FrameStorageSize = Megabytes(256);
    GameMemory.AssetsStorageSize = Megabytes(2048);
    GameMemory.RenderCommandsStorageSize = Megabytes(16);
    GameMemory.AudioCommandsStorageSize = Megabytes(16);
    GameMemory.Platform = &PlatformApi;
    GameMemory.Profiler = &PlatformProfiler;
    GameMemory.JobQueue = &JobQueue;

#if RELEASE
    void *BaseAddress = 0;
#else
    void *BaseAddress = (void *)Terabytes(2);
#endif
    PlatformState.GameMemoryBlockSize = (
        GameMemory.PermanentStorageSize + 
        GameMemory.FrameStorageSize + 
        GameMemory.AssetsStorageSize + 
        GameMemory.RenderCommandsStorageSize + 
        GameMemory.AudioCommandsStorageSize
    );
    PlatformState.GameMemoryBlock = Win32AllocateMemory(BaseAddress, PlatformState.GameMemoryBlockSize);

    GameMemory.PermanentStorage = (u8 *) PlatformState.GameMemoryBlock;
    GameMemory.FrameStorage = (u8 *) GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;
    GameMemory.AssetsStorage = (u8 *) GameMemory.FrameStorage + GameMemory.FrameStorageSize;
    GameMemory.RenderCommandsStorage = (u8 *) GameMemory.AssetsStorage + GameMemory.AssetsStorageSize;
    GameMemory.AudioCommandsStorage = (u8 *) GameMemory.RenderCommandsStorage + GameMemory.RenderCommandsStorageSize;

    Win32GetFullPathToEXEDirectory(PlatformState.EXEDirectoryFullPath);

    wchar SourceGameCodeDLLFullPath[WIN32_FILE_PATH] = L"";
    ConcatenateString(SourceGameCodeDLLFullPath, PlatformState.EXEDirectoryFullPath);
    ConcatenateString(SourceGameCodeDLLFullPath, L"dummy.dll");

    wchar TempGameCodeDLLFullPath[WIN32_FILE_PATH] = L"";
    ConcatenateString(TempGameCodeDLLFullPath, PlatformState.EXEDirectoryFullPath);
    ConcatenateString(TempGameCodeDLLFullPath, L"dummy_temp.dll");

    wchar GameCodeLockFullPath[WIN32_FILE_PATH] = L"";
    ConcatenateString(GameCodeLockFullPath, PlatformState.EXEDirectoryFullPath);
    ConcatenateString(GameCodeLockFullPath, L"dummy_lock.dll");

    win32_game_code GameCode = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, GameCodeLockFullPath);

    Assert(GameCode.IsValid);

    WNDCLASS WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = L"Dummy Window Class";
    WindowClass.hCursor = LoadCursor(hInstance, IDC_ARROW);
    WindowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
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
        win32_renderer_state RendererState = {};
        Win32InitRenderer(&RendererState, &PlatformState, &PlatformApi, &PlatformProfiler, Renderer_OpenGL);

        win32_audio_state AudioState = {};
        Win32InitAudio(&AudioState, &PlatformState, &PlatformApi, &PlatformProfiler, Audio_XAudio2);

        // Center window on screen
        PlatformState.WindowPositionX = PlatformState.ScreenWidth / 2 - PlatformState.WindowWidth / 2;
        PlatformState.WindowPositionY = PlatformState.ScreenHeight / 2 - PlatformState.WindowHeight / 2;

        SetWindowPos(PlatformState.WindowHandle, 0, 
            PlatformState.WindowPositionX, PlatformState.WindowPositionY,
            FullWindowWidth, FullWindowHeight, 0
        );

        ShowWindow(PlatformState.WindowHandle, nShowCmd);

        if (PlatformState.IsFullScreen.Value)
        {
            Win32ToggleFullScreen(&PlatformState);
        }
        
        Win32SetMouseMode((void *)&PlatformState, MouseMode_Cursor);

#if EDITOR
        editor_state EditorState = {};
        EditorState.Renderer = &RendererState;
        EditorState.Audio = &AudioState;
        EditorState.Platform = &PlatformApi;

        umm EditorArenaSize = Megabytes(32);
        InitMemoryArena(&EditorState.Arena, Win32AllocateMemory(0, EditorArenaSize), EditorArenaSize);

        EDITOR_INIT(&EditorState, &PlatformState);
#endif

        game_params GameParameters = {};
        game_input GameInput = {};

        platform_input_keyboard KeyboardInput = {};
        platform_input_mouse MouseInput = {};
        platform_input_xbox_controller XboxControllerInput = {};

        LARGE_INTEGER LastPerformanceCounter;
        QueryPerformanceCounter(&LastPerformanceCounter);

        GameParameters.WindowWidth = PlatformState.WindowWidth;
        GameParameters.WindowHeight = PlatformState.WindowHeight;
        GameParameters.Samples = PlatformState.Samples;
        GameParameters.UpdateRate = 1.f / 50.f;
        GameParameters.TimeScale = 1.f;

        PlatformState.IsGameRunning = true;

        GameCode.Init(&GameMemory, &GameParameters);

        // Game Loop
        while (PlatformState.IsGameRunning)
        {
            PROFILER_START_FRAME(&PlatformProfiler);

            if (Changed(PlatformState.IsFullScreen))
            {
                Win32ToggleFullScreen(&PlatformState);
            }

            SavePrevValueState(&PlatformState.IsFullScreen);

#if 1
            char WindowTitle[64];
            FormatString(WindowTitle, "Dummy | %.3f ms, %.1f fps", GameParameters.Delta * 1000.f, 1.f / GameParameters.Delta);
            SetWindowTextA(PlatformState.WindowHandle, WindowTitle);
#endif

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
                GameParameters.WindowWidth = PlatformState.GameWindowWidth;
                GameParameters.WindowHeight = PlatformState.GameWindowHeight;

                // Input
                {
                    PROFILE(&PlatformProfiler, "ProcessInput");

                    XboxControllerInput2GameInput(&XboxControllerInput, &GameInput, GameParameters.Delta);

                    if (!EDITOR_CAPTURE_KEYBOARD_INPUT(&EditorState))
                    {
                        KeyboardInput2GameInput(&KeyboardInput, &GameInput);
                    }

                    if (!EDITOR_CAPTURE_MOUSE_INPUT(&EditorState))
                    {
                        MouseInput2GameInput(&MouseInput, &GameInput);
                    }

                    GameCode.ProcessInput(&GameMemory, &GameParameters, &GameInput);
                }

                // Fixed Update
                {
                    PROFILE(&PlatformProfiler, "FixedUpdate");

                    GameParameters.UpdateAccumulator += GameParameters.Delta;

                    while (GameParameters.UpdateAccumulator >= GameParameters.UpdateRate)
                    {
                        GameCode.Update(&GameMemory, &GameParameters, &GameInput);
                        GameParameters.UpdateAccumulator -= GameParameters.UpdateRate;
                    }

                    GameParameters.UpdateLag = GameParameters.UpdateAccumulator / GameParameters.UpdateRate;

                    Assert(InRange(GameParameters.UpdateLag, 0.f, 1.f));
                }

                // Render
                GameCode.Render(&GameMemory, &GameParameters, &GameInput);

                audio_commands *AudioCommands = GetAudioCommands(&GameMemory);
                ProcessAudioCommands(&AudioState, AudioCommands);
                ClearAudioCommands(&GameMemory);

                render_commands *RenderCommands = GetRenderCommands(&GameMemory);
                ProcessRenderCommands(&RendererState, RenderCommands);
                ClearRenderCommands(&GameMemory);
            }

            {
                PROFILE(&PlatformProfiler, "EDITOR_UI_RENDER");
                EDITOR_RENDER(&EditorState, &PlatformState, &GameMemory, &GameParameters, &GameInput);
            }

            ClearGameInput(&GameInput);

            {
                PROFILE(&PlatformProfiler, "Win32PresentFrame");
                Win32PresentFrame(&RendererState, PlatformState.VSync);
            }

            LARGE_INTEGER CurrentPerformanceCounter;
            QueryPerformanceCounter(&CurrentPerformanceCounter);

            u64 DeltaPerformanceCounter = CurrentPerformanceCounter.QuadPart - LastPerformanceCounter.QuadPart;
            f32 Delta = (f32) DeltaPerformanceCounter / (f32) PlatformState.PerformanceFrequency;

            // If Delta is too large, we must have resumed from a breakpoint - frame-lock to the target rate this frame
            if (Delta > 1.f)
            {
                Delta = 1.f / 60.f;
            }

            GameParameters.UnscaledDelta = Delta;
            GameParameters.UnscaledTime += GameParameters.UnscaledDelta;

            GameParameters.Time = GameParameters.UnscaledTime * GameParameters.TimeScale;
            GameParameters.Delta = GameParameters.UnscaledDelta * GameParameters.TimeScale;

            LastPerformanceCounter = CurrentPerformanceCounter;
        }

        // Cleanup
        EDITOR_SHUTDOWN(&EditorState);

        Win32ShutdownRenderer(&RendererState);
        Win32ShutdownAudio(&AudioState);

        DestroyWindow(PlatformState.WindowHandle);
        UnregisterClass(WindowClass.lpszClassName, hInstance);

        CoUninitialize();
    }

    return 0;
}
