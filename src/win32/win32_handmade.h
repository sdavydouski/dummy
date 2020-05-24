#pragma once

#define WIN32_FILE_PATH MAX_PATH

#define GET_MOUSE_CURSOR_X(lParam) (i32)((lParam) & 0xFFFF)
#define GET_MOUSE_CURSOR_Y(lParam) (i32)((lParam) >> 16)

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

	b32 HasXboxController;

	mouse_mode MouseMode;
};

struct win32_game_code
{
	HMODULE GameDLL;
	FILETIME LastWriteTime;

	game_init *Init;
	game_process_input *ProcessInput;
	game_update *Update;
	game_render *Render;

	b32 IsValid;
};

inline void
Win32OutputString(const char *String)
{
	OutputDebugStringA(String);
}

inline void
Win32OutputString(wchar *String)
{
	OutputDebugStringW(String);
}
