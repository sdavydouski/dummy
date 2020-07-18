#include <windows.h>

#include <glad/glad.c>

#include "handmade_defs.h"
#include "handmade_platform.h"
#include "handmade_string.h"
#include "handmade_math.h"
#include "win32_handmade.h"

// for debugging purpuses only!
#include "handmade_collision.h"
#include "handmade_physics.h"
#include "handmade_assets.h"
#include "handmade.h"
//

#include "win32_handmade_opengl.cpp"
#include "handmade_opengl.cpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>

#include "imgui_impl_win32.h"
#include "imgui_impl_win32.cpp"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3.cpp>

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
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
    {
        return true;
    }

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

internal void
Win32ProcessXboxControllerInput(win32_platform_state *PlatformState, platform_input_xbox_controller *XboxControllerInput)
{
    SavePrevButtonState(&XboxControllerInput->Start);

    XINPUT_STATE PrevControllerState = {};
    XINPUT_STATE CurrentControllerState = {};

    if (XInputGetState(0, &CurrentControllerState) == ERROR_SUCCESS)
    {
        PlatformState->HasXboxController = true;

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

            XboxControllerInput->Start.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START;
            XboxControllerInput->Back.IsPressed = (CurrentControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);

            PrevControllerState = CurrentControllerState;
        }
    }
    else
    {
        PlatformState->HasXboxController = false;
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
            }
            break;
            case 'S':
            case VK_DOWN:
            {
                KeyboardInput->Down.IsPressed = IsKeyPressed;
            }
            break;
            case 'A':
            case VK_LEFT:
            {
                KeyboardInput->Left.IsPressed = IsKeyPressed;
            }
            break;
            case 'D':
            case VK_RIGHT:
            {
                KeyboardInput->Right.IsPressed = IsKeyPressed;
            }
            break;
            case VK_TAB:
            {
                KeyboardInput->Tab.IsPressed = IsKeyPressed;
            }
            break;
            case VK_CONTROL:
            {
                KeyboardInput->Ctrl.IsPressed = IsKeyPressed;
            }
            break;
            case VK_SPACE:
            {
                KeyboardInput->Space.IsPressed = IsKeyPressed;
            }
            break;
            case VK_ESCAPE:
            {
                PlatformState->IsGameRunning = false;
            }
            break;
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
    // todo:
    SavePrevButtonState(&KeyboardInput->Tab);
    SavePrevButtonState(&KeyboardInput->Space);

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
            default:
            {
                TranslateMessage(&WindowMessage);
                DispatchMessage(&WindowMessage);
                break;
            }
        }
    }
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

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = (u32)FileSize.QuadPart;
            Result.Contents = PushSize(Arena, FileSize32);

            DWORD BytesRead;
            if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && FileSize32 == BytesRead)
            {
                Result.Size = FileSize32;
            }
            else
            {
                Assert(!"ReadFile failed");
            }
        }
        else
        {
            Assert(!"GetFileSizeEx failed");
        }
    }
    else
    {
        Assert(!"CreateFileA failed");
    }

    return Result;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    SetProcessDPIAware();

    win32_platform_state PlatformState = {};
    PlatformState.WindowWidth = 1600;
    PlatformState.WindowHeight = 900;
    PlatformState.ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    PlatformState.ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    PlatformState.WindowPlacement = {sizeof(WINDOWPLACEMENT)};
	PlatformState.VSync = false;

    LARGE_INTEGER PerformanceFrequency;
    QueryPerformanceFrequency(&PerformanceFrequency);
    PlatformState.PerformanceFrequency = PerformanceFrequency.QuadPart;

    platform_api PlatformApi = {};
    PlatformApi.PlatformHandle = (void *)&PlatformState;
    PlatformApi.SetMouseMode = Win32SetMouseMode;
    PlatformApi.ReadFile = Win32ReadFile;

    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(64);
    GameMemory.TransientStorageSize = Megabytes(256);
    GameMemory.RenderCommandsStorageSize = Megabytes(4);
    GameMemory.Platform = &PlatformApi;

    void *BaseAddress = (void *)Terabytes(2);

    PlatformState.GameMemoryBlockSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize + GameMemory.RenderCommandsStorageSize;
    PlatformState.GameMemoryBlock = Win32AllocateMemory(BaseAddress, PlatformState.GameMemoryBlockSize);

    GameMemory.PermanentStorage = PlatformState.GameMemoryBlock;
    GameMemory.TransientStorage = (u8 *)PlatformState.GameMemoryBlock + GameMemory.PermanentStorageSize;
    GameMemory.RenderCommandsStorage = (u8 *)PlatformState.GameMemoryBlock + GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;

    Win32GetFullPathToEXEDirectory(PlatformState.EXEDirectoryFullPath);

    wchar SourceGameCodeDLLFullPath[WIN32_FILE_PATH];
    ConcatenateStrings(PlatformState.EXEDirectoryFullPath, L"handmade.dll", SourceGameCodeDLLFullPath);

    wchar TempGameCodeDLLFullPath[WIN32_FILE_PATH];
    ConcatenateStrings(PlatformState.EXEDirectoryFullPath, L"handmade_temp.dll", TempGameCodeDLLFullPath);

    wchar GameCodeLockFullPath[WIN32_FILE_PATH];
    ConcatenateStrings(PlatformState.EXEDirectoryFullPath, L"handmade_lock.tmp", GameCodeLockFullPath);

    win32_game_code GameCode = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, GameCodeLockFullPath);

	Assert(GameCode.IsValid);

    WNDCLASS WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = L"Handmade Window Class";
    WindowClass.hCursor = LoadCursor(hInstance, IDC_ARROW);

    RegisterClass(&WindowClass);

    PlatformState.WindowStyles = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    RECT Rect = { 0, 0, PlatformState.WindowWidth, PlatformState.WindowHeight };
    AdjustWindowRectEx(&Rect, PlatformState.WindowStyles, false, 0);

    u32 FullWindowWidth = Rect.right - Rect.left;
    u32 FullWindowHeight = Rect.bottom - Rect.top;

    PlatformState.WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, L"Handmade", PlatformState.WindowStyles,
        CW_USEDEFAULT, CW_USEDEFAULT, FullWindowWidth, FullWindowHeight, 0, 0, hInstance, &PlatformState
    );

    if (PlatformState.WindowHandle)
    {
        HDC WindowDC = GetDC(PlatformState.WindowHandle);

		opengl_state OpenGLState = {};

        // todo: do I really need memory arena inside renderer?
        umm RendererArenaSize = Megabytes(32);
        InitMemoryArena(&OpenGLState.Arena, Win32AllocateMemory(0, RendererArenaSize), RendererArenaSize);
        Win32InitOpenGL(&OpenGLState, hInstance, PlatformState.WindowHandle);
		Win32OpenGLSetVSync(&OpenGLState, PlatformState.VSync);

        PlatformState.WindowPositionX = PlatformState.ScreenWidth / 2 - PlatformState.WindowWidth / 2;
        PlatformState.WindowPositionY = PlatformState.ScreenHeight / 2 - PlatformState.WindowHeight / 2;

        // Center window on screen
        SetWindowPos(PlatformState.WindowHandle, 0, 
            PlatformState.WindowPositionX, PlatformState.WindowPositionY,
            FullWindowWidth, FullWindowHeight, 0
        );

        ShowWindow(PlatformState.WindowHandle, nShowCmd);
        
        Win32SetMouseMode((void *)&PlatformState, MouseMode_Cursor);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer bindings
        ImGui_ImplWin32_Init(PlatformState.WindowHandle);
        ImGui_ImplOpenGL3_Init();

        // Load Fonts
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Consola.ttf", 16);

		render_commands *RenderCommands = GetRenderCommandsFromMemory(&GameMemory);

		game_parameters GameParameters = {};
        game_input GameInput = {};

        platform_input_keyboard KeyboardInput = {};
        platform_input_mouse MouseInput = {};
        platform_input_xbox_controller XboxControllerInput = {};

		GameCode.Init(&GameMemory);
		OpenGLProcessRenderCommands(&OpenGLState, RenderCommands);

		LARGE_INTEGER LastPerformanceCounter;
		QueryPerformanceCounter(&LastPerformanceCounter);

        GameParameters.UpdateRate = 1.f / 30.f;
        f32 UpdateLag = 0.f;
        u32 UpdateCount = 0;
        u32 MaxUpdateCount = 5;

        PlatformState.IsGameRunning = true;

		// Game Loop
        while (PlatformState.IsGameRunning)
        {
            Win32ProcessWindowMessages(&PlatformState, &KeyboardInput, &MouseInput);
            if (PlatformState.IsWindowActive)
            {
                Win32ProcessXboxControllerInput(&PlatformState, &XboxControllerInput);
            }

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

                if (PlatformState.HasXboxController)
                {
                    XboxControllerInput2GameInput(&XboxControllerInput, &GameInput);
                }
                else
                {
                    KeyboardInput2GameInput(&KeyboardInput, &GameInput);
                    MouseInput2GameInput(&MouseInput, &GameInput, GameParameters.Delta);
                }

                GameCode.ProcessInput(&GameMemory, &GameParameters, &GameInput);

                UpdateLag += GameParameters.Delta;
                UpdateCount = 0;

                while (UpdateLag >= GameParameters.UpdateRate && UpdateCount < MaxUpdateCount)
                {
                    GameCode.Update(&GameMemory, &GameParameters);

                    UpdateLag -= GameParameters.UpdateRate;
                    UpdateCount++;
                }

                // todo: pass interpolated normalized update value (UpdateLag / UpdateRage) ?
                GameCode.Render(&GameMemory, &GameParameters);
				OpenGLProcessRenderCommands(&OpenGLState, RenderCommands);
            }

            win32_platform_state LastPlatformState = PlatformState;
#if 1
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            game_state *GameState = (game_state *)GameMemory.PermanentStorage;

            ImGui::Begin("Stats");

            ImGui::Text("%.3f ms/frame (%.1f FPS)", GameParameters.Delta * 1000.f, 1.f / GameParameters.Delta);
            ImGui::Text("Window Size: %d, %d", PlatformState.WindowWidth, PlatformState.WindowHeight);
            ImGui::Checkbox("FullScreen", (bool *)&PlatformState.IsFullScreen);
            ImGui::Checkbox("VSync", (bool *)&PlatformState.VSync);

            ImGui::End();

            f32 Margin = 10.f;
            ImVec2 ContainerSize = ImVec2(400.f, 350.f);
            ImGui::SetNextWindowSize(ContainerSize);
            ImGui::SetNextWindowPos(ImVec2(PlatformState.WindowWidth - ContainerSize.x - Margin, Margin));

            ImGui::Begin("Game State");

            ImGui::Text("Animations:");

            for (u32 AnimationIndex = 0; AnimationIndex < GameState->AnimationCount; ++AnimationIndex)
            {
                animation_clip *Animation = GameState->Animations + AnimationIndex;

                if (ImGui::RadioButton(Animation->Name, Animation == GameState->CurrentAnimation))
                {
                    GameState->CurrentAnimation = GameState->Animations + AnimationIndex;
                    GameState->CurrentTime = 0.f;
                }
            }

            ImGui::SliderFloat("Playback Rate", &GameState->PlaybackRate, 0.1f, 2.f);

            ImGui::End();

            // Rendering
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
            if (LastPlatformState.IsFullScreen != PlatformState.IsFullScreen)
            {
                Win32ToggleFullScreen(&PlatformState);
            }
			if (LastPlatformState.VSync != PlatformState.VSync)
			{
				Win32OpenGLSetVSync(&OpenGLState, PlatformState.VSync);
			}

            SwapBuffers(WindowDC);

			// Counting
			LARGE_INTEGER CurrentPerformanceCounter;
			QueryPerformanceCounter(&CurrentPerformanceCounter);

			u64 DeltaPerformanceCounter = CurrentPerformanceCounter.QuadPart - LastPerformanceCounter.QuadPart;
			GameParameters.Delta = (f32)DeltaPerformanceCounter / (f32)PlatformState.PerformanceFrequency;
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
