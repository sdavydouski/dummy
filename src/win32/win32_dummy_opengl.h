#pragma once

#include <glad.h>
#include <wglext.h>

#define OPENGL_RELOADABLE_SHADERS 0
#define OPENGL_MAX_SHADER_FILE_PATH 256

#define OPENGL_MAX_POINT_LIGHT_COUNT 16

#define OPENGL_WORLD_SPACE_MODE 0x1
#define OPENGL_SCREEN_SPACE_MODE 0x2
#define OPENGL_MAX_JOINT_COUNT 256
#define OPENGL_MAX_WEIGHT_COUNT 4
#define OPENGL_UNIFORM_MAX_LENGTH 64
#define OPENGL_UNIFORM_MAX_COUNT 509

const char *OpenGLCommonShaders[] =
{
    "shaders\\glsl\\common\\version.glsl",
    "shaders\\glsl\\common\\constants.glsl",
    "shaders\\glsl\\common\\math.glsl",
    "shaders\\glsl\\common\\lights.glsl",
    "shaders\\glsl\\common\\uniform.glsl",
    "shaders\\glsl\\common\\shadows.glsl"
};

#define OPENGL_COMMON_SHADER_COUNT ArrayCount(OpenGLCommonShaders)

struct opengl_load_shader_params
{
    char ShaderKey[256];

    const char *VertexShaderFileName;
    const char *GeometryShaderFileName;
    const char *FragmentShaderFileName;
    const char *ComputeShaderFileName;
};

opengl_load_shader_params OpenGLShaders[] =
{
    {
        .ShaderKey = "simple.color",
        .VertexShaderFileName = "shaders\\glsl\\simple.vert",
        .FragmentShaderFileName = "shaders\\glsl\\color.frag"
    },
    {
        .ShaderKey = "mesh.phong",
        .VertexShaderFileName = "shaders\\glsl\\mesh.vert",
        .FragmentShaderFileName = "shaders\\glsl\\phong.frag"
    },
    {
        .ShaderKey = "mesh.pbr",
        .VertexShaderFileName = "shaders\\glsl\\mesh.vert",
        .FragmentShaderFileName = "shaders\\glsl\\pbr.frag"
    },
    {
        .ShaderKey = "mesh_instanced.phong",
        .VertexShaderFileName = "shaders\\glsl\\mesh_instanced.vert",
        .FragmentShaderFileName = "shaders\\glsl\\phong.frag"
    },
    {
        .ShaderKey = "mesh_instanced.pbr",
        .VertexShaderFileName = "shaders\\glsl\\mesh_instanced.vert",
        .FragmentShaderFileName = "shaders\\glsl\\pbr.frag"
    },
    {
        .ShaderKey = "skinned_mesh.phong",
        .VertexShaderFileName = "shaders\\glsl\\skinned_mesh.vert",
        .FragmentShaderFileName = "shaders\\glsl\\phong.frag"
    },
    {
        .ShaderKey = "skinned_mesh.pbr",
        .VertexShaderFileName = "shaders\\glsl\\skinned_mesh.vert",
        .FragmentShaderFileName = "shaders\\glsl\\pbr.frag"
    },
    {
        .ShaderKey = "skinned_mesh_instanced.phong",
        .VertexShaderFileName = "shaders\\glsl\\skinned_mesh_instanced.vert",
        .FragmentShaderFileName = "shaders\\glsl\\phong.frag"
    },
     {
        .ShaderKey = "skinned_mesh_instanced.pbr",
        .VertexShaderFileName = "shaders\\glsl\\skinned_mesh_instanced.vert",
        .FragmentShaderFileName = "shaders\\glsl\\pbr.frag"
    },
    {
        .ShaderKey = "grid",
        .VertexShaderFileName = "shaders\\glsl\\grid.vert",
        .FragmentShaderFileName = "shaders\\glsl\\grid.frag"
    },
    {
        .ShaderKey = "framebuffer",
        .VertexShaderFileName = "shaders\\glsl\\framebuffer.vert",
        .FragmentShaderFileName = "shaders\\glsl\\framebuffer.frag"
    },
    {
        .ShaderKey = "text",
        .VertexShaderFileName = "shaders\\glsl\\text.vert",
        .GeometryShaderFileName = "shaders\\glsl\\text.geom",
        .FragmentShaderFileName = "shaders\\glsl\\text.frag"
    },
    {
        .ShaderKey = "particle",
        .VertexShaderFileName = "shaders\\glsl\\particle.vert",
        .GeometryShaderFileName = "shaders\\glsl\\particle.geom",
        .FragmentShaderFileName = "shaders\\glsl\\particle.frag"
    },
    {
        .ShaderKey = "textured_quad",
        .VertexShaderFileName = "shaders\\glsl\\textured_quad.vert",
        .FragmentShaderFileName = "shaders\\glsl\\textured_quad.frag"
    },
    {
        .ShaderKey = "billboard",
        .VertexShaderFileName = "shaders\\glsl\\billboard.vert",
        .GeometryShaderFileName = "shaders\\glsl\\billboard.geom",
        .FragmentShaderFileName = "shaders\\glsl\\billboard.frag"
    },
    {
        .ShaderKey = "equirect2cube",
        .ComputeShaderFileName = "shaders\\glsl\\equirect2cube.comp"
    },
    {
        .ShaderKey = "specular_map",
        .ComputeShaderFileName = "shaders\\glsl\\specular_map.comp"
    },
    {
        .ShaderKey = "irradiance_map",
        .ComputeShaderFileName = "shaders\\glsl\\irradiance_map.comp"
    },
    {
        .ShaderKey = "specular_brdf",
        .ComputeShaderFileName = "shaders\\glsl\\specular_brdf.comp"
    },
    {
        .ShaderKey = "skybox",
        .VertexShaderFileName = "shaders\\glsl\\skybox.vert",
        .FragmentShaderFileName = "shaders\\glsl\\skybox.frag"
    },
    {
        .ShaderKey = "skinned_mesh",
        .ComputeShaderFileName = "shaders\\glsl\\skinned_mesh.comp"
    },
    {
        .ShaderKey = "skinned_mesh_instanced",
        .ComputeShaderFileName = "shaders\\glsl\\skinned_mesh_instanced.comp"
    }
};

struct opengl_uniform
{
    char Key[OPENGL_UNIFORM_MAX_LENGTH];
    GLint Location;
    GLint Size;
};

struct win32_shader_file
{
    char FileName[OPENGL_MAX_SHADER_FILE_PATH];
    FILETIME LastWriteTime;
};

struct opengl_shader
{
    char Key[256];
    GLuint Program;
    hash_table<opengl_uniform> Uniforms;

    win32_shader_file CommonShaders[OPENGL_COMMON_SHADER_COUNT];
    win32_shader_file VertexShader;
    win32_shader_file GeometryShader;
    win32_shader_file ComputeShader;
    win32_shader_file FragmentShader;
};

struct opengl_buffer
{
    GLuint VAO;
    GLuint VBO;
};

struct opengl_mesh_buffer
{
    u32 Key;

    u32 VertexCount;
    u32 IndexCount;

    GLuint VAO;
    GLuint VertexBuffer;
    GLuint InstanceBuffer;
    GLuint IndexBuffer;

    GLuint PositionsBuffer;
    GLuint WeightsBuffer;
    GLuint JointIndicesBuffer;
    GLuint SkinningMatricesBuffer;

    u32 BufferSize;
    u32 InstanceCount;
};

struct opengl_skinning_buffer
{
    u32 Key;

    GLuint SkinningTBO;
    GLuint SkinningTBOTexture;
};

struct opengl_texture
{
    u32 Key;

    u32 Width;
    u32 Height;

    GLuint Handle;
};

struct opengl_skybox
{
    u32 Key;

    GLuint EnvTexture;
    GLuint SpecularEnvTexture;
    GLuint IrradianceTexture;
    GLuint SpecularBRDF;
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
    mat4 ViewProjection;
    mat4 ScreenProjection;
    mat4 SkyProjection;

    alignas(16) vec3 CameraPosition;
    alignas(16) vec3 CameraDirection;

    f32 Time;
};

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
    bool32 EnableShadows;
    bool32 WireframeMode;

    mat4 CascadeView;
    mat4 CascadeProjection;
    u32 CascadeIndex;
};

struct opengl_state
{
    stream *Stream;
    memory_arena *Arena;
    platform_api *Platform;
    platform_profiler *Profiler;

    HDC WindowDC;
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
    PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;

    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;

    u32 WindowWidth;
    u32 WindowHeight;

    opengl_framebuffer SourceFramebuffer;
    opengl_framebuffer DestFramebuffer;
    opengl_framebuffer EditorFramebuffer;

    opengl_buffer Line;
    opengl_buffer Rectangle;
    opengl_buffer Box;
    opengl_buffer Text;
    opengl_buffer Particle;

    GLuint TransformUBO;
    GLuint ShadingUBO;

    hash_table<opengl_mesh_buffer> MeshBuffers;
    hash_table<opengl_skinning_buffer> SkinningBuffers;
    hash_table<opengl_texture> Textures;
    hash_table<opengl_shader> Shaders;
    hash_table<opengl_skybox> Skyboxes;

    u32 CurrentSkyboxId;

    u32 CascadeShadowMapSize;
    GLuint CascadeShadowMapFBO;
    GLuint CascadeShadowMaps[4];
    vec2 CascadeBounds[4];
    mat4 CascadeViewProjection[4];
};
