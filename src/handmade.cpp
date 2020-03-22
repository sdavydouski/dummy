#include <cmath>

#include "handmade_defs.h"
#include "handmade_math.h"
#include "handmade_memory.h"
#include "handmade_platform.h"
#include "handmade.h"

#include "handmade_renderer.cpp"

extern "C" DLLExport
GAME_INIT(GameInit)
{
	game_state *State = (game_state *)Memory->PermanentStorage;

	State->Mode = GameMode_World;
	State->PlayerPosition = vec2(0.f);

	render_commands *RenderCommands = GetRenderCommandsFromMemory(Memory);
	InitRectangle(RenderCommands);
}

extern "C" DLLExport
GAME_PROCESS_INPUT(GameProcessInput)
{
	game_state *State = (game_state *)Memory->PermanentStorage;

	vec2 Move = Input->Move.Range;

	if ((Move.x == -1.f || Move.x == 1.f) && (Move.y == -1.f || Move.y == 1.f))
	{
		Move = Normalize(Move);
	}

	if (Input->Menu.IsActivated)
	{
		if (State->Mode == GameMode_Menu)
		{
			State->Mode = GameMode_World;
		}
		else if (State->Mode == GameMode_World)
		{
			State->Mode = GameMode_Menu;
		}
	}

	State->IsBackgroundHighlighted = Input->HighlightBackground.IsActive;

	switch (State->Mode)
	{
		case GameMode_World:
		{
			State->PlayerPosition += Move * Parameters->Delta * 5.f;
		}
		break;
		case GameMode_Menu:
		{
			
		}
		break;
		default:
		{
			Assert(!"GameMode is not supported");
		}
	}
	
}

extern "C" DLLExport 
GAME_RENDER(GameRender)
{
	game_state *State = (game_state *)Memory->PermanentStorage;
	render_commands *RenderCommands = GetRenderCommandsFromMemory(Memory);

	SetViewport(RenderCommands, 0, 0, Parameters->WindowWidth, Parameters->WindowHeight);
	

	f32 FrustrumWidthInUnits = 20.f;
	f32 PixelsPerUnit = (f32)Parameters->WindowWidth / FrustrumWidthInUnits;
	f32 FrustrumHeightInUnits = (f32)Parameters->WindowHeight / PixelsPerUnit;

	SetOrthographicProjection(RenderCommands,
		-FrustrumWidthInUnits / 2.f, FrustrumWidthInUnits / 2.f,
		-FrustrumHeightInUnits / 2.f, FrustrumHeightInUnits / 2.f,
		-10.f, 10.f
	);

	switch (State->Mode)
	{
		case GameMode_World:
		{
			if (State->IsBackgroundHighlighted)
			{
				Clear(RenderCommands, vec4(0.f, 1.f, 1.f, 1.f));
			}
			else
			{
				Clear(RenderCommands, vec4(0.1f, 0.1f, 0.1f, 1.f));
			}

			DrawRectangle(RenderCommands, State->PlayerPosition, vec2(0.5f), vec4(1.f, 1.f, 0.f, 1.f));
		}
		break;
		case GameMode_Menu:
		{
			Clear(RenderCommands, vec4(1.f, 1.f, 1.f, 1.f));

			DrawRectangle(RenderCommands, vec2(-2.f, 2.f) * vec2(cos(Parameters->Time), 1.f), vec2(0.5f), vec4(1.f, 0.f, 0.f, 1.f));
			DrawRectangle(RenderCommands, vec2(2.f, 2.f) * vec2(1.f, cos(Parameters->Time)), vec2(0.5f), vec4(0.f, 1.f, 0.f, 1.f));
			DrawRectangle(RenderCommands, vec2(2.f, -2.f) * vec2(-cos(Parameters->Time + PI), 1.f), vec2(0.5f), vec4(0.f, 0.f, 1.f, 1.f));
			DrawRectangle(RenderCommands, vec2(-2.f, -2.f) * vec2(1.f, cos(Parameters->Time)), vec2(0.5f), vec4(1.f, 1.f, 0.f, 1.f));
		}
		break;
		default:
		{
			Assert(!"GameMode is not supported");
		}
	}
}
