#pragma once

#define WIN32_FILE_PATH MAX_PATH

struct win32_platform_state
{
	HWND WindowHandle;
	WINDOWPLACEMENT WindowPlacement;

	i32 WindowWidth;
	i32 WindowHeight;
	b32 IsFullScreen;

	wchar EXEDirectoryFullPath[WIN32_FILE_PATH];

	umm GameMemoryBlockSize;
	void *GameMemoryBlock;

	b32 IsGameRunning;
};

struct win32_game_code
{
	HMODULE GameDLL;
	FILETIME LastWriteTime;

	game_init *Init;
	game_render *Render;

	b32 IsValid;
};

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

inline void
Win32OutputString(char *String)
{
	OutputDebugStringA(String);
	OutputDebugStringA("\n");
}

inline void
Win32OutputString(wchar *String)
{
	OutputDebugStringW(String);
	OutputDebugStringW(L"\n");
}
