#include <windows.h>

#include <glad/glad.c>

#include "handmade_defs.h"
#include "handmade_platform.h"
#include "handmade_utils.h"
#include "handmade_memory.h"
#include "win32_handmade.h"

#include "win32_handmade_opengl.cpp"

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
            Result.Render = (game_render *)GetProcAddress(Result.GameDLL, "GameRender");

            if (Result.Init && Result.Render)
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
    GameCode->Render = 0;
}

internal void
Win32ToggleFullScreen(win32_platform_state *PlatformState)
{
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353

    DWORD Style = GetWindowLong(PlatformState->WindowHandle, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        if (GetWindowPlacement(PlatformState->WindowHandle, &PlatformState->WindowPlacement) &&
            GetMonitorInfo(MonitorFromWindow(PlatformState->WindowHandle, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(PlatformState->WindowHandle, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);

            SetWindowPos(PlatformState->WindowHandle, HWND_TOP,
                MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(PlatformState->WindowHandle, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(PlatformState->WindowHandle, &PlatformState->WindowPlacement);
        SetWindowPos(PlatformState->WindowHandle, 0, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
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
		}
		break;
		case WM_DESTROY:
		{
			win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);
			PlatformState->IsGameRunning = false;
		}
		break;
		case WM_CLOSE:
		{
			win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);
			PlatformState->IsGameRunning = false;
		}
		break;
		case WM_SIZE:
		{
			win32_platform_state *PlatformState = GetWin32PlatformState(hwnd);
			PlatformState->WindowWidth = LOWORD(lParam);
			PlatformState->WindowHeight = HIWORD(lParam);
		}
		break;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    win32_platform_state PlatformState = {};
    PlatformState.WindowWidth = 1600;
    PlatformState.WindowHeight = 900;
    PlatformState.WindowPlacement = {sizeof(WINDOWPLACEMENT)};
	PlatformState.VSync = true;

	LARGE_INTEGER PerformanceFrequency;
	QueryPerformanceFrequency(&PerformanceFrequency);
	PlatformState.PerformanceFrequency = PerformanceFrequency.QuadPart;

	SetProcessDPIAware();

    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(64);
    GameMemory.TransientStorageSize = Megabytes(256);
    GameMemory.RenderCommandsStorageSize = Megabytes(4);

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

    DWORD WindowStyles = WS_OVERLAPPEDWINDOW;
    RECT Rect = { 0, 0, PlatformState.WindowWidth, PlatformState.WindowHeight };
    AdjustWindowRectEx(&Rect, WindowStyles, false, 0);

    u32 FullWindowWidth = Rect.right - Rect.left;
    u32 FullWindowHeight = Rect.bottom - Rect.top;

    PlatformState.WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, L"Handmade", WindowStyles,
        CW_USEDEFAULT, CW_USEDEFAULT, FullWindowWidth, FullWindowHeight, 0, 0, hInstance, &PlatformState
    );

    if (PlatformState.WindowHandle)
    {
        HDC WindowDC = GetDC(PlatformState.WindowHandle);
        
		opengl_state OpenGLState = {};
        Win32InitOpenGL(&OpenGLState, hInstance, PlatformState.WindowHandle);
		Win32OpenGLSetVSync(&OpenGLState, PlatformState.VSync);

        ShowWindow(PlatformState.WindowHandle, nShowCmd);

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
        io.Fonts->AddFontFromFileTTF("c:/windows/fonts/consola.ttf", 24);

        PlatformState.IsGameRunning = true;

		render_commands *RenderCommands = GetRenderCommandsFromMemory(&GameMemory);

		game_parameters GameParameters = {};

		GameCode.Init(&GameMemory);
		OpenGLProcessRenderCommands(&OpenGLState, RenderCommands);

		LARGE_INTEGER LastPerformanceCounter;
		QueryPerformanceCounter(&LastPerformanceCounter);

		// Game Loop
        while (PlatformState.IsGameRunning)
        {
            MSG WindowMessage = {};
            while (PeekMessage(&WindowMessage, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&WindowMessage);
                DispatchMessage(&WindowMessage);
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

                GameCode.Render(&GameMemory, &GameParameters);
				OpenGLProcessRenderCommands(&OpenGLState, RenderCommands);
            }

            win32_platform_state LastPlatformState = PlatformState;
#if 1
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Stats");

            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Window Size: %d, %d", PlatformState.WindowWidth, PlatformState.WindowHeight);
            ImGui::Checkbox("FullScreen", (bool *)&PlatformState.IsFullScreen);
            ImGui::Checkbox("VSync", (bool *)&PlatformState.VSync);

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
