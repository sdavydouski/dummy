#pragma once

enum material_type
{
    // A material for drawing geometries in a simple shaded way
    MaterialType_Basic,
    // A material for shiny surfaces with specular highlights
    MaterialType_Phong,
    // A standard physically based material, using Metallic-Roughness workflow
    MaterialType_Standard
};

struct material_options
{
    bool32 Wireframe;
    bool32 CastShadow;
};

struct material
{
    material_type Type;

    union
    {
        vec4 Color;
        mesh_material *MeshMaterial;
    };

    material_options Options;
};

struct light_attenuation
{
    f32 Constant;
    f32 Linear;
    f32 Quadratic;
};

struct directional_light
{
    vec3 Direction;
    vec3 Color;
};

struct point_light
{
    vec3 Position;
    vec3 Color;
    light_attenuation Attenuation;
};

struct spot_light
{
    vec3 Position;
    vec3 Color;
    vec3 Direction;

    f32 InnerCutOffAngle;
    f32 OuterCutOffAngle;

    light_attenuation Attenuation;
};

struct particle
{
    vec3 Position;
    vec3 Velocity;
    vec3 Acceleration;
    vec4 Color;
    vec4 dColor;
    vec2 Size;
    vec2 dSize;
    f32 CameraDistanceSquared;
};

struct particle_emitter
{
    u32 NextParticleIndex;

    u32 ParticleCount;
    particle *Particles;

    u32 ParticlesSpawn;
    vec4 Color;
    vec2 Size;
};

struct skinning_data
{
    skeleton_pose *BindPose;
    skeleton_pose *Pose;
    u32 SkinningMatrixCount;
    mat4 *SkinningMatrices;
};

enum render_command_type
{
    RenderCommand_AddMesh,
    RenderCommand_AddTexture,
    RenderCommand_AddSkinningBuffer,
    RenderCommand_AddSkybox,

    RenderCommand_SetViewport,
    RenderCommand_SetScreenProjection,
    RenderCommand_SetViewProjection,
    RenderCommand_SetTime,
    RenderCommand_SetDirectionalLight,
    RenderCommand_SetPointLights,

    RenderCommand_Clear,
    RenderCommand_DrawPoint,
    RenderCommand_DrawLine,
    RenderCommand_DrawRectangle,
    RenderCommand_DrawBox,
    RenderCommand_DrawText,
    RenderCommand_DrawGrid,
    RenderCommand_DrawMesh,
    RenderCommand_DrawMeshInstanced,
    RenderCommand_DrawSkinnedMesh,
    RenderCommand_DrawSkinnedMeshInstanced,
    RenderCommand_DrawParticles,
    RenderCommand_DrawTexturedQuad,
    RenderCommand_DrawBillboard,
    RenderCommand_DrawSkybox,

    RenderCommand_Count
};

const char *RenderCommandNames[] =
{
    "AddMesh",
    "AddTexture",
    "AddSkinningBuffer",
    "AddSkybox",

    "SetViewport",
    "SetScreenProjection",
    "SetViewProjection",
    "SetTime",
    "SetDirectionalLight",
    "SetPointLights",

    "Clear",
    "DrawPoint",
    "DrawLine",
    "DrawRectangle",
    "DrawBox",
    "DrawText",
    "DrawGrid",
    "DrawMesh",
    "DrawMeshInstanced",
    "DrawSkinnedMesh",
    "DrawSkinnedMeshInstanced",
    "DrawParticles",
    "DrawTexturedQuad",
    "DrawBillboard",
    "DrawSkybox"
};

CTAssert(ArrayCount(RenderCommandNames) == RenderCommand_Count);

struct render_command_header
{
    render_command_type Type;
    u32 Size;
};

struct render_command_add_mesh
{
    render_command_header Header;

    u32 MeshId;

    u32 VertexCount;

    vec3 *Positions;
    vec3 *Normals;
    vec3 *Tangents;
    vec3 *Bitangents;
    vec2 *TextureCoords;
    vec4 *Weights;
    ivec4 *JointIndices;

    u32 IndexCount;
    u32 *Indices;
};

struct render_command_add_texture
{
    render_command_header Header;

    u32 Id;
    bitmap *Bitmap;
    // todo: filtering, wrapping, mipmapping...
};

struct render_command_add_skinning_buffer
{
    render_command_header Header;

    u32 SkinningBufferId;
    u32 SkinningMatrixCount;
};

struct render_command_set_viewport
{
    render_command_header Header;
    u32 x;
    u32 y;
    u32 Width;
    u32 Height;
};

struct render_command_set_screen_projection
{
    render_command_header Header;
    f32 Left;
    f32 Right;
    f32 Bottom;
    f32 Top;
    f32 Near;
    f32 Far;
};

struct render_command_set_view_projection
{
    render_command_header Header;
    game_camera *Camera;
};

struct render_command_set_directional_light
{
    render_command_header Header;
    directional_light Light;
    mat4 LightProjection;
    mat4 LightView;
};

struct render_command_set_point_lights
{
    render_command_header Header;
    u32 PointLightCount;
    point_light *PointLights;
};

struct render_command_set_time
{
    render_command_header Header;
    f32 Time;
};

struct render_command_clear
{
    render_command_header Header;
    vec4 Color;
};

struct render_command_draw_point
{
    render_command_header Header;
    vec3 Position;
    vec4 Color;
    f32 Size;
};

struct render_command_draw_line
{
    render_command_header Header;
    vec3 Start;
    vec3 End;
    vec4 Color;
    f32 Thickness;
};

struct render_command_draw_rectangle
{
    render_command_header Header;
    transform Transform;
    vec4 Color;
};

struct render_command_draw_box
{
    render_command_header Header;
    transform Transform;
    vec4 Color;
};

enum draw_text_mode
{
    DrawText_WorldSpace,
    DrawText_ScreenSpace
};

enum draw_text_alignment
{
    DrawText_AlignLeft,
    DrawText_AlignCenter,
    DrawText_AlignRight
};

struct render_command_draw_text
{
    render_command_header Header;

    wchar Text[256];
    font *Font;
    vec3 Position;
    f32 Scale;
    vec4 Color;
    draw_text_alignment Alignment;
    draw_text_mode Mode;
    bool32 DepthEnabled;
};

struct render_command_draw_grid
{
    render_command_header Header;
};

struct render_command_draw_mesh
{
    render_command_header Header;

    u32 MeshId;

    transform Transform;
    material Material;
};

struct mesh_instance
{
    mat4 Model;
    vec3 Color;
};

struct render_command_draw_mesh_instanced
{
    render_command_header Header;

    u32 MeshId;
    material Material;

    u32 InstanceCount;
    mesh_instance *Instances;
};

struct render_command_draw_skinned_mesh
{
    render_command_header Header;

    u32 MeshId;
    material Material;

    u32 SkinningBufferId;
    u32 SkinningMatrixCount;
    mat4 *SkinningMatrices;
};

struct skinned_mesh_instance
{
    u32 SkinningMatrixCount;
    mat4 *SkinningMatrices;
};

struct render_command_draw_skinned_mesh_instanced
{
    render_command_header Header;

    u32 MeshId;
    material Material;
    u32 SkinningBufferId;
    u32 InstanceCount;
    skinned_mesh_instance *Instances;
};

struct render_command_draw_particles
{
    render_command_header Header;

    u32 ParticleCount;
    particle *Particles;

    texture *Texture;
};

struct render_command_draw_textured_quad
{
    render_command_header Header;
    transform Transform;
    texture *Texture;
};

struct render_command_draw_billboard
{
    render_command_header Header;
    vec3 Position;
    vec2 Size;

    vec4 Color;
    texture *Texture;
};

struct render_command_add_skybox
{
    render_command_header Header;
    u32 SkyboxId;
    u32 EnvMapSize;
    texture *EquirectEnvMap;
};

struct render_command_draw_skybox
{
    render_command_header Header;
    u32 SkyboxId;
};

struct render_commands_settings
{
    i32 WindowWidth;
    i32 WindowHeight;

    i32 PrevWindowWidth;
    i32 PrevWindowHeight;

    u32 Samples;

    f32 Time;
    f32 PixelsPerUnit;
    f32 UnitsPerPixel;
    
    bool32 ShowCascades;
    bool32 EnableShadows;
    bool32 WireframeMode;
    
    game_camera *Camera;
    
    mat4 WorldToCamera;
    mat4 CameraToWorld;

    directional_light *DirectionalLight;
};

struct render_commands
{
    u32 MaxRenderCommandsBufferSize;
    u32 RenderCommandsBufferSize;
    void *RenderCommandsBuffer;

    render_commands_settings Settings;
};
