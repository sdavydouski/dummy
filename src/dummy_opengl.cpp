#include "dummy_opengl_shaders.h"

internal GLuint
CreateShader(GLenum Type, char *Source)
{
    GLuint Shader = glCreateShader(Type);
    glShaderSource(Shader, 1, &Source, NULL);
    glCompileShader(Shader);

    i32 IsShaderCompiled;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &IsShaderCompiled);
    if (!IsShaderCompiled)
    {
        i32 LogLength;
        glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLength);

        char ErrorLog[256];
        glGetShaderInfoLog(Shader, LogLength, NULL, ErrorLog);

        glDeleteShader(Shader);

        Assert(!ErrorLog);
    }

    return Shader;
}

internal GLuint
CreateProgram(GLuint VertexShader, GLuint FragmentShader)
{
    GLuint Program = glCreateProgram();
    glAttachShader(Program, VertexShader);
    glAttachShader(Program, FragmentShader);
    glLinkProgram(Program);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    i32 IsProgramLinked;
    glGetProgramiv(Program, GL_LINK_STATUS, &IsProgramLinked);

    if (!IsProgramLinked)
    {
        i32 LogLength;
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogLength);

        char ErrorLog[256];
        glGetProgramInfoLog(Program, LogLength, NULL, ErrorLog);

        Assert(!ErrorLog);
    }

    return Program;
}

internal void
InitLine(opengl_state *State)
{
    vec3 LineVertices[] = {
        vec3(0.f, 0.f, 0.f),
        vec3(1.f, 1.f, 1.f),
    };

    glGenVertexArrays(1, &State->LineVAO);
    glBindVertexArray(State->LineVAO);

    GLuint LineVBO;
    glGenBuffers(1, &LineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, LineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertices), LineVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);

    glBindVertexArray(0);
}

internal void
InitRectangle(opengl_state *State)
{
    f32 RectangleVertices[] = {
        // potisions     // uvs
        -1.f, -1.f, 0.f, 0.f, 0.f,
        1.f, -1.f, 0.f,  1.f, 0.f,
        -1.f, 1.f, 0.f,  0.f, 1.f,
        1.f, 1.f, 0.f,   1.f, 1.f
    };

    GLsizei Stride = sizeof(vec3) + sizeof(vec2);

    glGenVertexArrays(1, &State->RectangleVAO);
    glBindVertexArray(State->RectangleVAO);

    GLuint RectangleVBO;
    glGenBuffers(1, &RectangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, RectangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RectangleVertices), RectangleVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Stride, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, Stride, (void *) sizeof(vec3));

    glBindVertexArray(0);
}

// todo: use hashtable
inline opengl_mesh_buffer *
OpenGLGetMeshBuffer(opengl_state *State, u32 Id)
{
    opengl_mesh_buffer *Result = 0;

    for (u32 MeshBufferIndex = 0; MeshBufferIndex < State->CurrentMeshBufferCount; ++MeshBufferIndex)
    {
        opengl_mesh_buffer *MeshBuffer = State->MeshBuffers + MeshBufferIndex;

        if (MeshBuffer->Id == Id)
        {
            Result = MeshBuffer;
            break;
        }
    }

    Assert(Result);

    return Result;
}

// todo: use hashtable
inline opengl_texture *
OpenGLGetTexture(opengl_state *State, u32 Id)
{
    opengl_texture *Result = 0;

    for (u32 TextureIndex = 0; TextureIndex < State->CurrentTextureCount; ++TextureIndex)
    {
        opengl_texture *Texture = State->Textures + TextureIndex;

        if (Texture->Id == Id)
        {
            Result = Texture;
            break;
        }
    }

    Assert(Result);

    return Result;
}

// todo: use hashtable
inline opengl_shader *
OpenGLGetShader(opengl_state *State, u32 Id)
{
    opengl_shader *Result = State->Shaders + Id;

    for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
    {
        opengl_shader *Shader = State->Shaders + ShaderIndex;

        if (Shader->Id == Id)
        {
            Result = Shader;
            break;
        }
    }

    Assert(Result);

    return Result;
}

internal void
OpenGLAddMeshBuffer(opengl_state *State, u32 MeshId, u32 VertexCount, skinned_vertex *Vertices, u32 IndexCount, u32 *Indices)
{
    Assert(State->CurrentMeshBufferCount < OPENGL_MAX_MESH_BUFFER_COUNT);

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, VertexCount * sizeof(skinned_vertex), Vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Tangent));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Bitangent));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, TextureCoords));

    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_UNSIGNED_INT, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, JointIndices));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Weights));

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexCount * sizeof(u32), Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    opengl_mesh_buffer *MeshBuffer = State->MeshBuffers + State->CurrentMeshBufferCount++;
    MeshBuffer->Id = MeshId;
    MeshBuffer->VertexCount = VertexCount;
    MeshBuffer->IndexCount = IndexCount;
    MeshBuffer->VAO = VAO;
    MeshBuffer->VBO = VBO;
    MeshBuffer->EBO = EBO;
}

internal void
OpenGLAddMeshBufferInstanced(
    opengl_state *State, 
    u32 MeshId, 
    u32 VertexCount, 
    skinned_vertex *Vertices, 
    u32 IndexCount, 
    u32 *Indices, 
    u32 MaxInstanceCount
)
{
    Assert(State->CurrentMeshBufferCount < OPENGL_MAX_MESH_BUFFER_COUNT);

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, VertexCount * sizeof(skinned_vertex) + MaxInstanceCount * sizeof(render_instance), 0, GL_STREAM_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, VertexCount * sizeof(skinned_vertex), Vertices);
    glBufferSubData(GL_ARRAY_BUFFER, VertexCount * sizeof(skinned_vertex), MaxInstanceCount * sizeof(render_instance), 0);

    // per-vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Tangent));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Bitangent));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, TextureCoords));

    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_UNSIGNED_INT, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, JointIndices));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Weights));

    // per-instance attributes
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *)(VertexCount * sizeof(skinned_vertex) + StructOffset(render_instance, Model) + 0));
    glVertexAttribDivisor(7, 1);

    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *)(VertexCount * sizeof(skinned_vertex) + StructOffset(render_instance, Model) + sizeof(vec4)));
    glVertexAttribDivisor(8, 1);

    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *)(VertexCount * sizeof(skinned_vertex) + StructOffset(render_instance, Model) + 2 * sizeof(vec4)));
    glVertexAttribDivisor(9, 1);

    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *)(VertexCount * sizeof(skinned_vertex) + StructOffset(render_instance, Model) + 3 * sizeof(vec4)));
    glVertexAttribDivisor(10, 1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexCount * sizeof(u32), Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    opengl_mesh_buffer *MeshBuffer = State->MeshBuffers + State->CurrentMeshBufferCount++;
    MeshBuffer->Id = MeshId;
    MeshBuffer->VertexCount = VertexCount;
    MeshBuffer->IndexCount = IndexCount;
    MeshBuffer->VAO = VAO;
    MeshBuffer->VBO = VBO;
    MeshBuffer->EBO = EBO;
}

inline GLint
OpenGLGetTextureFormat(bitmap *Bitmap)
{
    if (Bitmap->Channels == 1) return GL_RED;
    if (Bitmap->Channels == 2) return GL_RG;
    if (Bitmap->Channels == 3) return GL_RGB;
    if (Bitmap->Channels == 4) return GL_RGBA;

    Assert(!"Invalid number of channels");

    return -1;
}

internal void
OpenGLAddTexture(opengl_state *State, u32 Id, bitmap *Bitmap)
{
    Assert(State->CurrentTextureCount < OPENGL_MAX_TEXTURE_COUNT);

    GLuint TextureHandle;

    glGenTextures(1, &TextureHandle);
    glBindTexture(GL_TEXTURE_2D, TextureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLint Format = OpenGLGetTextureFormat(Bitmap);

    glTexImage2D(GL_TEXTURE_2D, 0, Format, Bitmap->Width, Bitmap->Height, 0, Format, GL_UNSIGNED_BYTE, Bitmap->Pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    opengl_texture *Texture = State->Textures + State->CurrentTextureCount++;
    Texture->Id = Id;
    Texture->Handle = TextureHandle;
}

internal void
OpenGLAddShader(opengl_state *State, u32 Id, char *VertexShaderSource, char *FragmentShaderSource)
{
    GLuint VertexShader = CreateShader(GL_VERTEX_SHADER, VertexShaderSource);
    GLuint FragmentShader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSource);
    GLuint Program = CreateProgram(VertexShader, FragmentShader);

    opengl_shader *Shader = State->Shaders + State->CurrentShaderCount++;

    Shader->Id = Id;
    Shader->Program = Program;

    Shader->ModelUniformLocation = glGetUniformLocation(Program, "u_Model");
    Shader->ViewUniformLocation = glGetUniformLocation(Program, "u_View");
    Shader->ProjectionUniformLocation = glGetUniformLocation(Program, "u_Projection");
    Shader->SkinningMatricesSamplerUniformLocation = glGetUniformLocation(Program, "u_SkinningMatricesSampler");

    Shader->ColorUniformLocation = glGetUniformLocation(Program, "u_Color");
    Shader->CameraPositionUniformLocation = glGetUniformLocation(Program, "u_CameraPosition");

    Shader->MaterialSpecularShininessUniformLocation = glGetUniformLocation(Program, "u_Material.SpecularShininess");

    Shader->MaterialAmbientColorUniformLocation = glGetUniformLocation(Program, "u_Material.AmbientColor");
    Shader->MaterialDiffuseColorUniformLocation = glGetUniformLocation(Program, "u_Material.DiffuseColor");
    Shader->MaterialSpecularColorUniformLocation = glGetUniformLocation(Program, "u_Material.SpecularColor");

    Shader->MaterialDiffuseMapUniformLocation = glGetUniformLocation(Program, "u_Material.DiffuseMap");
    Shader->MaterialSpecularMapUniformLocation = glGetUniformLocation(Program, "u_Material.SpecularMap");
    Shader->MaterialShininessMapUniformLocation = glGetUniformLocation(Program, "u_Material.ShininessMap");
    Shader->MaterialNormalMapUniformLocation = glGetUniformLocation(Program, "u_Material.NormalMap");

    Shader->MaterialHasDiffuseMapUniformLocation = glGetUniformLocation(Program, "u_Material.HasDiffuseMap");
    Shader->MaterialHasSpecularMapUniformLocation = glGetUniformLocation(Program, "u_Material.HasSpecularMap");
    Shader->MaterialHasShininessMapUniformLocation = glGetUniformLocation(Program, "u_Material.HasShininessMap");
    Shader->MaterialHasNormalMapUniformLocation = glGetUniformLocation(Program, "u_Material.HasNormalMap");

    Shader->DirectionalLightDirectionUniformLocation = glGetUniformLocation(Program, "u_DirectionalLight.Direction");
    Shader->DirectionalLightColorUniformLocation = glGetUniformLocation(Program, "u_DirectionalLight.Color");

    Shader->PointLight1PositionUniformLocation = glGetUniformLocation(Program, "u_PointLights[0].Position");
    Shader->PointLight1ColorUniformLocation = glGetUniformLocation(Program, "u_PointLights[0].Color");
    Shader->PointLight1AttenuationConstantUniformLocation = glGetUniformLocation(Program, "u_PointLights[0].Attenuation.Constant");
    Shader->PointLight1AttenuationLinearUniformLocation = glGetUniformLocation(Program, "u_PointLights[0].Attenuation.Linear");
    Shader->PointLight1AttenuationQuadraticUniformLocation = glGetUniformLocation(Program, "u_PointLights[0].Attenuation.Quadratic");

    Shader->PointLight2PositionUniformLocation = glGetUniformLocation(Program, "u_PointLights[1].Position");
    Shader->PointLight2ColorUniformLocation = glGetUniformLocation(Program, "u_PointLights[1].Color");
    Shader->PointLight2AttenuationConstantUniformLocation = glGetUniformLocation(Program, "u_PointLights[1].Attenuation.Constant");
    Shader->PointLight2AttenuationLinearUniformLocation = glGetUniformLocation(Program, "u_PointLights[1].Attenuation.Linear");
    Shader->PointLight2AttenuationQuadraticUniformLocation = glGetUniformLocation(Program, "u_PointLights[1].Attenuation.Quadratic");

    Shader->ScreenTextureUniformLocation = glGetUniformLocation(Program, "u_ScreenTexture");
    Shader->TimeUniformLocation = glGetUniformLocation(Program, "u_Time");
}

internal void
OpenGLBlinnPhongShading(opengl_state *State, opengl_shader *Shader, mesh_material *MeshMaterial, point_light *PointLight1, point_light *PointLight2)
{
    glUniform1i(Shader->MaterialHasDiffuseMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasSpecularMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasShininessMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasNormalMapUniformLocation, false);

    // todo: magic numbers
    opengl_texture *Texture = OpenGLGetTexture(State, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, Texture->Handle);

    for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MeshMaterial->PropertyCount; ++MaterialPropertyIndex)
    {
        material_property *MaterialProperty = MeshMaterial->Properties + MaterialPropertyIndex;

        switch (MaterialProperty->Type)
        {
            case MaterialProperty_Float_Shininess:
            {
                glUniform1f(Shader->MaterialSpecularShininessUniformLocation, MaterialProperty->Value);
                break;
            }
            case MaterialProperty_Color_Ambient:
            {
                glUniform3f(
                    Shader->MaterialAmbientColorUniformLocation,
                    MaterialProperty->Color.r,
                    MaterialProperty->Color.g,
                    MaterialProperty->Color.b
                );
                break;
            }
            case MaterialProperty_Color_Diffuse:
            {
                glUniform3f(
                    Shader->MaterialDiffuseColorUniformLocation,
                    MaterialProperty->Color.r,
                    MaterialProperty->Color.g,
                    MaterialProperty->Color.b
                );
                break;
            }
            case MaterialProperty_Color_Specular:
            {
                glUniform3f(
                    Shader->MaterialSpecularColorUniformLocation,
                    MaterialProperty->Color.r,
                    MaterialProperty->Color.g,
                    MaterialProperty->Color.b
                );
                break;
            }
            case MaterialProperty_Texture_Diffuse:
            {
                opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->Id);

                glActiveTexture(GL_TEXTURE0 + MaterialPropertyIndex);
                glBindTexture(GL_TEXTURE_2D, Texture->Handle);

                glUniform1i(Shader->MaterialHasDiffuseMapUniformLocation, true);
                glUniform1i(Shader->MaterialDiffuseMapUniformLocation, MaterialPropertyIndex);
                break;
            }
            case MaterialProperty_Texture_Specular:
            {
                opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->Id);

                glActiveTexture(GL_TEXTURE0 + MaterialPropertyIndex);
                glBindTexture(GL_TEXTURE_2D, Texture->Handle);

                glUniform1i(Shader->MaterialHasSpecularMapUniformLocation, true);
                glUniform1i(Shader->MaterialSpecularMapUniformLocation, MaterialPropertyIndex);
                break;
            }
            case MaterialProperty_Texture_Shininess:
            {
                opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->Id);

                glActiveTexture(GL_TEXTURE0 + MaterialPropertyIndex);
                glBindTexture(GL_TEXTURE_2D, Texture->Handle);

                glUniform1i(Shader->MaterialHasShininessMapUniformLocation, true);
                glUniform1i(Shader->MaterialShininessMapUniformLocation, MaterialPropertyIndex);
                break;
            }
            case MaterialProperty_Texture_Normal:
            {
                opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->Id);

                glActiveTexture(GL_TEXTURE0 + MaterialPropertyIndex);
                glBindTexture(GL_TEXTURE_2D, Texture->Handle);

                glUniform1i(Shader->MaterialHasNormalMapUniformLocation, true);
                glUniform1i(Shader->MaterialNormalMapUniformLocation, MaterialPropertyIndex);
                break;
            }
            default:
            {
                Assert(!"Invalid material property");
            }
        }
    }

    // Point Lights
    {
        glUniform3f(
            Shader->PointLight1PositionUniformLocation,
            PointLight1->Position.x,
            PointLight1->Position.y,
            PointLight1->Position.z
        );
        glUniform3f(
            Shader->PointLight1ColorUniformLocation,
            PointLight1->Color.r,
            PointLight1->Color.g,
            PointLight1->Color.b
        );
        glUniform1f(Shader->PointLight1AttenuationConstantUniformLocation, PointLight1->Attenuation.Constant);
        glUniform1f(Shader->PointLight1AttenuationLinearUniformLocation, PointLight1->Attenuation.Linear);
        glUniform1f(Shader->PointLight1AttenuationQuadraticUniformLocation, PointLight1->Attenuation.Quadratic);
    }

    {
        glUniform3f(
            Shader->PointLight2PositionUniformLocation,
            PointLight2->Position.x,
            PointLight2->Position.y,
            PointLight2->Position.z
        );
        glUniform3f(
            Shader->PointLight2ColorUniformLocation,
            PointLight2->Color.r,
            PointLight2->Color.g,
            PointLight2->Color.b
        );
        glUniform1f(Shader->PointLight2AttenuationConstantUniformLocation, PointLight2->Attenuation.Constant);
        glUniform1f(Shader->PointLight2AttenuationLinearUniformLocation, PointLight2->Attenuation.Linear);
        glUniform1f(Shader->PointLight2AttenuationQuadraticUniformLocation, PointLight2->Attenuation.Quadratic);
    }
}

// todo: reloadable shaders?
// todo: fix antialiasing?
internal void
OpenGLProcessRenderCommands(opengl_state *State, render_commands *Commands)
{
    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, State->Framebuffers[Entry->RenderTarget]);

        switch (Entry->Type)
        {
            case RenderCommand_InitRenderer:
            {
                render_command_init_renderer *Command = (render_command_init_renderer *)Entry;

                InitLine(State);
                InitRectangle(State);

                OpenGLAddShader(State, OPENGL_SIMPLE_SHADER_ID, SimpleVertexShader, SimpleFragmentShader);
                OpenGLAddShader(State, OPENGL_PHONG_SHADING_SHADER_ID, ForwardShadingVertexShader, ForwardShadingFragmentShader);
                OpenGLAddShader(State, OPENGL_SKINNED_PHONG_SHADING_SHADER_ID, SkinnedMeshVertexShader, ForwardShadingFragmentShader);
                OpenGLAddShader(State, OPENGL_FRAMEBUFFER_SHADER_ID, FramebufferVertexShader, FramebufferFragmentShader);
                OpenGLAddShader(State, OPENGL_GROUND_SHADER_ID, GroundVertexShader, GroundFragmentShader);
                OpenGLAddShader(State, OPENGL_INSTANCED_PHONG_SHADING_SHADER_ID, InstancedForwardShadingVertexShader, ForwardShadingFragmentShader);

                glGenBuffers(1, &State->SkinningTBO);
                glBindBuffer(GL_TEXTURE_BUFFER, State->SkinningTBO);
                // todo: size?
                glBufferData(GL_TEXTURE_BUFFER, Kilobytes(32), 0, GL_STREAM_DRAW);

                glGenTextures(1, &State->SkinningTBOTexture);

                glBindBuffer(GL_TEXTURE_BUFFER, 0);

                // todo: cleanup
                bitmap WhiteTexture = {};
                WhiteTexture.Width = 1;
                WhiteTexture.Height = 1;
                WhiteTexture.Channels = 4;
                u32 *WhitePixel = PushType(&State->Arena, u32);
                *WhitePixel = 0xFFFFFFFF;
                WhiteTexture.Pixels = WhitePixel;

                OpenGLAddTexture(State, 0, &WhiteTexture);

                //glEnable(GL_CULL_FACE);
                //glCullFace(GL_BACK);
                glFrontFace(GL_CCW);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_LINE_SMOOTH);
                glEnable(GL_MULTISAMPLE);

                // todo: use GL_ZERO_TO_ONE?
                glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);

                glGenFramebuffers(ArrayCount(State->Framebuffers) - 1, State->Framebuffers);
                glGenRenderbuffers(ArrayCount(State->FramebufferColorRBOs) - 1, State->FramebufferColorRBOs);
                glGenRenderbuffers(ArrayCount(State->FramebufferDepthRBOs) - 1, State->FramebufferDepthRBOs);

                GLint MSAA = 16;

                // todo: handle resizing
                for (u32 FramebufferIndex = 0; FramebufferIndex < ArrayCount(State->Framebuffers) - 1; ++FramebufferIndex)
                {
#if 0
                    glBindTexture(GL_TEXTURE_2D, State->FramebufferTextures[FramebufferIndex]);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                    // todo: rgb or rgba?
                    // todo: multisample?
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Commands->WindowWidth, Commands->WindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                    glBindTexture(GL_TEXTURE_2D, 0);
#endif
                    glBindRenderbuffer(GL_RENDERBUFFER, State->FramebufferColorRBOs[FramebufferIndex]);
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA, GL_RGB8, Commands->WindowWidth, Commands->WindowHeight);

                    glBindRenderbuffer(GL_RENDERBUFFER, State->FramebufferDepthRBOs[FramebufferIndex]);
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA, GL_DEPTH_COMPONENT, Commands->WindowWidth, Commands->WindowHeight);

                    glBindFramebuffer(GL_FRAMEBUFFER, State->Framebuffers[FramebufferIndex]);
                    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, State->FramebufferTextures[FramebufferIndex], 0);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, State->FramebufferColorRBOs[FramebufferIndex]);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, State->FramebufferDepthRBOs[FramebufferIndex]);

                    GLenum FramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                    Assert(FramebufferStatus == GL_FRAMEBUFFER_COMPLETE)
                }

                i32 FramebufferIndex = ArrayCount(State->Framebuffers) - 1;

#if 1
                glGenFramebuffers(1, &State->Framebuffers[FramebufferIndex]);
                glGenTextures(1, &State->FramebufferTextures[FramebufferIndex]);
                glGenRenderbuffers(1, &State->FramebufferDepthRBOs[FramebufferIndex]);

                glBindTexture(GL_TEXTURE_2D, State->FramebufferTextures[FramebufferIndex]);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap generation included in OpenGL v1.4

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Commands->WindowWidth, Commands->WindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                glBindTexture(GL_TEXTURE_2D, 0);
#endif

                glBindRenderbuffer(GL_RENDERBUFFER, State->FramebufferDepthRBOs[FramebufferIndex]);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Commands->WindowWidth, Commands->WindowHeight);

                glBindFramebuffer(GL_FRAMEBUFFER, State->Framebuffers[FramebufferIndex]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, State->FramebufferTextures[FramebufferIndex], 0);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, State->FramebufferDepthRBOs[FramebufferIndex]);

                GLenum FramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                Assert(FramebufferStatus == GL_FRAMEBUFFER_COMPLETE)

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                
                break;
            }
            case RenderCommand_AddMesh:
            {
                render_command_add_mesh *Command = (render_command_add_mesh *)Entry;

                if (Command->MaxInstanceCount > 0)
                {
                    OpenGLAddMeshBufferInstanced(State, Command->MeshId, Command->VertexCount, Command->Vertices, Command->IndexCount, Command->Indices, Command->MaxInstanceCount);
                }
                else
                {
                    OpenGLAddMeshBuffer(State, Command->MeshId, Command->VertexCount, Command->Vertices, Command->IndexCount, Command->Indices);
                }

                
                break;
            }
            case RenderCommand_AddTexture:
            {
                render_command_add_texture *Command = (render_command_add_texture *)Entry;

                OpenGLAddTexture(State, Command->Id, Command->Bitmap);
                
                break;
            }
            case RenderCommand_SetViewport:
            {
                render_command_set_viewport *Command = (render_command_set_viewport *)Entry;

                glViewport(Command->x, Command->y, Command->Width, Command->Height);
                
                break;
            }
            case RenderCommand_SetOrthographicProjection:
            {
                render_command_set_orthographic_projection *Command = (render_command_set_orthographic_projection *)Entry;

                mat4 Projection = Orthographic(Command->Left, Command->Right, Command->Bottom, Command->Top, Command->Near, Command->Far);

                // todo: use uniform buffer?
                for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
                {
                    opengl_shader *Shader = State->Shaders + ShaderIndex;

                    glUseProgram(Shader->Program);
                    glUniformMatrix4fv(Shader->ProjectionUniformLocation, 1, GL_TRUE, &Projection.Elements[0][0]);
                    glUseProgram(0);
                }
                
                break;
            }
            case RenderCommand_SetPerspectiveProjection:
            {
                render_command_set_perspective_projection *Command = (render_command_set_perspective_projection *)Entry;

                mat4 Projection = Perspective(Command->FovY, Command->Aspect, Command->Near, Command->Far);

                // todo: use uniform buffer?
                for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
                {
                    opengl_shader *Shader = State->Shaders + ShaderIndex;

                    glUseProgram(Shader->Program);
                    glUniformMatrix4fv(Shader->ProjectionUniformLocation, 1, GL_TRUE, &Projection.Elements[0][0]);
                    glUseProgram(0);
                }
                
                break;
            }
            case RenderCommand_SetCamera:
            {
                render_command_set_camera *Command = (render_command_set_camera *)Entry;

                mat4 View = LookAt(Command->Position, Command->Target, Command->Up);

                for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
                {
                    opengl_shader *Shader = State->Shaders + ShaderIndex;

                    glUseProgram(Shader->Program);
                    glUniformMatrix4fv(Shader->ViewUniformLocation, 1, GL_TRUE, &View.Elements[0][0]);
                    glUniform3f(Shader->CameraPositionUniformLocation, Command->Position.x, Command->Position.y, Command->Position.z);
                    glUseProgram(0);
                }

                break;
            }
            case RenderCommand_Clear:
            {
                render_command_clear *Command = (render_command_clear *)Entry;

                glClearColor(Command->Color.x, Command->Color.y, Command->Color.z, Command->Color.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                break;
            }
            case RenderCommand_DrawLine:
            {
                render_command_draw_line *Command = (render_command_draw_line *)Entry;

                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                glLineWidth(Command->Thickness);

                glBindVertexArray(State->LineVAO);
                glUseProgram(Shader->Program);
                {
                    mat4 T = Translate(Command->Start);
                    mat4 S = Scale(Command->End - Command->Start);
                    mat4 Model = T * S;

                    glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);
                    glUniform4f(Shader->ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
                }
                glDrawArrays(GL_LINES, 0, 2);

                glUseProgram(0);
                glBindVertexArray(0);

                glLineWidth(1.f);
                
                break;
            }
            case RenderCommand_DrawRectangle:
            {
                render_command_draw_rectangle *Command = (render_command_draw_rectangle *)Entry;

                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                glBindVertexArray(State->RectangleVAO);
                glUseProgram(Shader->Program);

                mat4 Model = Transform(Command->Transform);
                mat4 View = mat4(1.f);

                glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);
                glUniformMatrix4fv(Shader->ViewUniformLocation, 1, GL_TRUE, &View.Elements[0][0]);
                glUniform4f(Shader->ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glUseProgram(0);
                glBindVertexArray(0);
                
                break;
            }
            case RenderCommand_DrawGround:
            {
                // http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
                render_command_draw_ground *Command = (render_command_draw_ground *)Entry;

                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_GROUND_SHADER_ID);

                glBindVertexArray(State->RectangleVAO);
                glUseProgram(Shader->Program);

                glUniform3f(Shader->CameraPositionUniformLocation, Command->CameraPosition.x, Command->CameraPosition.y, Command->CameraPosition.z);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glUseProgram(0);
                glBindVertexArray(0);
                
                break;
            }
            case RenderCommand_SetDirectionalLight:
            {
                render_command_set_directional_light *Command = (render_command_set_directional_light *)Entry;

                for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
                {
                    opengl_shader *Shader = State->Shaders + ShaderIndex;

                    glUseProgram(Shader->Program);
                    glUniform3f(Shader->DirectionalLightDirectionUniformLocation, Command->Light.Direction.x, Command->Light.Direction.y, Command->Light.Direction.z);
                    glUniform3f(Shader->DirectionalLightColorUniformLocation, Command->Light.Color.r, Command->Light.Color.g, Command->Light.Color.b);
                    glUseProgram(0);
                }

                break;
            }
            case RenderCommand_DrawMesh:
            {
                render_command_draw_mesh *Command = (render_command_draw_mesh *)Entry;

                opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);

                glBindVertexArray(MeshBuffer->VAO);

                switch (Command->Material.Type)
                {
                    case MaterialType_BlinnPhong:
                    {
                        mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                        opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_SHADING_SHADER_ID);

                        glUseProgram(Shader->Program);

                        mat4 Model = Transform(Command->Transform);
                        glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

                        OpenGLBlinnPhongShading(State, Shader, Command->Material.MeshMaterial, &Command->PointLight1, &Command->PointLight2);

                        break;
                    }
                    case MaterialType_Unlit:
                    {
                        opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                        glUseProgram(Shader->Program);

                        mat4 Model = Transform(Command->Transform);
                        material Material = Command->Material;

                        glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);
                        glUniform4f(Shader->ColorUniformLocation, Material.Color.r, Material.Color.g, Material.Color.b, 1.f);

                        break;
                    }
                    default:
                    {
                        Assert(!"Invalid material type");
                        break;
                    }
                }

                GLint PrevPolygonMode[2];
                glGetIntegerv(GL_POLYGON_MODE, PrevPolygonMode);
                glPolygonMode(GL_FRONT_AND_BACK, Command->Material.IsWireframe ? GL_LINE : GL_FILL);

                glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);

                glPolygonMode(GL_FRONT_AND_BACK, PrevPolygonMode[0]);

                glUseProgram(0);
                glBindVertexArray(0);
                
                break;
            }
            case RenderCommand_DrawSkinnedMesh:
            {
                render_command_draw_skinned_mesh *Command = (render_command_draw_skinned_mesh *)Entry;

                opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);

                glBindVertexArray(MeshBuffer->VAO);

                switch (Command->Material.Type)
                {
                    // todo: organize materials (https://threejs.org/docs/#api/en/materials/MeshPhongMaterial)
                    case MaterialType_BlinnPhong:
                    {
                        mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                        opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SKINNED_PHONG_SHADING_SHADER_ID);

                        glUseProgram(Shader->Program);

                        glBindBuffer(GL_TEXTURE_BUFFER, State->SkinningTBO);
                        glBufferSubData(GL_TEXTURE_BUFFER, 0, Command->SkinningMatrixCount * sizeof(mat4), Command->SkinningMatrices);
                        glBindBuffer(GL_TEXTURE_BUFFER, 0);

                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_BUFFER, State->SkinningTBOTexture);
                        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, State->SkinningTBO);

                        glUniform1i(Shader->SkinningMatricesSamplerUniformLocation, 0);

                        OpenGLBlinnPhongShading(State, Shader, Command->Material.MeshMaterial, &Command->PointLight1, &Command->PointLight2);

                        break;
                    }
                    case MaterialType_Unlit:
                    {
                        Assert(!"Not Implemented");

                        break;
                    }
                    default:
                    {
                        Assert(!"Invalid material type");
                        break;
                    }
                }

                GLint PrevPolygonMode[2];
                glGetIntegerv(GL_POLYGON_MODE, PrevPolygonMode);
                glPolygonMode(GL_FRONT_AND_BACK, Command->Material.IsWireframe ? GL_LINE : GL_FILL);

                glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);

                glPolygonMode(GL_FRONT_AND_BACK, PrevPolygonMode[0]);

                glUseProgram(0);
                glBindVertexArray(0);

                break;
            }
            case RenderCommand_DrawMeshInstanced:
            {
                render_command_draw_mesh_instanced *Command = (render_command_draw_mesh_instanced *)Entry;

                opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);

                glBindVertexArray(MeshBuffer->VAO);
                glBufferSubData(GL_ARRAY_BUFFER, MeshBuffer->VertexCount * sizeof(skinned_vertex), Command->InstanceCount * sizeof(render_instance), Command->Instances);

                switch (Command->Material.Type)
                {
                    case MaterialType_BlinnPhong:
                    {
                        mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                        opengl_shader *Shader = OpenGLGetShader(State, OPENGL_INSTANCED_PHONG_SHADING_SHADER_ID);

                        glUseProgram(Shader->Program);

                        OpenGLBlinnPhongShading(State, Shader, Command->Material.MeshMaterial, &Command->PointLight1, &Command->PointLight2);

                        break;
                    }
                    case MaterialType_Unlit:
                    {
                        Assert(!"Not Implemented");
#if 0
                        opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                        glUseProgram(Shader->Program);

                        mat4 Model = Transform(Command->Transform);
                        material Material = Command->Material;

                        glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);
                        glUniform4f(Shader->ColorUniformLocation, Material.Color.r, Material.Color.g, Material.Color.b, 1.f);
#endif

                        break;
                    }
                    default:
                    {
                        Assert(!"Invalid material type");
                        break;
                    }
                }

                // todo: make polygon mode that operates globally and not on per-command basis
                GLint PrevPolygonMode[2];
                glGetIntegerv(GL_POLYGON_MODE, PrevPolygonMode);
                glPolygonMode(GL_FRONT_AND_BACK, Command->Material.IsWireframe ? GL_LINE : GL_FILL);

                glDrawElementsInstanced(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0, Command->InstanceCount);

                glPolygonMode(GL_FRONT_AND_BACK, PrevPolygonMode[0]);

                glUseProgram(0);
                glBindVertexArray(0);

                break;
            }
            default:
            {
                Assert(!"Render command is not supported");
            }
        }

        BaseAddress += Entry->Size;
    }

#if 0
    glDisable(GL_DEPTH_TEST);

    // copy rendered image from MSAA (multi-sample) to normal (single-sample)
    // NOTE: The multi samples at a pixel in read buffer will be converted
    // to a single sample at the target pixel in draw buffer.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, State->Framebuffers[0]); // src FBO (multi-sample)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, State->Framebuffers[ArrayCount(State->Framebuffers) - 1]);     // dst FBO (single-sample)

    glBlitFramebuffer(0, 0, Commands->WindowWidth, Commands->WindowHeight, 0, 0, Commands->WindowWidth, Commands->WindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, State->Framebuffers[ArrayCount(State->Framebuffers) - 1]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, Commands->WindowWidth, Commands->WindowHeight);
    glClearColor(1.f, 0.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_FRAMEBUFFER_SHADER_ID);

    glBindVertexArray(State->RectangleVAO);
    glUseProgram(Shader->Program);

    glUniform1f(Shader->TimeUniformLocation, Commands->Time);

    glBindTexture(GL_TEXTURE_2D, State->FramebufferTextures[ArrayCount(State->Framebuffers) - 1]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_DEPTH_TEST);
#endif
}
