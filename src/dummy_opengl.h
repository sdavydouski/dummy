#pragma once

#define WIN32_RELOADABLE_SHADERS 1

#define OPENGL_MAX_MESH_BUFFER_COUNT 256
#define OPENGL_MAX_SKINNING_BUFFER_COUNT 256
#define OPENGL_MAX_TEXTURE_COUNT 64
#define OPENGL_MAX_SHADER_COUNT 64

// todo: could do better
#define OPENGL_SIMPLE_SHADER_ID 0x1
#define OPENGL_PHONG_SHADING_SHADER_ID 0x2
#define OPENGL_SKINNED_PHONG_SHADING_SHADER_ID 0x3
#define OPENGL_FRAMEBUFFER_SHADER_ID 0x4
#define OPENGL_GROUND_SHADER_ID 0x5
#define OPENGL_INSTANCED_PHONG_SHADING_SHADER_ID 0x6
#define OPENGL_TEXT_SHADER_ID 0x7

#define OPENGL_MAX_POINT_LIGHT_COUNT 32
#define OPENGL_WORLD_SPACE_MODE 0x1
#define OPENGL_SCREEN_SPACE_MODE 0x2

#define MAX_SHADER_FILE_PATH 256

const char *OpenGLCommonShaders[] = {
    "..\\src\\renderers\\OpenGL\\shaders\\common\\version.glsl",
    "..\\src\\renderers\\OpenGL\\shaders\\common\\constants.glsl",
    "..\\src\\renderers\\OpenGL\\shaders\\common\\math.glsl",
    "..\\src\\renderers\\OpenGL\\shaders\\common\\uniform.glsl",
    "..\\src\\renderers\\OpenGL\\shaders\\common\\blinn_phong.glsl",
    "..\\src\\renderers\\OpenGL\\shaders\\common\\shadows.glsl"
};

#define OPENGL_COMMON_SHADER_COUNT ArrayCount(OpenGLCommonShaders)

struct opengl_mesh_buffer
{
    u32 Id;
    u32 VertexCount;
    u32 IndexCount;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    u32 BufferSize;
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
    win32_shader_file FragmentShader;
#endif
    
    // Uniform locations
    GLint ModelUniformLocation;
    GLint ModeUniformLocation;
    GLint SkinningMatricesSamplerUniformLocation;

    GLint ColorUniformLocation;

    GLint MaterialSpecularShininessUniformLocation;
    GLint MaterialAmbientColorUniformLocation;
    GLint MaterialDiffuseColorUniformLocation;
    GLint MaterialSpecularColorUniformLocation;
    GLint MaterialDiffuseMapUniformLocation;
    GLint MaterialSpecularMapUniformLocation;
    GLint MaterialShininessMapUniformLocation;
    GLint MaterialNormalMapUniformLocation;

    GLint MaterialHasDiffuseMapUniformLocation;
    GLint MaterialHasSpecularMapUniformLocation;
    GLint MaterialHasShininessMapUniformLocation;
    GLint MaterialHasNormalMapUniformLocation;

    GLint DirectionalLightDirectionUniformLocation;
    GLint DirectionalLightColorUniformLocation;

    GLint PointLightCountUniformLocation;

    GLint ScreenTextureUniformLocation;
};

struct opengl_shader_state
{
    mat4 WorldProjection;
    mat4 ScreenProjection;
    mat4 View;
    alignas(16) vec3 CameraPosition;
    alignas(16) vec3 CameraDirection;
    f32 Time;
};

struct opengl_render_options
{
    b32 RenderShadowMap;
    b32 ShowCascades;
    mat4 CascadeView;
    mat4 CascadeProjection;
    u32 CascadeIndex;
};

struct opengl_state
{
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;

    u32 WindowWidth;
    u32 WindowHeight;

    memory_arena Arena;
    platform_api *Platform;
    platform_profiler *Profiler;

    GLuint MultiSampledFBO;
    GLuint MultiSampledColorTexture;
    GLuint MultiSampledDepthTexture;

    GLuint SingleSampledFBO;
    GLuint SingleSampledColorTexture;
    GLuint SingleSampledDepthTexture;

    GLuint LineVAO;
    GLuint RectangleVAO;

    GLuint ShaderStateUBO;

    u32 CurrentMeshBufferCount;
    opengl_mesh_buffer MeshBuffers[OPENGL_MAX_MESH_BUFFER_COUNT];

    u32 CurrentSkinningBufferCount;
    opengl_skinning_buffer SkinningBuffers[OPENGL_MAX_SKINNING_BUFFER_COUNT];

    u32 CurrentTextureCount;
    opengl_texture Textures[OPENGL_MAX_TEXTURE_COUNT];

    u32 CurrentShaderCount;
    opengl_shader Shaders[OPENGL_MAX_SHADER_COUNT];

#if 0
    hash_table<opengl_mesh_buffer> MeshBuffers;
    hash_table<opengl_skinned_mesh_buffer> SkinnedMeshBuffers;
    hash_table<opengl_texture> Textures;
    hash_table<opengl_shader> Shaders;
#endif

    u32 CascadeShadowMapSize;
    GLuint CascadeShadowMapFBO;
    GLuint CascadeShadowMaps[4];
    vec2 CascadeBounds[4];
    mat4 CascadeViewProjection[4];
};
