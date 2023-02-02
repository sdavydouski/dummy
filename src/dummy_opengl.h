#pragma once

#define WIN32_RELOADABLE_SHADERS 0

#define OPENGL_MAX_MESH_BUFFER_COUNT 256
#define OPENGL_MAX_SKINNING_BUFFER_COUNT 1024
#define OPENGL_MAX_TEXTURE_COUNT 64
#define OPENGL_MAX_SHADER_COUNT 64

#define OPENGL_MAX_POINT_LIGHT_COUNT 8
#define OPENGL_WORLD_SPACE_MODE 0x1
#define OPENGL_SCREEN_SPACE_MODE 0x2
#define OPENGL_MAX_JOINT_COUNT 256
#define OPENGL_UNIFORM_MAX_LENGTH 64
#define OPENGL_UNIFORM_MAX_COUNT 257

#define MAX_SHADER_FILE_PATH 256

// todo: could do better
#define OPENGL_COLOR_SHADER_ID 0x1
#define OPENGL_PHONG_SHADER_ID 0x2
#define OPENGL_PHONG_INSTANCED_SHADER_ID 0x3
#define OPENGL_PHONG_SKINNED_SHADER_ID 0x4
#define OPENGL_PHONG_SKINNED_INSTANCED_SHADER_ID 0x5
#define OPENGL_FRAMEBUFFER_SHADER_ID 0x6
#define OPENGL_GRID_SHADER_ID 0x7
#define OPENGL_TEXT_SHADER_ID 0x8
#define OPENGL_PARTICLE_SHADER_ID 0x9
#define OPENGL_TEXTURED_QUAD_SHADER_ID 0x10
#define OPENGL_BILLBOARD_SHADER_ID 0x11
#define OPENGL_SKYBOX_SHADER_ID 0x12
#define OPENGL_EQUIRECT_TO_CUBEMAP_SHADER_ID 0x13

const char *OpenGLCommonShaders[] = 
{
    "shaders\\glsl\\common\\version.glsl",
    "shaders\\glsl\\common\\constants.glsl",
    "shaders\\glsl\\common\\math.glsl",
    "shaders\\glsl\\common\\phong.glsl",
    "shaders\\glsl\\common\\uniform.glsl",
    "shaders\\glsl\\common\\shadows.glsl"
};

#define OPENGL_COMMON_SHADER_COUNT ArrayCount(OpenGLCommonShaders)

struct opengl_load_shader_params
{
    u32 ShaderId;

    const char *VertexShaderFileName;
    const char *GeometryShaderFileName;
    const char *FragmentShaderFileName;
    const char *ComputeShaderFileName;
};

opengl_load_shader_params OpenGLShaders[] =
{
    {
        .ShaderId = OPENGL_COLOR_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\simple.vert",
        .FragmentShaderFileName = "shaders\\glsl\\color.frag"
    },
    {
        .ShaderId = OPENGL_PHONG_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\mesh.vert",
        .FragmentShaderFileName = "shaders\\glsl\\phong.frag"
    },
    {
        .ShaderId = OPENGL_PHONG_INSTANCED_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\mesh_instanced.vert",
        .FragmentShaderFileName = "shaders\\glsl\\phong.frag"
    },
    {
        .ShaderId = OPENGL_PHONG_SKINNED_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\skinned_mesh.vert",
        .FragmentShaderFileName = "shaders\\glsl\\phong.frag"
    },
    {
        .ShaderId = OPENGL_PHONG_SKINNED_INSTANCED_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\skinned_mesh_instanced.vert",
        .FragmentShaderFileName = "shaders\\glsl\\phong.frag"
    },
    {
        .ShaderId = OPENGL_GRID_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\grid.vert",
        .FragmentShaderFileName = "shaders\\glsl\\grid.frag"
    },
    {
        .ShaderId = OPENGL_FRAMEBUFFER_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\framebuffer.vert",
        .FragmentShaderFileName = "shaders\\glsl\\framebuffer.frag"
    },
    {
        .ShaderId = OPENGL_TEXT_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\text.vert",
        .GeometryShaderFileName = "shaders\\glsl\\text.geom",
        .FragmentShaderFileName = "shaders\\glsl\\text.frag"
    },
    {
        .ShaderId = OPENGL_PARTICLE_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\particle.vert",
        .GeometryShaderFileName = "shaders\\glsl\\particle.geom",
        .FragmentShaderFileName = "shaders\\glsl\\particle.frag"
    },
    {
        .ShaderId = OPENGL_TEXTURED_QUAD_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\textured_quad.vert",
        .FragmentShaderFileName = "shaders\\glsl\\textured_quad.frag"
    },
    {
        .ShaderId = OPENGL_BILLBOARD_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\billboard.vert",
        .GeometryShaderFileName = "shaders\\glsl\\billboard.geom",
        .FragmentShaderFileName = "shaders\\glsl\\billboard.frag"
    },
    {
        .ShaderId = OPENGL_EQUIRECT_TO_CUBEMAP_SHADER_ID,
        .ComputeShaderFileName = "shaders\\glsl\\equirect2cube.comp"
    },
    {
        .ShaderId = OPENGL_SKYBOX_SHADER_ID,
        .VertexShaderFileName = "shaders\\glsl\\skybox.vert",
        .FragmentShaderFileName = "shaders\\glsl\\skybox.frag"
    }
};

struct opengl_uniform
{
    char Key[OPENGL_UNIFORM_MAX_LENGTH];
    GLint Location;
    GLint Size;
};

#if WIN32_RELOADABLE_SHADERS
struct win32_shader_file
{
    char FileName[MAX_SHADER_FILE_PATH];
    FILETIME LastWriteTime;
};
#endif

struct opengl_shader
{
    u32 Id;
    GLuint Program;

#if WIN32_RELOADABLE_SHADERS
    win32_shader_file CommonShaders[OPENGL_COMMON_SHADER_COUNT];
    win32_shader_file VertexShader;
    win32_shader_file GeometryShader;
    win32_shader_file FragmentShader;
#endif

    u32 UniformCount;
    hash_table<opengl_uniform> Uniforms;
};

struct opengl_mesh_buffer
{
    u32 Id;
    u32 VertexCount;
    u32 IndexCount;

    GLuint VAO;
    GLuint VertexBuffer;
    GLuint InstanceBuffer;
    GLuint IndexBuffer;

    u32 BufferSize;
    u32 InstanceCount;
};

struct opengl_skinning_buffer
{
    u32 Id;
    GLuint SkinningTBO;
    GLuint SkinningTBOTexture;
};

struct opengl_texture
{
    u32 Id;
    GLuint Handle;
};

struct opengl_framebuffer
{
    GLuint Handle;
    GLuint ColorTarget;
    GLuint DepthStencilTarget;

    u32 Width;
    u32 Height;
    u32 Samples;
};

struct opengl_character_point
{
    vec3 Position;
    vec2 Size;
    vec2 SpriteSize;
    vec2 SpriteOffset;
};

struct opengl_particle
{
    vec3 Position;
    vec2 Size;
    vec4 Color;
};

struct opengl_uniform_buffer_transform
{
    mat4 WorldProjection;
    mat4 ScreenProjection;
    mat4 View;

    alignas(16) vec3 CameraPosition;
    alignas(16) vec3 CameraDirection;
    
    f32 Time;
};

// todo:
struct opengl_directional_light
{
    alignas(16) vec3 LightDirection;
    alignas(16) vec3 LightColor;
};

struct opengl_point_light
{
    alignas(16) vec3 Position;
    alignas(16) vec3 Color;
    alignas(16) vec3 Attenuation;
};

struct opengl_uniform_buffer_shading
{
    opengl_directional_light DirectinalLight;
    u32 PointLightCount;
    opengl_point_light PointLights[OPENGL_MAX_POINT_LIGHT_COUNT];
};

struct opengl_render_options
{
    bool32 RenderShadowMap;
    bool32 ShowCascades;
    mat4 CascadeView;
    mat4 CascadeProjection;
    u32 CascadeIndex;
    bool32 EnableLog;
};

struct opengl_state
{
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;

    u32 WindowWidth;
    u32 WindowHeight;

    stream DebugStream;
    stream RendererStream;
    memory_arena Arena;
    platform_api *Platform;
    platform_profiler *Profiler;

    opengl_framebuffer SourceFramebuffer;
    opengl_framebuffer DestFramebuffer;

    GLuint LineVAO;
    GLuint RectangleVAO;
    GLuint BoxVAO;
    GLuint TextVAO;
    GLuint TextVBO;
    GLuint ParticleVAO;
    GLuint ParticleVBO;

    GLuint TransformUBO;
    GLuint ShadingUBO;

    u32 CurrentMeshBufferCount;
    opengl_mesh_buffer MeshBuffers[OPENGL_MAX_MESH_BUFFER_COUNT];

    u32 CurrentSkinningBufferCount;
    opengl_skinning_buffer SkinningBuffers[OPENGL_MAX_SKINNING_BUFFER_COUNT];

    u32 CurrentTextureCount;
    opengl_texture Textures[OPENGL_MAX_TEXTURE_COUNT];

    u32 CurrentShaderCount;
    opengl_shader Shaders[OPENGL_MAX_SHADER_COUNT];

    u32 CascadeShadowMapSize;
    GLuint CascadeShadowMapFBO;
    GLuint CascadeShadowMaps[4];
    vec2 CascadeBounds[4];
    mat4 CascadeViewProjection[4];
};
