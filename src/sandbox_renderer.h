#pragma once

#include "sandbox_defs.h"
#include "sandbox_math.h"

enum render_command_type
{
	RenderCommand_SetViewport,
	RenderCommand_Clear,
	RenderCommand_DrawRectangle
};

struct render_command_header
{
	render_command_type Type;
	u32 Size;
};

struct render_command_set_viewport
{
	render_command_header Header;
	u32 x;
	u32 y;
	u32 Width;
	u32 Height;
};

struct render_command_clear
{
	render_command_header Header;
	vec4 Color;
};

struct render_command_draw_rectangle
{
	render_command_header Header;
	vec2 Min;
	vec2 Max;
	vec4 Color;
};

struct render_buffer
{
	u32 Size;
	u32 Used;
	void *Base;
};
