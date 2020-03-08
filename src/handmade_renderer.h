#pragma once

#include "handmade_defs.h"
#include "handmade_math.h"

enum render_command_type
{
	RenderCommand_SetViewport,
	RenderCommand_SetOrthographicProjection,
	RenderCommand_SetPerspectiveProjection,

	RenderCommand_Clear,

	RenderCommand_InitRectangle,
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

struct render_command_set_orthographic_projection
{
	render_command_header Header;
	f32 Left;
	f32 Right;
	f32 Bottom;
	f32 Top;
	f32 Near;
	f32 Far;
};

struct render_command_clear
{
	render_command_header Header;
	vec4 Color;
};

struct render_command_init_rectangle
{
	render_command_header Header;
};

struct render_command_draw_rectangle
{
	render_command_header Header;
	vec2 Position;
	vec2 Size;
	vec4 Color;
};

struct render_commands
{
	u32 MaxRenderCommandsBufferSize;
	u32 RenderCommandsBufferSize;
	void *RenderCommandsBuffer;
};
