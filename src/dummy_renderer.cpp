#include "dummy.h"

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
    Assert(Bitmap->Pixels);

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
    render_command_set_screen_projection *Command = PushRenderCommand(Commands, render_command_set_screen_projection, RenderCommand_SetScreenProjection);
    Command->Left = Left;
    Command->Right = Right;
    Command->Bottom = Bottom;
    Command->Top = Top;
    Command->Near = Near;
    Command->Far = Far;
}

inline void
SetViewProjection(render_commands *Commands, game_camera *Camera)
{
    render_command_set_view_projection *Command = PushRenderCommand(Commands, render_command_set_view_projection, RenderCommand_SetViewProjection);
    Command->Camera = Camera;
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
    bool32 DepthEnabled = false
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
    bool32 DepthEnabled = false
)
{
    wchar WideText[256];
    ConvertToWideString(Text, WideText);

    DrawText(Commands, WideText, Font, Position, Scale, Color, Alignment, Mode, DepthEnabled);
}

inline void
DrawGrid(render_commands *Commands)
{
    render_command_draw_grid *Command = PushRenderCommand(Commands, render_command_draw_grid, RenderCommand_DrawGrid);
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
DrawMeshInstanced(
    render_commands *Commands,
    u32 MeshId,
    u32 InstanceCount,
    mesh_instance *Instances,
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
DrawSkinnedMesh(
    render_commands *Commands,
    u32 MeshId,
    material Material,
    u32 SkinningBufferId,
    u32 SkinningMatrixCount,
    mat4 *SkinningMatrices
)
{
    render_command_draw_skinned_mesh *Command = 
        PushRenderCommand(Commands, render_command_draw_skinned_mesh, RenderCommand_DrawSkinnedMesh);
    Command->MeshId = MeshId;
    Command->Material = Material;
    Command->SkinningBufferId = SkinningBufferId;
    Command->SkinningMatrixCount = SkinningMatrixCount;
    Command->SkinningMatrices = SkinningMatrices;
}

inline void
DrawSkinnedMeshInstanced(
    render_commands *Commands,
    u32 MeshId,
    material Material,
    u32 SkinningBufferId,
    u32 InstanceCount,
    skinned_mesh_instance *Instances
)
{
    render_command_draw_skinned_mesh_instanced *Command =
        PushRenderCommand(Commands, render_command_draw_skinned_mesh_instanced, RenderCommand_DrawSkinnedMeshInstanced);
    Command->MeshId = MeshId;
    Command->Material = Material;
    Command->SkinningBufferId = SkinningBufferId;
    Command->InstanceCount = InstanceCount;
    Command->Instances = Instances;
}

inline void
DrawParticles(render_commands *Commands, u32 ParticleCount, particle *Particles, texture *Texture)
{
    render_command_draw_particles *Command = PushRenderCommand(Commands, render_command_draw_particles, RenderCommand_DrawParticles);
    Command->ParticleCount = ParticleCount;
    Command->Particles = Particles;
    Command->Texture = Texture;
}

inline void
DrawTexturedQuad(render_commands *Commands, transform Transform, texture *Texture)
{
    render_command_draw_textured_quad *Command = PushRenderCommand(Commands, render_command_draw_textured_quad, RenderCommand_DrawTexturedQuad);
    Command->Transform = Transform;
    Command->Texture = Texture;
}

inline void
DrawBillboard(render_commands *Commands, vec3 Position, vec2 Size, texture *Texture)
{
    render_command_draw_billboard *Command = PushRenderCommand(Commands, render_command_draw_billboard, RenderCommand_DrawBillboard);
    Command->Position = Position;
    Command->Size = Size;
    Command->Texture = Texture;
    Command->Color = vec4(1.f, 1.f, 0.f, 1.f);
}

inline void
DrawBillboard(render_commands *Commands, vec3 Position, vec2 Size, vec4 Color)
{
    render_command_draw_billboard *Command = PushRenderCommand(Commands, render_command_draw_billboard, RenderCommand_DrawBillboard);
    Command->Position = Position;
    Command->Size = Size;
    Command->Color = Color;
    Command->Texture = 0;
}

#if 0
inline void
DrawTexturedQuadInstanced(render_commands *Commands, u32 InstanceCount, mat4 *Instances, texture *Texture)
{
    render_command_draw_textured_quad_instanced *Command = 
        PushRenderCommand(Commands, render_command_draw_textured_quad_instanced, RenderCommand_DrawTexturedQuadInstanced);
    Command->InstanceCount = InstanceCount;
    Command->Instances = Instances;
    Command->Texture = Texture;
}
#endif

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

inline void
AddSkybox(render_commands *Commands, u32 SkyboxId, u32 EnvMapSize, texture *EquirectEnvMap)
{
    Assert(EnvMapSize >= 512);
    Assert(EquirectEnvMap->Bitmap.Width == 2 * EquirectEnvMap->Bitmap.Height);

    render_command_add_skybox *Command = PushRenderCommand(Commands, render_command_add_skybox, RenderCommand_AddSkybox);
    Command->SkyboxId = SkyboxId;
    Command->EnvMapSize = EnvMapSize;
    Command->EquirectEnvMap = EquirectEnvMap;
}

inline void
SetSkybox(render_commands *Commands, u32 SkyboxId)
{
    render_command_set_skybox *Command = PushRenderCommand(Commands, render_command_set_skybox, RenderCommand_SetSkybox);
    Command->SkyboxId = SkyboxId;
}

inline void
DrawSkybox(render_commands *Commands, u32 SkyboxId)
{
    render_command_draw_skybox *Command = PushRenderCommand(Commands, render_command_draw_skybox, RenderCommand_DrawSkybox);
    Command->SkyboxId = SkyboxId;
}
