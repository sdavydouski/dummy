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
Clear(render_commands *Commands, vec4 Color)
{
	render_command_clear *Command = PushRenderCommand(Commands, render_command_clear, RenderCommand_Clear);
	Command->Color = Color;
}

inline void
InitRectangle(render_commands *Commands)
{
	render_command_init_rectangle *Command = PushRenderCommand(Commands, render_command_init_rectangle, RenderCommand_InitRectangle);
}

inline void
DrawRectangle(render_commands *Commands, vec2 Position, vec2 Size, vec4 Color)
{
	render_command_draw_rectangle *Command = PushRenderCommand(Commands, render_command_draw_rectangle, RenderCommand_DrawRectangle);
	Command->Position = Position;
	Command->Size = Size;
	Command->Color = Color;
}
