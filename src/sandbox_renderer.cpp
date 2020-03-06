#include "sandbox_renderer.h"

#pragma warning(disable: 26812)

inline void
InitRenderBuffer(render_buffer *Buffer, void *Memory, u32 Size)
{
	Buffer->Base = Memory;
	Buffer->Size = Size;
	Buffer->Used = 0;
}

inline render_command_header *
PushRenderCommand_(render_buffer *Buffer, u32 Size, render_command_type Type)
{
	Assert(Buffer->Used + Size < Buffer->Size);

	render_command_header *Result = (render_command_header *)((u8 *)Buffer->Base + Buffer->Used);
	Result->Type = Type;
	Result->Size = Size;

	Buffer->Used += Size;

	return Result;
}

#define PushRenderCommand(Buffer, Struct, Type) (Struct *)PushRenderCommand_(Buffer, sizeof(Struct), Type)

inline void
SetViewport(render_buffer *Buffer, u32 x, u32 y, u32 Width, u32 Height)
{
	render_command_set_viewport *Command = PushRenderCommand(Buffer, render_command_set_viewport, RenderCommand_SetViewport);
	Command->x = x;
	Command->y = y;
	Command->Width = Width;
	Command->Height = Height;
}

inline void
Clear(render_buffer *Buffer, vec4 Color)
{
	render_command_clear *Command = PushRenderCommand(Buffer, render_command_clear, RenderCommand_Clear);
	Command->Color = Color;
}

inline void
DrawRectangle(render_buffer *Buffer, vec2 Min, vec2 Max, vec4 Color)
{
	render_command_draw_rectangle *Command = PushRenderCommand(Buffer, render_command_draw_rectangle, RenderCommand_DrawRectangle);
	Command->Min = Min;
	Command->Max = Max;
	Command->Color = Color;
}
