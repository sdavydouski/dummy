#pragma once

#include "dummy_defs.h"
#include "dummy_math.h"
#include "dummy_animation.h"
#include "dummy_assets.h"

#define MAX_NUMBER_OF_POINT_LIGHTS_PER_OBJECT 2

enum material_type
{
    MaterialType_BlinnPhong,
    MaterialType_Unlit
};

struct material
{
    material_type Type;
    mesh_material *MeshMaterial;

    vec3 Color;
    b32 IsWireframe;
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

struct render_instance
{
    mat4 Model;
};

enum render_command_type
{
    RenderCommand_InitRenderer,
    RenderCommand_AddMesh,
    RenderCommand_AddTexture,

    RenderCommand_SetViewport,
    RenderCommand_SetOrthographicProjection,
    RenderCommand_SetPerspectiveProjection,
    RenderCommand_SetCamera,
    RenderCommand_SetDirectionalLight,

    RenderCommand_Clear,
    
    RenderCommand_DrawLine,
    RenderCommand_DrawRectangle,
    RenderCommand_DrawGround,
    RenderCommand_DrawMesh,
    RenderCommand_DrawSkinnedMesh,
    RenderCommand_DrawMeshInstanced
};

struct render_command_header
{
    render_command_type Type;
    u32 RenderTarget;
    u32 Size;
};

struct render_command_init_renderer
{
    render_command_header Header;
};

struct render_command_add_mesh
{
    render_command_header Header;

    u32 MeshId;

    u32 VertexCount;
    skinned_vertex *Vertices;

    u32 IndexCount;
    u32 *Indices;

    u32 MaxInstanceCount;
};

struct render_command_add_texture
{
    render_command_header Header;

    u32 Id;
    bitmap *Bitmap;
    // todo: filtering, wrapping, mipmapping...
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
#if 1
    f32 Left;
    f32 Right;
    f32 Bottom;
    f32 Top;
    f32 Near;
    f32 Far;
#else
    mat4 Projection;
#endif
};

struct render_command_set_perspective_projection
{
    render_command_header Header;
#if 1
    f32 FovY;
    f32 Aspect;
    f32 Near;
    f32 Far;
#else
    mat4 Projection;
#endif
};

struct render_command_set_camera
{
    render_command_header Header;
#if 1
    vec3 Position;
    vec3 Target;
    vec3 Up;
#else
    mat4 View;
#endif
};

struct render_command_set_directional_light
{
    render_command_header Header;
    directional_light Light;
};

struct render_command_clear
{
    render_command_header Header;
    vec4 Color;
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

struct render_command_draw_ground
{
    render_command_header Header;
    // todo: put CameraPosition in the Header?
    vec3 CameraPosition;
};

struct render_command_draw_mesh
{
    render_command_header Header;

    u32 MeshId;

    transform Transform;

    material Material;

    point_light PointLight1;
    point_light PointLight2;
};

struct render_command_draw_skinned_mesh
{
    render_command_header Header;

    u32 MeshId;

    transform Transform;

    material Material;

    point_light PointLight1;
    point_light PointLight2;

    u32 SkinningMatrixCount;
    mat4 *SkinningMatrices;
};

struct render_command_draw_mesh_instanced
{
    render_command_header Header;

    u32 MeshId;

    u32 InstanceCount;
    render_instance *Instances;

    material Material;

    point_light PointLight1;
    point_light PointLight2;
};

struct render_commands
{
    u32 MaxRenderCommandsBufferSize;
    u32 RenderCommandsBufferSize;
    void *RenderCommandsBuffer;

    i32 WindowWidth;
    i32 WindowHeight;
    f32 Time;
};
