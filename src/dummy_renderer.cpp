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
InitRenderer(render_commands *Commands, u32 GridCount)
{
    render_command_init_renderer *Command = PushRenderCommand(Commands, render_command_init_renderer, RenderCommand_InitRenderer);
    Command->GridCount = GridCount;
}

inline void
AddMesh(
    render_commands *Commands,
    u32 Id,
    primitive_type PrimitiveType,
    u32 VertexCount,
    skinned_vertex *Vertices,
    u32 IndexCount,
    u32 *Indices
)
{
    render_command_add_mesh *Command = PushRenderCommand(Commands, render_command_add_mesh, RenderCommand_AddMesh);
    Command->Id = Id;
    Command->PrimitiveType = PrimitiveType;
    Command->VertexCount = VertexCount;
    Command->Vertices = Vertices;
    Command->IndexCount = IndexCount;
    Command->Indices = Indices;
}

inline void
AddTexture(render_commands *Commands, u32 Id, bitmap *Bitmap)
{
    render_command_add_texture *Command = PushRenderCommand(Commands, render_command_add_texture, RenderCommand_AddTexture);
    Command->Id = Id;
    Command->Bitmap = Bitmap;
}

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
SetCamera(render_commands *Commands, vec3 Eye, vec3 Target, vec3 Up, vec3 Position)
{
    render_command_set_camera *Command =
        PushRenderCommand(Commands, render_command_set_camera, RenderCommand_SetCamera);
    Command->Eye = Eye;
    Command->Target = Target;
    Command->Up = Up;
    Command->Position = Position;
}

inline void
Clear(render_commands *Commands, vec4 Color)
{
    render_command_clear *Command = PushRenderCommand(Commands, render_command_clear, RenderCommand_Clear);
    Command->Color = Color;
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
DrawRectangle(render_commands *Commands, transform Transform, vec4 Color)
{
    render_command_draw_rectangle *Command = PushRenderCommand(Commands, render_command_draw_rectangle, RenderCommand_DrawRectangle);
    Command->Transform = Transform;
    Command->Color = Color;
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

inline void
DrawMesh(
    render_commands *Commands, 
    u32 Id, 
    transform Transform,
    material Material,
    point_light PointLight1,
    point_light PointLight2
)
{
    render_command_draw_mesh *Command = PushRenderCommand(Commands, render_command_draw_mesh, RenderCommand_DrawMesh);
    Command->Id = Id;
    Command->Transform = Transform;
    Command->Material = Material;
    Command->PointLight1 = PointLight1;
    Command->PointLight2 = PointLight2;
}

inline void
DrawSkinnedMesh(
    render_commands *Commands,
    u32 Id,
    transform Transform,
    material Material,
    point_light PointLight1,
    point_light PointLight2,
    u32 SkinningMatrixCount,
    mat4 *SkinningMatrices
)
{
    render_command_draw_skinned_mesh *Command = 
        PushRenderCommand(Commands, render_command_draw_skinned_mesh, RenderCommand_DrawSkinnedMesh);
    Command->Id = Id;
    Command->Transform = Transform;
    Command->Material = Material;
    Command->PointLight1 = PointLight1;
    Command->PointLight2 = PointLight2;
    Command->SkinningMatrixCount = SkinningMatrixCount;
    Command->SkinningMatrices = SkinningMatrices;
}

inline void
SetDirectionalLight(render_commands *Commands, directional_light Light)
{
    render_command_set_directional_light *Command = 
        PushRenderCommand(Commands, render_command_set_directional_light, RenderCommand_SetDirectionalLight);
    Command->Light = Light;
}
