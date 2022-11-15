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
AddMesh(
    render_commands *Commands,
    u32 MeshId,
    u32 VertexCount,
    vec3 *Positions,
    vec3 *Normals,
    vec3 *Tangents,
    vec3 *Bitangents,
    vec2 *TextureCoords,
    vec4 *Weights,
    ivec4 *JointIndices,
    u32 IndexCount,
    u32 *Indices
)
{
    render_command_add_mesh *Command = PushRenderCommand(Commands, render_command_add_mesh, RenderCommand_AddMesh);
    Command->MeshId = MeshId;
    Command->VertexCount = VertexCount;
    Command->Positions = Positions;
    Command->Normals = Normals;
    Command->Tangents = Bitangents;
    Command->Bitangents = Bitangents;
    Command->TextureCoords = TextureCoords;
    Command->Weights = Weights;
    Command->JointIndices = JointIndices;
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
AddSkinningBuffer(render_commands *Commands, u32 SkinningBufferId, u32 SkinningMatrixCount)
{
    render_command_add_skinning_buffer *Command = PushRenderCommand(Commands, render_command_add_skinning_buffer, RenderCommand_AddSkinningBuffer);

    Command->SkinningBufferId = SkinningBufferId;
    Command->SkinningMatrixCount = SkinningMatrixCount;
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
SetScreenProjection(render_commands *Commands, f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far)
{
    render_command_set_screen_projection *Command = 
        PushRenderCommand(Commands, render_command_set_screen_projection, RenderCommand_SetScreenProjection);
    Command->Left = Left;
    Command->Right = Right;
    Command->Bottom = Bottom;
    Command->Top = Top;
    Command->Near = Near;
    Command->Far = Far;
}

inline void
SetWorldProjection(render_commands *Commands, f32 FovY, f32 Aspect, f32 Near, f32 Far)
{
    render_command_set_world_projection *Command =
        PushRenderCommand(Commands, render_command_set_world_projection, RenderCommand_SetWorldProjection);
    Command->FovY = FovY;
    Command->Aspect = Aspect;
    Command->Near = Near;
    Command->Far = Far;
}

inline void
SetCamera(render_commands *Commands, vec3 Position, vec3 Direction, vec3 Up)
{
    render_command_set_camera *Command = PushRenderCommand(Commands, render_command_set_camera, RenderCommand_SetCamera);
    Command->Position = Position;
    Command->Direction = Direction;
    Command->Up = Up;
}

inline void
SetTime(render_commands *Commands, f32 Time)
{
    render_command_set_time *Command = PushRenderCommand(Commands, render_command_set_time, RenderCommand_SetTime);
    Command->Time = Time;
}

inline void
Clear(render_commands *Commands, vec4 Color)
{
    render_command_clear *Command = PushRenderCommand(Commands, render_command_clear, RenderCommand_Clear);
    Command->Color = Color;
}

inline void
DrawPoint(render_commands *Commands, vec3 Position, vec4 Color, f32 Size)
{
    render_command_draw_point *Command = PushRenderCommand(Commands, render_command_draw_point, RenderCommand_DrawPoint);
    Command->Position = Position;
    Command->Color = Color;
    Command->Size = Size;
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
DrawBox(render_commands *Commands, transform Transform, vec4 Color)
{
    render_command_draw_box *Command = PushRenderCommand(Commands, render_command_draw_box, RenderCommand_DrawBox);
    Command->Transform = Transform;
    Command->Color = Color;
}

inline void
DrawText(
    render_commands *Commands,
    const wchar *Text,
    font *Font,
    vec3 Position,
    f32 Scale,
    vec4 Color,
    draw_text_alignment Alignment,
    draw_text_mode Mode,
    b32 DepthEnabled = false
)
{
    render_command_draw_text *Command = PushRenderCommand(Commands, render_command_draw_text, RenderCommand_DrawText);
    CopyString(Text, Command->Text);
    Command->Font = Font;
    Command->Position = Position;
    Command->Scale = Scale;
    Command->Color = Color;
    Command->Alignment = Alignment;
    Command->Mode = Mode;
    Command->DepthEnabled = DepthEnabled;
}

inline void
DrawText(
    render_commands *Commands, 
    const char *Text, 
    font *Font, 
    vec3 Position, 
    f32 Scale, 
    vec4 Color, 
    draw_text_alignment Alignment, 
    draw_text_mode Mode, 
    b32 DepthEnabled = false
)
{
    wchar WideText[256];
    ConvertToWideString(Text, WideText);

    DrawText(Commands, WideText, Font, Position, Scale, Color, Alignment, Mode, DepthEnabled);
}

inline void
DrawGround(render_commands *Commands)
{
    render_command_draw_ground *Command = PushRenderCommand(Commands, render_command_draw_ground, RenderCommand_DrawGround);
}

inline void
DrawMesh(
    render_commands *Commands, 
    u32 MeshId,
    transform Transform,
    material Material
)
{
    render_command_draw_mesh *Command = PushRenderCommand(Commands, render_command_draw_mesh, RenderCommand_DrawMesh);
    Command->MeshId = MeshId;
    Command->Transform = Transform;
    Command->Material = Material;
}

inline void
DrawSkinnedMesh(
    render_commands *Commands,
    u32 MeshId,
    transform Transform,
    material Material,
    u32 SkinningBufferId,
    u32 SkinningMatrixCount,
    mat4 *SkinningMatrices
)
{
    render_command_draw_skinned_mesh *Command = 
        PushRenderCommand(Commands, render_command_draw_skinned_mesh, RenderCommand_DrawSkinnedMesh);
    Command->MeshId = MeshId;
    Command->Transform = Transform;
    Command->Material = Material;
    Command->SkinningBufferId = SkinningBufferId;
    Command->SkinningMatrixCount = SkinningMatrixCount;
    Command->SkinningMatrices = SkinningMatrices;
}

inline void
DrawMeshInstanced(
    render_commands *Commands,
    u32 MeshId,
    u32 InstanceCount,
    render_instance *Instances,
    material Material
)
{
    render_command_draw_mesh_instanced *Command = 
        PushRenderCommand(Commands, render_command_draw_mesh_instanced, RenderCommand_DrawMeshInstanced);
    Command->MeshId = MeshId;
    Command->InstanceCount = InstanceCount;
    Command->Instances = Instances;
    Command->Material = Material;
}

inline void
DrawParticles(render_commands *Commands, u32 ParticleCount, particle *Particles)
{
    render_command_draw_particles *Command = PushRenderCommand(Commands, render_command_draw_particles, RenderCommand_DrawParticles);
    Command->ParticleCount = ParticleCount;
    Command->Particles = Particles;
}

inline void
SetDirectionalLight(render_commands *Commands, directional_light Light)
{
    render_command_set_directional_light *Command = 
        PushRenderCommand(Commands, render_command_set_directional_light, RenderCommand_SetDirectionalLight);
    Command->Light = Light;
}

inline void
SetPointLights(render_commands *Commands, u32 PointLightCount, point_light *PointLights)
{
    render_command_set_point_lights *Command =
        PushRenderCommand(Commands, render_command_set_point_lights, RenderCommand_SetPointLights);
    Command->PointLightCount = PointLightCount;
    Command->PointLights = PointLights;
}
