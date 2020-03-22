#pragma once

#include "handmade_renderer.h"

enum game_mode
{
	GameMode_None,
	GameMode_World,
	GameMode_Menu
};

struct game_state
{
	game_mode Mode;

	vec2 PlayerPosition;
};
