#include "sandbox_defs.h"
#include "sandbox_math.h"
#include "sandbox_memory.h"
#include "sandbox_platform.h"
#include "sandbox.h"

#include "sandbox_renderer.cpp"

extern "C" DllExport
GAME_INIT(GameInit)
{
	game_state *State = (game_state *)Memory->PermanentStorage;
}

extern "C" DllExport 
GAME_RENDER(GameRender)
{
	game_state *State = (game_state *)Memory->PermanentStorage;

	Memory->RenderBuffer.Used = 0;

	SetViewport(&Memory->RenderBuffer, 0, 0, Parameters->WindowWidth, Parameters->WindowHeight);
	Clear(&Memory->RenderBuffer, vec4(0.1f, 0.1f, 0.1f, 1.f));

	DrawRectangle(&Memory->RenderBuffer, vec2(-0.5f, -0.5f), vec2(0.5f, 0.5f), vec4(1.f, 1.f, 0.f, 1.f));
}
