#include "handmade_defs.h"
#include "handmade_math.h"
#include "handmade_memory.h"
#include "handmade_platform.h"
#include "handmade.h"

#include "handmade_renderer.cpp"

extern "C" DllExport
GAME_INIT(GameInit)
{
	render_commands *RenderCommands = GetRenderCommandsFromMemory(Memory);
	InitRectangle(RenderCommands);
}

extern "C" DllExport 
GAME_RENDER(GameRender)
{
	game_state *State = (game_state *)Memory->PermanentStorage;
	render_commands *RenderCommands = GetRenderCommandsFromMemory(Memory);

	// todo: setup projection matrix

	SetViewport(RenderCommands, 0, 0, Parameters->WindowWidth, Parameters->WindowHeight);
	Clear(RenderCommands, vec4(0.1f, 0.1f, 0.1f, 1.f));

	DrawRectangle(RenderCommands, vec2(-0.5f, 0.5f), vec2(0.5f), vec4(1.f, 0.f, 0.f, 1.f));
	DrawRectangle(RenderCommands, vec2(0.5f, 0.5f), vec2(0.5f), vec4(0.f, 1.f, 0.f, 1.f));
	DrawRectangle(RenderCommands, vec2(0.5f, -0.5f), vec2(0.5f), vec4(0.f, 0.f, 1.f, 1.f));
	DrawRectangle(RenderCommands, vec2(-0.5f, -0.5f), vec2(0.5f), vec4(1.f, 1.f, 0.f, 1.f));
}
