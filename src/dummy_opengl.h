#pragma once

#define OPENGL_MAX_MESH_BUFFER_COUNT 64
#define OPENGL_MAX_TEXTURE_COUNT 64
#define OPENGL_MAX_SHADER_COUNT 64

#define OPENGL_SIMPLE_SHADER_ID 0
#define OPENGL_GRID_SHADER_ID 1
#define OPENGL_PHONG_SHADING_SHADER_ID 2
#define OPENGL_SKINNED_PHONG_SHADING_SHADER_ID 3

struct opengl_mesh_buffer
{
    u32 Id;
    u32 IndexCount;
    primitive_type PrimitiveType;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

struct opengl_texture
{
    u32 Id;
    GLuint Handle;
};

struct opengl_shader
{
    u32 Id;
    GLuint Program;

    GLint ModelUniformLocation;
    GLint ViewUniformLocation;
    GLint ProjectionUniformLocation;
    GLint SkinningMatricesSamplerUniformLocation;

    GLint ColorUniformLocation;
    GLint CameraPositionUniformLocation;

    GLint MaterialSpecularShininessUniformLocation;
    GLint MaterialAmbientColorUniformLocation;
    GLint MaterialDiffuseColorUniformLocation;
    GLint MaterialSpecularColorUniformLocation;
    GLint MaterialDiffuseMapUniformLocation;
    GLint MaterialSpecularMapUniformLocation;
    GLint MaterialShininessMapUniformLocation;
    GLint MaterialNormalMapUniformLocation;

    GLint DirectionalLightDirectionUniformLocation;
    GLint DirectionalLightColorUniformLocation;

    GLint PointLight1PositionUniformLocation;
    GLint PointLight1ColorUniformLocation;
    GLint PointLight1AttenuationConstantUniformLocation;
    GLint PointLight1AttenuationLinearUniformLocation;
    GLint PointLight1AttenuationQuadraticUniformLocation;

    GLint PointLight2PositionUniformLocation;
    GLint PointLight2ColorUniformLocation;
    GLint PointLight2AttenuationConstantUniformLocation;
    GLint PointLight2AttenuationLinearUniformLocation;
    GLint PointLight2AttenuationQuadraticUniformLocation;
};

struct opengl_state
{
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;

    memory_arena Arena;

    GLuint LineVAO;
    GLuint RectangleVAO;
    GLuint GridVAO;

    GLuint SkinningTBO;
    GLuint SkinningTBOTexture;

    u32 CurrentMeshBufferCount;
    opengl_mesh_buffer MeshBuffers[OPENGL_MAX_MESH_BUFFER_COUNT];

    u32 CurrentTextureCount;
    opengl_texture Textures[OPENGL_MAX_TEXTURE_COUNT];

    u32 CurrentShaderCount;
    opengl_shader Shaders[OPENGL_MAX_SHADER_COUNT];
};
