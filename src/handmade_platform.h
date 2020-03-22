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

struct game_input_action
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
	game_input_action Action;
	game_input_action Jump;
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
