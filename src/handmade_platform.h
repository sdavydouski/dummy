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

struct game_parameters
{
	u32 WindowWidth;
	u32 WindowHeight;

	f32 Time;
	f32 Delta;
};

#define GAME_INIT(name) void name(game_memory *Memory)
typedef GAME_INIT(game_init);

#define GAME_RENDER(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_RENDER(game_render);
