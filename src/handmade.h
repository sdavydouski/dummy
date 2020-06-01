#pragma once

#include "handmade_renderer.h"

enum game_mode
{
	GameMode_None,
	GameMode_World,
	GameMode_Menu
};

struct game_camera
{
	vec3 Position;
	vec3 Direction;
	vec3 Up;

	f32 Pitch;
	f32 Yaw;
	f32 FovY;
};

struct game_state
{
	memory_arena WorldArena;

	game_mode Mode;
	game_camera DebugCamera;

	u32 RigidBodiesCount;
	rigid_body *RigidBodies;

	plane Ground;

	u32 GridCount;
	b32 IsBackgroundHighlighted;
	b32 Advance;
};
