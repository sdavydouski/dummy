#pragma once

#include "sandbox_renderer.h"

struct game_memory
{
	umm PermanentStorageSize;
	void *PermanentStorage;

	umm TransientStorageSize;
	void *TransientStorage;

	render_buffer RenderBuffer;
};

struct game_parameters
{
	u32 WindowWidth;
	u32 WindowHeight;
};

#define GAME_INIT(name) void name(game_memory *Memory)
typedef GAME_INIT(game_init);

#define GAME_RENDER(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_RENDER(game_render);
