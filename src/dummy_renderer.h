#pragma once

enum material_type
{
    MaterialType_Basic,
    MaterialType_Phong
};

struct material
{
    material_type Type;

    union
    {
        vec4 Color;
        mesh_material *MeshMaterial;
    };

    b32 Wireframe;
    b32 CastShadow;
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

struct game_camera
{
    transform Transform;
    vec3 Direction;
    vec3 Up;

    f32 Pitch;
    f32 Yaw;

    // Vertical field of view
    f32 FieldOfView;
    f32 FocalLength;
    f32 AspectRatio;
    f32 NearClipPlane;
    f32 FarClipPlane;

    // For orbiting camera
    vec3 PivotPosition;
    f32 Radius;
    vec3_lerp PivotPositionLerp;
    //
};

inline mat4
GetCameraTransform(game_camera *Camera)
{
    mat4 Result = LookAt(Camera->Transform.Translation, Camera->Transform.Translation + Camera->Direction, Camera->Up);
    return Result;
}

struct render_instance
{
    mat4 Model;
    vec3 Color;
};

struct skinning_data
{
    skeleton_pose *Pose;
    u32 SkinningBufferId;
    u32 SkinningMatrixCount;
    mat4 *SkinningMatrices;
};

enum render_command_type
{
    RenderCommand_AddMesh,
    RenderCommand_AddTexture,
    RenderCommand_AddSkinningBuffer,

    RenderCommand_SetViewport,
    RenderCommand_SetScreenProjection,
    RenderCommand_SetWorldProjection,
    RenderCommand_SetCamera,
    RenderCommand_SetTime,
    RenderCommand_SetDirectionalLight,
    RenderCommand_SetPointLights,

    RenderCommand_Clear,
    RenderCommand_DrawLine,
    RenderCommand_DrawRectangle,
    RenderCommand_DrawBox,
    RenderCommand_DrawText,
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
    i32 *JointIndices;

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

struct render_command_set_world_projection
{
    render_command_header Header;
    f32 FovY;
    f32 Aspect;
    f32 Near;
    f32 Far;
};

struct render_command_set_camera
{
    render_command_header Header;
    vec3 Position;
    vec3 Direction;
    vec3 Up;
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

struct render_command_draw_text
{
    render_command_header Header;

    wchar Text[256];
    font *Font;
    vec3 Position;
    f32 Scale;
    vec4 Color;
    draw_text_mode Mode;
    b32 DepthEnabled;
};

struct render_command_draw_ground
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

struct render_command_draw_skinned_mesh
{
    render_command_header Header;

    u32 MeshId;

    transform Transform;
    material Material;

    u32 SkinningBufferId;
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
};

struct render_commands_settings
{
    i32 WindowWidth;
    i32 WindowHeight;

    i32 PrevWindowWidth;
    i32 PrevWindowHeight;

    f32 Time;
    f32 PixelsPerUnit;
    f32 UnitsPerPixel;
    b32 ShowCascades;
    game_camera *Camera;
    directional_light *DirectionalLight;
};

struct render_commands
{
    u32 MaxRenderCommandsBufferSize;
    u32 RenderCommandsBufferSize;
    void *RenderCommandsBuffer;

    render_commands_settings Settings;
};
