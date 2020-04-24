#include "handmade_renderer.h"

#pragma warning(disable: 26812)

inline void
InitRenderCommandsBuffer(render_commands *Commands, void *Memory, u32 Size)
{
	*Commands = {};

	Commands->RenderCommandsBuffer = Memory;
	Commands->MaxRenderCommandsBufferSize = Size;
	Commands->RenderCommandsBufferSize = 0;
}

inline render_command_header *
PushRenderCommand_(render_commands *Commands, u32 Size, render_command_type Type)
{
	Assert(Commands->RenderCommandsBufferSize + Size < Commands->MaxRenderCommandsBufferSize);

	render_command_header *Result = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + Commands->RenderCommandsBufferSize);
	Result->Type = Type;
	Result->Size = Size;

	Commands->RenderCommandsBufferSize += Size;

	return Result;
}

#define PushRenderCommand(Buffer, Struct, Type) (Struct *)PushRenderCommand_(Buffer, sizeof(Struct), Type)

inline void
SetViewport(render_commands *Commands, u32 x, u32 y, u32 Width, u32 Height)
{
	render_command_set_viewport *Command = PushRenderCommand(Commands, render_command_set_viewport, RenderCommand_SetViewport);
	Command->x = x;
	Command->y = y;
	Command->Width = Width;
	Command->Height = Height;
}

inline void
SetOrthographicProjection(render_commands *Commands, f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far)
{
	render_command_set_orthographic_projection *Command = 
		PushRenderCommand(Commands, render_command_set_orthographic_projection, RenderCommand_SetOrthographicProjection);
	Command->Left = Left;
	Command->Right = Right;
	Command->Bottom = Bottom;
	Command->Top = Top;
	Command->Near = Near;
	Command->Far = Far;
}

inline void
SetPerspectiveProjection(render_commands *Commands, f32 FovY, f32 Aspect, f32 Near, f32 Far)
{
	render_command_set_perspective_projection *Command =
		PushRenderCommand(Commands, render_command_set_perspective_projection, RenderCommand_SetPerspectiveProjection);
	Command->FovY = FovY;
	Command->Aspect = Aspect;
	Command->Near = Near;
	Command->Far = Far;
}

inline void
SetCameraTransform(render_commands *Commands, vec3 Eye, vec3 Target, vec3 Up)
{
	render_command_set_camera_transform *Command =
		PushRenderCommand(Commands, render_command_set_camera_transform, RenderCommand_SetCameraTransform);
	Command->Eye = Eye;
	Command->Target = Target;
	Command->Up = Up;
}

inline void
SetWireframe(render_commands *Commands, b32 IsWireframe)
{
	render_command_set_wireframe *Command = PushRenderCommand(Commands, render_command_set_wireframe, RenderCommand_SetWireframe);
	Command->IsWireframe = IsWireframe;
}

inline void
Clear(render_commands *Commands, vec4 Color)
{
	render_command_clear *Command = PushRenderCommand(Commands, render_command_clear, RenderCommand_Clear);
	Command->Color = Color;
}

inline void
InitLine(render_commands *Commands)
{
	PushRenderCommand(Commands, render_command_init_line, RenderCommand_InitLine);
}

inline void
DrawLine(render_commands *Commands, vec3 Start, vec3 End, vec4 Color, f32 Thickness)
{
	render_command_draw_line *Command = PushRenderCommand(Commands, render_command_draw_line, RenderCommand_DrawLine);
	Command->Start = Start;
	Command->End = End;
	Command->Color = Color;
	Command->Thickness = Thickness;
}

inline void
InitRectangle(render_commands *Commands)
{
	PushRenderCommand(Commands, render_command_init_rectangle, RenderCommand_InitRectangle);
}

inline void
DrawRectangle(render_commands *Commands, vec2 Position, vec2 Size, vec4 Color, vec4 Rotation = vec4(0.f))
{
	render_command_draw_rectangle *Command = PushRenderCommand(Commands, render_command_draw_rectangle, RenderCommand_DrawRectangle);
	Command->Position = Position;
	Command->Size = Size;
	Command->Rotation = Rotation;
	Command->Color = Color;
}

inline void
InitBox(render_commands *Commands)
{
	PushRenderCommand(Commands, render_command_init_box, RenderCommand_InitBox);
}

inline void
DrawBox(render_commands *Commands, vec3 Position, vec3 Size, vec4 Color, vec4 Rotation = vec4(0.f))
{
	render_command_draw_box *Command = PushRenderCommand(Commands, render_command_draw_box, RenderCommand_DrawBox);
	Command->Position = Position;
	Command->Size = Size;
	Command->Rotation = Rotation;
	Command->Color = Color;
}

inline void
InitGrid(render_commands *Commands, u32 Count)
{
	render_command_init_grid *Command = PushRenderCommand(Commands, render_command_init_grid, RenderCommand_InitGrid);
	Command->Count = Count;
}

inline void
DrawGrid(render_commands *Commands, f32 Size, u32 Count, vec3 CameraPosition, vec3 Color)
{
	render_command_draw_grid *Command = PushRenderCommand(Commands, render_command_draw_grid, RenderCommand_DrawGrid);
	Command->Size = Size;
	Command->Count = Count;
	Command->CameraPosition = CameraPosition;
	Command->Color = Color;
}