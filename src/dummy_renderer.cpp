inline void
InitRenderCommandsBuffer(render_commands *Commands, void *Memory, u32 Size)
{
    *Commands = {};

    Commands->RenderCommandsBuffer = Memory;
    Commands->MaxRenderCommandsBufferSize = Size;
    Commands->RenderCommandsBufferSize = 0;
}

inline render_command_header *
PushRenderCommand_(render_commands *Commands, u32 Size, render_command_type Type, u32 RenderTarget)
{
    Assert(Commands->RenderCommandsBufferSize + Size < Commands->MaxRenderCommandsBufferSize);

    render_command_header *Result = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + Commands->RenderCommandsBufferSize);
    Result->Type = Type;
    Result->Size = Size;
    Result->RenderTarget = RenderTarget;

    Commands->RenderCommandsBufferSize += Size;

    return Result;
}

#define PushRenderCommand(Buffer, Struct, Type, RenderTarget) (Struct *)PushRenderCommand_(Buffer, sizeof(Struct), Type, RenderTarget)

inline void
InitRenderer(render_commands *Commands)
{
    render_command_init_renderer *Command = PushRenderCommand(Commands, render_command_init_renderer, RenderCommand_InitRenderer, 0);
}

inline void
AddMesh(
    render_commands *Commands,
    u32 MeshId,
    u32 VertexCount,
    skinned_vertex *Vertices,
    u32 IndexCount,
    u32 *Indices,
    u32 MaxInstanceCount
)
{
    render_command_add_mesh *Command = PushRenderCommand(Commands, render_command_add_mesh, RenderCommand_AddMesh, 0);
    Command->MeshId = MeshId;
    Command->VertexCount = VertexCount;
    Command->Vertices = Vertices;
    Command->IndexCount = IndexCount;
    Command->Indices = Indices;
    Command->MaxInstanceCount = MaxInstanceCount;
}

inline void
AddTexture(render_commands *Commands, u32 Id, bitmap *Bitmap)
{
    render_command_add_texture *Command = PushRenderCommand(Commands, render_command_add_texture, RenderCommand_AddTexture, 0);
    Command->Id = Id;
    Command->Bitmap = Bitmap;
}

inline void
SetViewport(render_commands *Commands, u32 x, u32 y, u32 Width, u32 Height, u32 RenderTarget = 0)
{
    render_command_set_viewport *Command = PushRenderCommand(Commands, render_command_set_viewport, RenderCommand_SetViewport, RenderTarget);
    Command->x = x;
    Command->y = y;
    Command->Width = Width;
    Command->Height = Height;
}

inline void
SetOrthographicProjection(render_commands *Commands, f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far, u32 RenderTarget = 0)
{
    render_command_set_orthographic_projection *Command = 
        PushRenderCommand(Commands, render_command_set_orthographic_projection, RenderCommand_SetOrthographicProjection, RenderTarget);
    Command->Left = Left;
    Command->Right = Right;
    Command->Bottom = Bottom;
    Command->Top = Top;
    Command->Near = Near;
    Command->Far = Far;
}

inline void
SetPerspectiveProjection(render_commands *Commands, f32 FovY, f32 Aspect, f32 Near, f32 Far, u32 RenderTarget = 0)
{
    render_command_set_perspective_projection *Command =
        PushRenderCommand(Commands, render_command_set_perspective_projection, RenderCommand_SetPerspectiveProjection, RenderTarget);
    Command->FovY = FovY;
    Command->Aspect = Aspect;
    Command->Near = Near;
    Command->Far = Far;
}

inline void
SetCamera(render_commands *Commands, vec3 Position, vec3 Target, vec3 Up, u32 RenderTarget = 0)
{
    render_command_set_camera *Command = PushRenderCommand(Commands, render_command_set_camera, RenderCommand_SetCamera, RenderTarget);
    Command->Position = Position;
    Command->Target = Target;
    Command->Up = Up;
}

inline void
SetTime(render_commands *Commands, f32 Time)
{
    render_command_set_time *Command = PushRenderCommand(Commands, render_command_set_time, RenderCommand_SetTime, 0);
    Command->Time = Time;
}

inline void
Clear(render_commands *Commands, vec4 Color, u32 RenderTarget = 0)
{
    render_command_clear *Command = PushRenderCommand(Commands, render_command_clear, RenderCommand_Clear, RenderTarget);
    Command->Color = Color;
}

inline void
DrawLine(render_commands *Commands, vec3 Start, vec3 End, vec4 Color, f32 Thickness, u32 RenderTarget = 0)
{
    render_command_draw_line *Command = PushRenderCommand(Commands, render_command_draw_line, RenderCommand_DrawLine, RenderTarget);
    Command->Start = Start;
    Command->End = End;
    Command->Color = Color;
    Command->Thickness = Thickness;
}

inline void
DrawRectangle(render_commands *Commands, transform Transform, vec4 Color, u32 RenderTarget = 0)
{
    render_command_draw_rectangle *Command = PushRenderCommand(Commands, render_command_draw_rectangle, RenderCommand_DrawRectangle, RenderTarget);
    Command->Transform = Transform;
    Command->Color = Color;
}

inline void
DrawGround(render_commands *Commands, vec3 CameraPosition, u32 RenderTarget = 0)
{
    render_command_draw_ground *Command = PushRenderCommand(Commands, render_command_draw_ground, RenderCommand_DrawGround, RenderTarget);
    Command->CameraPosition = CameraPosition;
}

inline void
DrawMesh(
    render_commands *Commands, 
    u32 MeshId,
    transform Transform,
    u32 Flags,
    material Material,
    point_light PointLight1,
    point_light PointLight2,
    u32 RenderTarget = 0
)
{
    render_command_draw_mesh *Command = PushRenderCommand(Commands, render_command_draw_mesh, RenderCommand_DrawMesh, RenderTarget);
    Command->MeshId = MeshId;
    Command->Transform = Transform;
    Command->Flags = Flags;
    Command->Material = Material;
    Command->PointLight1 = PointLight1;
    Command->PointLight2 = PointLight2;
}

inline void
DrawSkinnedMesh(
    render_commands *Commands,
    u32 MeshId,
    transform Transform,
    u32 Flags,
    material Material,
    point_light PointLight1,
    point_light PointLight2,
    u32 SkinningMatrixCount,
    mat4 *SkinningMatrices,
    u32 RenderTarget = 0
)
{
    render_command_draw_skinned_mesh *Command = 
        PushRenderCommand(Commands, render_command_draw_skinned_mesh, RenderCommand_DrawSkinnedMesh, RenderTarget);
    Command->MeshId = MeshId;
    Command->Transform = Transform;
    Command->Flags = Flags;
    Command->Material = Material;
    Command->PointLight1 = PointLight1;
    Command->PointLight2 = PointLight2;
    Command->SkinningMatrixCount = SkinningMatrixCount;
    Command->SkinningMatrices = SkinningMatrices;
}

inline void
DrawMeshInstanced(
    render_commands *Commands,
    u32 MeshId,
    u32 InstanceCount,
    render_instance *Instances,
    material Material,
    point_light PointLight1,
    point_light PointLight2,
    u32 RenderTarget = 0
)
{
    render_command_draw_mesh_instanced *Command = 
        PushRenderCommand(Commands, render_command_draw_mesh_instanced, RenderCommand_DrawMeshInstanced, RenderTarget);
    Command->MeshId = MeshId;
    Command->InstanceCount = InstanceCount;
    Command->Instances = Instances;
    Command->Material = Material;
    Command->PointLight1 = PointLight1;
    Command->PointLight2 = PointLight2;
}

inline void
SetDirectionalLight(render_commands *Commands, directional_light Light, u32 RenderTarget = 0)
{
    render_command_set_directional_light *Command = 
        PushRenderCommand(Commands, render_command_set_directional_light, RenderCommand_SetDirectionalLight, RenderTarget);
    Command->Light = Light;
}
