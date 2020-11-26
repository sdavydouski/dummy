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

        //PrintError("Error: ", "Shader compilation failed", ErrorLog);

        glDeleteShader(Shader);
    }
    Assert(IsShaderCompiled);

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

        int t = 0;

        //PrintError("Error: ", "Shader program linkage failed", ErrorLog);
    }
    Assert(IsProgramLinked);

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

internal void
InitGrid(opengl_state *State, u32 GridCount)
{
    scoped_memory ScopedMemory(&State->Arena);

    vec3 *GridVertices = PushArray(ScopedMemory.Arena, GridCount * 8, vec3);

    for (u32 Index = 0; Index < GridCount; ++Index)
    {
        f32 Coord = (f32)(Index + 1) / (f32)GridCount;
        u32 GridVertexIndex = Index * 8;

        vec3 *GridVertex0 = GridVertices + GridVertexIndex + 0;
        *GridVertex0 = vec3(-Coord, 0.f, -1.f);

        vec3 *GridVertex1 = GridVertices + GridVertexIndex + 1;
        *GridVertex1 = vec3(-Coord, 0.f, 1.f);

        vec3 *GridVertex2 = GridVertices + GridVertexIndex + 2;
        *GridVertex2 = vec3(Coord, 0.f, -1.f);

        vec3 *GridVertex3 = GridVertices + GridVertexIndex + 3;
        *GridVertex3 = vec3(Coord, 0.f, 1.f);

        vec3 *GridVertex4 = GridVertices + GridVertexIndex + 4;
        *GridVertex4 = vec3(-1.f, 0.f, -Coord);

        vec3 *GridVertex5 = GridVertices + GridVertexIndex + 5;
        *GridVertex5 = vec3(1.f, 0.f, -Coord);

        vec3 *GridVertex6 = GridVertices + GridVertexIndex + 6;
        *GridVertex6 = vec3(-1.f, 0.f, Coord);

        vec3 *GridVertex7 = GridVertices + GridVertexIndex + 7;
        *GridVertex7 = vec3(1.f, 0.f, Coord);
    }

    glGenVertexArrays(1, &State->GridVAO);
    glBindVertexArray(State->GridVAO);

    GLuint GridVBO;
    glGenBuffers(1, &GridVBO);
    glBindBuffer(GL_ARRAY_BUFFER, GridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * GridCount * 8, GridVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);

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
        }
    }

    Assert(Result);

    return Result;
}

// todo: use hashtable
inline opengl_texture *
OpenGLGetTexture(opengl_state *State, u32 Id)
{
    // todo: why?
    Assert((GL_TEXTURE0 + Id) < GL_TEXTURE15);

    opengl_texture *Result = 0;

    for (u32 TextureIndex = 0; TextureIndex < State->CurrentTextureCount; ++TextureIndex)
    {
        opengl_texture *Texture = State->Textures + TextureIndex;

        if (Texture->Id == Id)
        {
            Result = Texture;
        }
    }

    Assert(Result);

    return Result;
}

internal void
OpenGLAddMeshBuffer(opengl_state *State, u32 Id, primitive_type PrimitiveType, u32 VertexCount, skinned_vertex *Vertices, u32 IndexCount, u32 *Indices)
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
    MeshBuffer->Id = Id;
    MeshBuffer->PrimitiveType = PrimitiveType;
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

inline opengl_shader *
OpenGLGetShader(opengl_state *State, u32 Id)
{
    opengl_shader *Result = State->Shaders + Id;

    Assert(Result);

    return Result;
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
                InitGrid(State, Command->GridCount);

                OpenGLAddShader(State, OPENGL_SIMPLE_SHADER_ID, SimpleVertexShader, SimpleFragmentShader);
                OpenGLAddShader(State, OPENGL_GRID_SHADER_ID, GridVertexShader, GridFragmentShader);
                OpenGLAddShader(State, OPENGL_PHONG_SHADING_SHADER_ID, ForwardShadingVertexShader, ForwardShadingFragmentShader);
                OpenGLAddShader(State, OPENGL_SKINNED_PHONG_SHADING_SHADER_ID, SkinnedMeshVertexShader, ForwardShadingFragmentShader);
                OpenGLAddShader(State, OPENGL_FRAMEBUFFER_SHADER_ID, FramebufferVertexShader, FramebufferFragmentShader);

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

                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
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

                GLint MSAA = 4;

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

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_AddMesh:
            {
                render_command_add_mesh *Command = (render_command_add_mesh *)Entry;

                OpenGLAddMeshBuffer(State, Command->Id, Command->PrimitiveType, Command->VertexCount, Command->Vertices, Command->IndexCount, Command->Indices);

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_AddTexture:
            {
                render_command_add_texture *Command = (render_command_add_texture *)Entry;

                OpenGLAddTexture(State, Command->Id, Command->Bitmap);

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_SetViewport:
            {
                render_command_set_viewport *Command = (render_command_set_viewport *)Entry;

                glViewport(Command->x, Command->y, Command->Width, Command->Height);

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_SetOrthographicProjection:
            {
                render_command_set_orthographic_projection *Command = (render_command_set_orthographic_projection *)Entry;

                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                // todo: use uniform buffer
                glUseProgram(Shader->Program);
                {
                    mat4 Projection = Orthographic(Command->Left, Command->Right, Command->Bottom, Command->Top, Command->Near, Command->Far);

                    glUniformMatrix4fv(Shader->ProjectionUniformLocation, 1, GL_TRUE, &Projection.Elements[0][0]);
                }
                glUseProgram(0);

                BaseAddress += sizeof(*Command);
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

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_SetCamera:
            {
                render_command_set_camera *Command = (render_command_set_camera *)Entry;

                mat4 View = LookAtLH(Command->Eye, Command->Target, Command->Up);

                for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
                {
                    opengl_shader *Shader = State->Shaders + ShaderIndex;

                    glUseProgram(Shader->Program);
                    glUniformMatrix4fv(Shader->ViewUniformLocation, 1, GL_TRUE, &View.Elements[0][0]);
                    glUniform3f(Shader->CameraPositionUniformLocation, Command->Position.x, Command->Position.y, Command->Position.z);
                    glUseProgram(0);
                }

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_Clear:
            {
                render_command_clear *Command = (render_command_clear *)Entry;

                glClearColor(Command->Color.x, Command->Color.y, Command->Color.z, Command->Color.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                BaseAddress += sizeof(*Command);
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

                BaseAddress += sizeof(*Command);
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

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_DrawGrid:
            {
                render_command_draw_grid *Command = (render_command_draw_grid *)Entry;

                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_GRID_SHADER_ID);

                glLineWidth(2.f);

                glBindVertexArray(State->GridVAO);
                glUseProgram(Shader->Program);

                mat4 Model = Scale(Command->Size);

                glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);
                glUniform3f(Shader->ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b);
                glUniform3f(Shader->CameraPositionUniformLocation, Command->CameraPosition.x, Command->CameraPosition.y, Command->CameraPosition.z);

                glDrawArrays(GL_LINES, 0, Command->Count * 8);

                glUseProgram(0);
                glBindVertexArray(0);

                glLineWidth(1.f);

                BaseAddress += sizeof(*Command);
                // todo: ?
                //BaseAddress += Command->Header.Size;

                break;
            }
            case RenderCommand_DrawMesh:
            {
                render_command_draw_mesh *Command = (render_command_draw_mesh *)Entry;

                opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->Id);

                glBindVertexArray(MeshBuffer->VAO);

                switch (Command->Material.Type)
                {
#if 0
                    case MaterialType_Standard:
                    {
                        glUseProgram(State->ForwardShadingShaderProgram);

                        mat4 Model = Transform(Command->Transform);

                        glUniformMatrix4fv(ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

                        // todo: store uniform locations
                        glUniform3f(
                            MaterialDiffuseColorUniformLocation,
                            Command->Material.DiffuseColor.r,
                            Command->Material.DiffuseColor.g,
                            Command->Material.DiffuseColor.b
                        );
                        glUniform1f(MaterialAmbientStrengthUniformLocation, Command->Material.AmbientStrength);
                        glUniform1f(MaterialSpecularStrengthUniformLocation, Command->Material.SpecularStrength);
                        glUniform1f(MaterialSpecularShininessUniformLocation, Command->Material.SpecularShininess);

                        // Point Lights
                        {
                            glUniform3f(
                                PointLight1PositionUniformLocation,
                                Command->PointLight1.Position.x,
                                Command->PointLight1.Position.y,
                                Command->PointLight1.Position.z
                            );
                            glUniform3f(
                                PointLight1ColorUniformLocation,
                                Command->PointLight1.Color.r,
                                Command->PointLight1.Color.g,
                                Command->PointLight1.Color.b
                            );
                            glUniform1f(PointLight1AttenuationConstantUniformLocation, Command->PointLight1.Attenuation.Constant);
                            glUniform1f(PointLight1AttenuationLinearUniformLocation, Command->PointLight1.Attenuation.Linear);
                            glUniform1f(PointLight1AttenuationQuadraticUniformLocation, Command->PointLight1.Attenuation.Quadratic);
                        }

                        {
                            glUniform3f(
                                PointLight2PositionUniformLocation,
                                Command->PointLight2.Position.x,
                                Command->PointLight2.Position.y,
                                Command->PointLight2.Position.z
                            );
                            glUniform3f(
                                PointLight2ColorUniformLocation,
                                Command->PointLight2.Color.r,
                                Command->PointLight2.Color.g,
                                Command->PointLight2.Color.b
                            );
                            glUniform1f(PointLight2AttenuationConstantUniformLocation, Command->PointLight2.Attenuation.Constant);
                            glUniform1f(PointLight2AttenuationLinearUniformLocation, Command->PointLight2.Attenuation.Linear);
                            glUniform1f(PointLight2AttenuationQuadraticUniformLocation, Command->PointLight2.Attenuation.Quadratic);
                        }

                        break;
                    }
#endif
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

                switch (MeshBuffer->PrimitiveType)
                {
                    case PrimitiveType_Line:
                    {
                        glDrawElements(GL_LINES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
                        break;
                    }
                    case PrimitiveType_Triangle:
                    {
                        glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
                        break;
                    }
                    default:
                    {
                        Assert(!"Invalid primitive type");
                    }
                }

                glPolygonMode(GL_FRONT_AND_BACK, PrevPolygonMode[0]);

                glUseProgram(0);
                glBindVertexArray(0);

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_SetDirectionalLight:
            {
                render_command_set_directional_light *Command = (render_command_set_directional_light *)Entry;

                {
                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_SHADING_SHADER_ID);

                    glUseProgram(Shader->Program);

                    glUniform3f(Shader->DirectionalLightDirectionUniformLocation, Command->Light.Direction.x, Command->Light.Direction.y, Command->Light.Direction.z);
                    glUniform3f(Shader->DirectionalLightColorUniformLocation, Command->Light.Color.r, Command->Light.Color.g, Command->Light.Color.b);

                    glUseProgram(0);
                }

                {
                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SKINNED_PHONG_SHADING_SHADER_ID);

                    glUseProgram(Shader->Program);

                    glUniform3f(Shader->DirectionalLightDirectionUniformLocation, Command->Light.Direction.x, Command->Light.Direction.y, Command->Light.Direction.z);
                    glUniform3f(Shader->DirectionalLightColorUniformLocation, Command->Light.Color.r, Command->Light.Color.g, Command->Light.Color.b);

                    glUseProgram(0);
                }

                BaseAddress += sizeof(*Command);
                break;
            }
            case RenderCommand_DrawSkinnedMesh:
            {
                render_command_draw_skinned_mesh *Command = (render_command_draw_skinned_mesh *)Entry;

                opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->Id);

                glBindVertexArray(MeshBuffer->VAO);

                switch (Command->Material.Type)
                {
                    // todo: organize materials (https://threejs.org/docs/#api/en/materials/MeshPhongMaterial)
                    case MaterialType_Phong:
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
                                Command->PointLight1.Position.x,
                                Command->PointLight1.Position.y,
                                Command->PointLight1.Position.z
                            );
                            glUniform3f(
                                Shader->PointLight1ColorUniformLocation,
                                Command->PointLight1.Color.r,
                                Command->PointLight1.Color.g,
                                Command->PointLight1.Color.b
                            );
                            glUniform1f(Shader->PointLight1AttenuationConstantUniformLocation, Command->PointLight1.Attenuation.Constant);
                            glUniform1f(Shader->PointLight1AttenuationLinearUniformLocation, Command->PointLight1.Attenuation.Linear);
                            glUniform1f(Shader->PointLight1AttenuationQuadraticUniformLocation, Command->PointLight1.Attenuation.Quadratic);
                        }

                        {
                            glUniform3f(
                                Shader->PointLight2PositionUniformLocation,
                                Command->PointLight2.Position.x,
                                Command->PointLight2.Position.y,
                                Command->PointLight2.Position.z
                            );
                            glUniform3f(
                                Shader->PointLight2ColorUniformLocation,
                                Command->PointLight2.Color.r,
                                Command->PointLight2.Color.g,
                                Command->PointLight2.Color.b
                            );
                            glUniform1f(Shader->PointLight2AttenuationConstantUniformLocation, Command->PointLight2.Attenuation.Constant);
                            glUniform1f(Shader->PointLight2AttenuationLinearUniformLocation, Command->PointLight2.Attenuation.Linear);
                            glUniform1f(Shader->PointLight2AttenuationQuadraticUniformLocation, Command->PointLight2.Attenuation.Quadratic);
                        }

                        break;
                    }
                    case MaterialType_Unlit:
                    {
                        Assert(!"Not Implemented");

#if 0
                        glUseProgram(State->SimpleShaderProgram);

                        mat4 Model = Transform(Command->Position, Command->Scale, Command->Rotation);

                        i32 ModelUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Model");
                        glUniformMatrix4fv(ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

                        i32 ColorUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Color");
                        glUniform4f(ColorUniformLocation, Command->Material.Color.r, Command->Material.Color.g, Command->Material.Color.b, 1.f);
#endif

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

                switch (MeshBuffer->PrimitiveType)
                {
                    case PrimitiveType_Line:
                    {
                        glDrawElements(GL_LINES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
                        break;
                    }
                    case PrimitiveType_Triangle:
                    {
                        glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
                        break;
                    }
                    default:
                    {
                        Assert(!"Invalid primitive type");
                    }
                }

                glPolygonMode(GL_FRONT_AND_BACK, PrevPolygonMode[0]);

                glUseProgram(0);
                glBindVertexArray(0);

                BaseAddress += sizeof(*Command);
                break;
            }
            default:
            {
                Assert(!"Render command is not supported");
            }
        }
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
