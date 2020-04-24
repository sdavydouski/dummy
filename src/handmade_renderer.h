#pragma once

#include "handmade_defs.h"
#include "handmade_math.h"

// todo: probably need to have only one "Init" command type where the renderer will do initialization
enum render_command_type
{
	RenderCommand_SetViewport,
	RenderCommand_SetOrthographicProjection,
	RenderCommand_SetPerspectiveProjection,
	RenderCommand_SetCameraTransform,
	RenderCommand_SetWireframe,

	RenderCommand_Clear,

	RenderCommand_InitLine,
	RenderCommand_DrawLine,

	RenderCommand_InitRectangle,
	RenderCommand_DrawRectangle,

	RenderCommand_InitBox,
	RenderCommand_DrawBox,

	RenderCommand_InitGrid,
	RenderCommand_DrawGrid
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

struct render_command_set_perspective_projection
{
	render_command_header Header;
	f32 FovY;
	f32 Aspect;
	f32 Near;
	f32 Far;
};

struct render_command_set_camera_transform
{
	render_command_header Header;
	vec3 Eye;
	vec3 Target;
	vec3 Up;
};

struct render_command_set_wireframe
{
	render_command_header Header;
	b32 IsWireframe;
};

struct render_command_clear
{
	render_command_header Header;
	vec4 Color;
};

struct render_command_init_line
{
	render_command_header Header;
};

struct render_command_draw_line
{
	render_command_header Header;
	vec3 Start;
	vec3 End;
	vec4 Color;
	f32 Thickness;
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
	vec4 Rotation;
	vec4 Color;
};

struct render_command_init_box
{
	render_command_header Header;
};

struct render_command_draw_box
{
	render_command_header Header;
	vec3 Position;
	vec3 Size;
	vec4 Rotation;
	vec4 Color;
};

struct render_command_init_grid
{
	render_command_header Header;
	u32 Count;
};

struct render_command_draw_grid
{
	render_command_header Header;
	f32 Size;
	u32 Count;
	vec3 CameraPosition;
	vec3 Color;
};

struct render_commands
{
	u32 MaxRenderCommandsBufferSize;
	u32 RenderCommandsBufferSize;
	void *RenderCommandsBuffer;
};
