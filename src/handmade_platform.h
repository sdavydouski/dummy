#pragma once

#include "handmade_renderer.h"

struct game_memory
{
	umm PermanentStorageSize;
	void *PermanentStorage;

	umm TransientStorageSize;
	void *TransientStorage;

	umm RenderCommandsStorageSize;
	void *RenderCommandsStorage;
};

inline render_commands *
GetRenderCommandsFromMemory(game_memory *Memory)
{
	render_commands *RenderCommands = (render_commands *)Memory->RenderCommandsStorage;
	RenderCommands->MaxRenderCommandsBufferSize = (u32)(Memory->RenderCommandsStorageSize - sizeof(render_commands));
	RenderCommands->RenderCommandsBufferSize = 0;
	RenderCommands->RenderCommandsBuffer = (u8 *)Memory->RenderCommandsStorage + sizeof(render_commands);

	return RenderCommands;
}

struct platform_button_state
{
	b32 IsPressed;
	b32 WasPressed;
};

inline void
SavePrevButtonState(platform_button_state *Button)
{
	Button->WasPressed = Button->IsPressed;
}

struct platform_input_keyboard
{
	platform_button_state Up;
	platform_button_state Down;
	platform_button_state Left;
	platform_button_state Right;

	platform_button_state Tab;
	platform_button_state Ctrl;
};

struct platform_input_xbox_controller
{
	vec2 LeftStick;

	platform_button_state Start;
	platform_button_state Back;
};

struct game_input_action
{
	b32 IsActivated;
};

struct game_input_state
{
	b32 IsActive;
};

struct game_input_range
{
	vec2 Range;
};

struct game_input
{
	game_input_range Move;
	game_input_action Menu;
	game_input_state HighlightBackground;
};

struct game_parameters
{
	u32 WindowWidth;
	u32 WindowHeight;

	f32 Time;
	f32 Delta;
};

#define GAME_INIT(name) void name(game_memory *Memory)
typedef GAME_INIT(game_init);

#define GAME_PROCESS_INPUT(name) void name(game_memory *Memory, game_parameters *Parameters, game_input *Input)
typedef GAME_PROCESS_INPUT(game_process_input);

#define GAME_RENDER(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_RENDER(game_render);
