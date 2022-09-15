internal GLuint
OpenGLCreateShader(GLenum Type, GLsizei Count, char **Source, b32 CrashIfError = true)
{
    GLuint Shader = glCreateShader(Type);
    glShaderSource(Shader, Count, Source, NULL);
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
        Shader = 0;

        if (CrashIfError)
        {
            Assert(!ErrorLog);
        }
    }

    return Shader;
}

internal GLuint
OpenGLCreateProgram(GLuint VertexShader, GLuint FragmentShader, b32 CrashIfError = true)
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

        glDeleteProgram(Program);
        Program = 0;

        if (CrashIfError)
        {
            Assert(!ErrorLog);
        }
    }

    return Program;
}

internal void
OpenGLInitLine(opengl_state *State)
{
    vec3 LineVertices[] =
    {
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
OpenGLInitRectangle(opengl_state *State)
{
    f32 RectangleVertices[] = 
    {
        // potisions     // uvs
#if 1
        // todo: flipped uvs!
        -1.f, -1.f, 0.f, 0.f, 1.f,
        1.f, -1.f, 0.f,  1.f, 1.f,
        -1.f, 1.f, 0.f,  0.f, 0.f,
        1.f, 1.f, 0.f,   1.f, 0.f
#else
        0.f, 0.f, 0.f,  0.f, 1.f,
        0.f, 1.f, 0.f,  0.f, 0.f,
        1.f, 0.f, 0.f,  1.f, 1.f,
        1.f, 1.f, 0.f,  1.f, 0.f
#endif
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
inline opengl_skinning_buffer *
OpenGLGetSkinningBuffer(opengl_state *State, u32 Id)
{
    opengl_skinning_buffer *Result = 0;

    for (u32 SkinningBufferIndex = 0; SkinningBufferIndex < State->CurrentSkinningBufferCount; ++SkinningBufferIndex)
    {
        opengl_skinning_buffer *SkinningBuffer = State->SkinningBuffers + SkinningBufferIndex;

        if (SkinningBuffer->Id == Id)
        {
            Result = SkinningBuffer;
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
    opengl_shader *Result = 0;

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

inline u32
GetMeshVerticesSize(
    u32 VertexCount,
    vec3 *Positions,
    vec3 *Normals,
    vec3 *Tangents,
    vec3 *Bitangents,
    vec2 *TextureCoords,
    vec4 *Weights,
    i32 *JointIndices
)
{
    u32 Size = 0;

    if (Positions)
    {
        Size += VertexCount * sizeof(vec3);
    }

    if (Normals)
    {
        Size += VertexCount * sizeof(vec3);
    }

    if (Tangents)
    {
        Size += VertexCount * sizeof(vec3);
    }

    if (Bitangents)
    {
        Size += VertexCount * sizeof(vec3);
    }

    if (TextureCoords)
    {
        Size += VertexCount * sizeof(vec2);
    }

    if (Weights)
    {
        Size += VertexCount * sizeof(vec4);
    }

    if (JointIndices)
    {
        Size += VertexCount * sizeof(i32) * 4;
    }

    return Size;
}

internal void
OpenGLAddSkinningBuffer(opengl_state *State, u32 BufferId, u32 SkinningMatrixCount)
{
    opengl_skinning_buffer *SkinningBuffer = State->SkinningBuffers + State->CurrentSkinningBufferCount++;
    SkinningBuffer->Id = BufferId;

    glGenBuffers(1, &SkinningBuffer->SkinningTBO);
    glBindBuffer(GL_TEXTURE_BUFFER, SkinningBuffer->SkinningTBO);
    glBufferData(GL_TEXTURE_BUFFER, SkinningMatrixCount * sizeof(mat4), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    glGenTextures(1, &SkinningBuffer->SkinningTBOTexture);
}

internal void
OpenGLAddMeshBuffer(
    opengl_state *State, 
    u32 MeshId, 
    u32 VertexCount, 
    vec3 *Positions,
    vec3 *Normals,
    vec3 *Tangents,
    vec3 *Bitangents,
    vec2 *TextureCoords,
    vec4 *Weights,
    i32 *JointIndices,
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

    u32 BufferSize = GetMeshVerticesSize(VertexCount, Positions, Normals, Tangents, Bitangents, TextureCoords, Weights, JointIndices);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, BufferSize + MaxInstanceCount * sizeof(render_instance), 0, GL_STREAM_DRAW);

    // per-vertex attributes
    u64 Offset = 0;
    if (Positions)
    {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid *)Offset);

        glBufferSubData(GL_ARRAY_BUFFER, Offset, VertexCount * sizeof(vec3), Positions);
        Offset += VertexCount * sizeof(vec3);
    }

    if (Normals)
    {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid *)Offset);

        glBufferSubData(GL_ARRAY_BUFFER, Offset, VertexCount * sizeof(vec3), Normals);
        Offset += VertexCount * sizeof(vec3);
    }

    if (Tangents)
    {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid *)Offset);

        glBufferSubData(GL_ARRAY_BUFFER, Offset, VertexCount * sizeof(vec3), Tangents);
        Offset += VertexCount * sizeof(vec3);
    }

    if (Bitangents)
    {
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid *)Offset);

        glBufferSubData(GL_ARRAY_BUFFER, Offset, VertexCount * sizeof(vec3), Bitangents);
        Offset += VertexCount * sizeof(vec3);
    }

    if (TextureCoords)
    {
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid *)Offset);

        glBufferSubData(GL_ARRAY_BUFFER, Offset, VertexCount * sizeof(vec2), TextureCoords);
        Offset += VertexCount * sizeof(vec2);
    }

    if (Weights)
    {
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (GLvoid *)Offset);

        glBufferSubData(GL_ARRAY_BUFFER, Offset, VertexCount * sizeof(vec4), Weights);
        Offset += VertexCount * sizeof(vec4);
    }

    if (JointIndices)
    {
        glEnableVertexAttribArray(6);
        glVertexAttribIPointer(6, 4, GL_UNSIGNED_INT, sizeof(i32) * 4, (GLvoid *)Offset);

        glBufferSubData(GL_ARRAY_BUFFER, Offset, VertexCount * sizeof(i32) * 4, JointIndices);
        Offset += VertexCount * sizeof(i32) * 4;
    }

    if (MaxInstanceCount > 0)
    {
        // per-instance attributes
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *) (BufferSize + StructOffset(render_instance, Model) + 0));
        glVertexAttribDivisor(7, 1);

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *) (BufferSize + StructOffset(render_instance, Model) + sizeof(vec4)));
        glVertexAttribDivisor(8, 1);

        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *) (BufferSize + StructOffset(render_instance, Model) + 2 * sizeof(vec4)));
        glVertexAttribDivisor(9, 1);

        glEnableVertexAttribArray(10);
        glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *) (BufferSize + StructOffset(render_instance, Model) + 3 * sizeof(vec4)));
        glVertexAttribDivisor(10, 1);

        glEnableVertexAttribArray(11);
        glVertexAttribPointer(11, 3, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *) (BufferSize + StructOffset(render_instance, Color)));
        glVertexAttribDivisor(11, 1);
    }

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

    MeshBuffer->BufferSize = BufferSize;
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
OpenGLLoadShaderUniforms(opengl_shader *Shader)
{
    GLuint Program = Shader->Program;

    Shader->ModelUniformLocation = glGetUniformLocation(Program, "u_Model");
    Shader->SkinningMatricesSamplerUniformLocation = glGetUniformLocation(Program, "u_SkinningMatricesSampler");

    Shader->ColorUniformLocation = glGetUniformLocation(Program, "u_Color");

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

    Shader->PointLightCountUniformLocation = glGetUniformLocation(Program, "u_PointLightCount");

    Shader->ScreenTextureUniformLocation = glGetUniformLocation(Program, "u_ScreenTexture");
}

inline char *
OpenGLPreprocessShader(char *ShaderSource, u32 InitialSize, memory_arena *Arena)
{
    u32 Size = InitialSize + 256;
    char *Result = PushString(Arena, Size);
    FormatString_(Result, Size, ShaderSource, OPENGL_MAX_POINT_LIGHT_COUNT);

    return Result;
}

internal char **
OpenGLLoadShaderFile(opengl_state *State, u32 Id, char *ShaderFileName, u32 Count, memory_arena *Arena)
{
    char **Sources = PushArray(Arena, Count, char *);

    for (u32 FileIndex = 0; FileIndex < OPENGL_COMMON_SHADER_COUNT; ++FileIndex)
    {
        read_file_result CommonShaderFile = State->Platform->ReadFile((char *) OpenGLCommonShaders[FileIndex], Arena, true);
        Sources[FileIndex] = OpenGLPreprocessShader((char *) CommonShaderFile.Contents, CommonShaderFile.Size, Arena);
    }

    read_file_result ShaderFile = State->Platform->ReadFile(ShaderFileName, Arena, true);
    Sources[OPENGL_COMMON_SHADER_COUNT] = (char *) ShaderFile.Contents;

    return Sources;
}

internal void
OpenGLLoadShader(opengl_state *State, u32 Id, char *VertexShaderFileName, char *FragmentShaderFileName)
{
    opengl_shader *Shader = State->Shaders + State->CurrentShaderCount++;

    scoped_memory ScopedMemory(&State->Arena);

    u32 Count = OPENGL_COMMON_SHADER_COUNT + 1;
    char **VertexSource = OpenGLLoadShaderFile(State, Id, VertexShaderFileName, Count, ScopedMemory.Arena);
    char **FragmentSource = OpenGLLoadShaderFile(State, Id, FragmentShaderFileName, Count, ScopedMemory.Arena);

    GLuint VertexShader = OpenGLCreateShader(GL_VERTEX_SHADER, Count, VertexSource);
    GLuint FragmentShader = OpenGLCreateShader(GL_FRAGMENT_SHADER, Count, FragmentSource);
    GLuint Program = OpenGLCreateProgram(VertexShader, FragmentShader);

    Shader->Id = Id;
    Shader->Program = Program;

#if WIN32_RELOADABLE_SHADERS
    for (u32 FileIndex = 0; FileIndex < ArrayCount(Shader->CommonShaders); ++FileIndex)
    {
        win32_shader_file *CommonShader = Shader->CommonShaders + FileIndex;

        CopyString(OpenGLCommonShaders[FileIndex], CommonShader->FileName);
        CommonShader->LastWriteTime = Win32GetLastWriteTime(CommonShader->FileName);
    }

    CopyString(VertexShaderFileName, Shader->VertexShader.FileName);
    Shader->VertexShader.LastWriteTime = Win32GetLastWriteTime(Shader->VertexShader.FileName);

    CopyString(FragmentShaderFileName, Shader->FragmentShader.FileName);
    Shader->FragmentShader.LastWriteTime = Win32GetLastWriteTime(Shader->FragmentShader.FileName);
#endif

    OpenGLLoadShaderUniforms(Shader);
}

#if WIN32_RELOADABLE_SHADERS
internal void
OpenGLReloadShader(opengl_state *State, u32 Id)
{
    opengl_shader *Shader = OpenGLGetShader(State, Id);

    scoped_memory ScopedMemory(&State->Arena);

    u32 Count = OPENGL_COMMON_SHADER_COUNT + 1;
    char **VertexSource = OpenGLLoadShaderFile(State, Id, Shader->VertexShader.FileName, Count, ScopedMemory.Arena);
    char **FragmentSource = OpenGLLoadShaderFile(State, Id, Shader->FragmentShader.FileName, Count, ScopedMemory.Arena);

    GLuint VertexShader = OpenGLCreateShader(GL_VERTEX_SHADER, Count, VertexSource, false);
    GLuint FragmentShader = OpenGLCreateShader(GL_FRAGMENT_SHADER, Count, FragmentSource, false);

    GLuint Program = 0;

    if (VertexShader && FragmentShader)
    {
        Program = OpenGLCreateProgram(VertexShader, FragmentShader, false);
    }

    if (Program)
    {
        glDeleteProgram(Shader->Program);
        Shader->Program = Program;

        OpenGLLoadShaderUniforms(Shader);
    }
}
#endif

internal void
OpenGLInitShaders(opengl_state *State)
{
    OpenGLLoadShader(
        State,
        OPENGL_SIMPLE_SHADER_ID,
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\simple.vert",
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\simple.frag"
    );

    OpenGLLoadShader(
        State,
        OPENGL_PHONG_SHADING_SHADER_ID,
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\forward_shading.vert",
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\forward_shading.frag"
    );

    OpenGLLoadShader(
        State,
        OPENGL_SKINNED_PHONG_SHADING_SHADER_ID,
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\skinned_mesh.vert",
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\forward_shading.frag"
    );

    OpenGLLoadShader(
        State,
        OPENGL_FRAMEBUFFER_SHADER_ID,
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\framebuffer.vert",
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\framebuffer.frag"
    );

    OpenGLLoadShader(
        State,
        OPENGL_GROUND_SHADER_ID,
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\ground.vert",
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\ground.frag"
    );

    OpenGLLoadShader(
        State,
        OPENGL_INSTANCED_PHONG_SHADING_SHADER_ID,
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\instanced_forward_shading.vert",
        (char *)"..\\src\\renderers\\OpenGL\\shaders\\forward_shading.frag"
    );

    OpenGLLoadShader(
        State,
        OPENGL_TEXT_SHADER_ID,
        (char *) "..\\src\\renderers\\OpenGL\\shaders\\text.vert",
        (char *) "..\\src\\renderers\\OpenGL\\shaders\\text.frag"
    );
}

internal void
OpenGLBlinnPhongShading(opengl_state *State, opengl_render_options *Options, opengl_shader *Shader, mesh_material *MeshMaterial)
{
    GLint RenderShadowMapUniformLocation = glGetUniformLocation(Shader->Program, "u_RenderShadowMap");
    glUniform1i(RenderShadowMapUniformLocation, Options->RenderShadowMap);

    if (Options->RenderShadowMap)
    {
        return;
    }

    glUniform1i(Shader->MaterialHasDiffuseMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasSpecularMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasShininessMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasNormalMapUniformLocation, false);

    // todo: magic numbers
    opengl_texture *Texture = OpenGLGetTexture(State, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, Texture->Handle);

    if (!Options->RenderShadowMap)
    {
        for (u32 CascadeIndex = 0; CascadeIndex < 4; ++CascadeIndex)
        {
            u32 TextureIndex = CascadeIndex + 16;

            // Cascasde Shadow Map
            char ShadowMapUniformName[64];
            FormatString(ShadowMapUniformName, "u_CascadeShadowMaps[%d]", CascadeIndex);
            GLint ShadowMapUniformLocation = glGetUniformLocation(Shader->Program, ShadowMapUniformName);

            glActiveTexture(GL_TEXTURE0 + TextureIndex);
            glBindTexture(GL_TEXTURE_2D, State->CascadeShadowMaps[CascadeIndex]);
            glUniform1i(ShadowMapUniformLocation, TextureIndex);

            // Cascade Bounds
            char CascadeBoundsUniformName[64];
            FormatString(CascadeBoundsUniformName, "u_CascadeBounds[%d]", CascadeIndex);
            GLint CascadeBoundsUniformLocation = glGetUniformLocation(Shader->Program, CascadeBoundsUniformName);

            vec2 CascadeBounds = State->CascadeBounds[CascadeIndex];
            glUniform2f(CascadeBoundsUniformLocation, CascadeBounds.x, CascadeBounds.y);

            // Cascade View Projection
            char CascadeViewProjectionUniformName[64];
            FormatString(CascadeViewProjectionUniformName, "u_CascadeViewProjection[%d]", CascadeIndex);
            GLint CascadeViewProjectionUniformLocation = glGetUniformLocation(Shader->Program, CascadeViewProjectionUniformName);

            mat4 CascadeViewProjection = State->CascadeViewProjection[CascadeIndex];
            glUniformMatrix4fv(CascadeViewProjectionUniformLocation, 1, GL_TRUE, (f32 *) CascadeViewProjection.Elements);
        }

        GLint ShowCascadesUniformLocation = glGetUniformLocation(Shader->Program, "u_ShowCascades");
        glUniform1i(ShowCascadesUniformLocation, Options->ShowCascades);
    }

    if (MeshMaterial)
    {
        // default values
        vec3 DefaultAmbient = vec3(0.4f);
        glUniform3f(Shader->MaterialAmbientColorUniformLocation, DefaultAmbient.x, DefaultAmbient.y, DefaultAmbient.z);
        glUniform1f(Shader->MaterialSpecularShininessUniformLocation, 1.f);

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
                    if (MaterialProperty->Color.x == 0.f && MaterialProperty->Color.y == 0.f && MaterialProperty->Color.z == 0.f)
                    {
                        MaterialProperty->Color.rgb = DefaultAmbient;
                    }

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
    }
}

internal void
OpenGLInitFramebuffers(opengl_state *State, i32 WindowWidth, i32 WindowHeight)
{
    GLint MSAA = 8;

    // Setup multi-sampled FBO (primary render target)
    glGenFramebuffers(1, &State->MultiSampledFBO);

    // Build the texture that will serve as the color attachment for the framebuffer.
    glGenTextures(1, &State->MultiSampledColorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, State->MultiSampledColorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA, GL_RGBA, WindowWidth, WindowHeight, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    //

    // Build the texture that will serve as the depth attachment for the framebuffer.
    glGenTextures(1, &State->MultiSampledDepthTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, State->MultiSampledDepthTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA, GL_DEPTH_COMPONENT32, WindowWidth, WindowHeight, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    //

    glBindFramebuffer(GL_FRAMEBUFFER, State->MultiSampledFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, State->MultiSampledColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, State->MultiSampledDepthTexture, 0);

    GLenum MultiSampledFramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    Assert(MultiSampledFramebufferStatus == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //

    // Setup single-sampled FBO
    glGenFramebuffers(1, &State->SingleSampledFBO);

    // Build the texture that will serve as the color attachment for the framebuffer.
    glGenTextures(1, &State->SingleSampledColorTexture);
    glBindTexture(GL_TEXTURE_2D, State->SingleSampledColorTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WindowWidth, WindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    //

    // Build the texture that will serve as the depth attachment for the framebuffer.
    glGenTextures(1, &State->SingleSampledDepthTexture);
    glBindTexture(GL_TEXTURE_2D, State->SingleSampledDepthTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    //

    glBindFramebuffer(GL_FRAMEBUFFER, State->SingleSampledFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, State->SingleSampledColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, State->SingleSampledDepthTexture, 0);

    GLenum SingleSampledFramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    Assert(SingleSampledFramebufferStatus == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
}

internal void
OpenGLOnWindowResize(opengl_state *State, i32 WindowWidth, i32 WindowHeight)
{
    State->WindowWidth = WindowWidth;
    State->WindowHeight = WindowHeight;

    glDeleteFramebuffers(1, &State->MultiSampledFBO);
    glDeleteFramebuffers(1, &State->SingleSampledFBO);

    glDeleteTextures(1, &State->MultiSampledColorTexture);
    glDeleteTextures(1, &State->MultiSampledDepthTexture);

    glDeleteTextures(1, &State->SingleSampledColorTexture);
    glDeleteTextures(1, &State->SingleSampledDepthTexture);

    OpenGLInitFramebuffers(State, WindowWidth, WindowHeight);
}

internal void
OpenGLInitRenderer(opengl_state* State, i32 WindowWidth, i32 WindowHeight)
{
    State->Vendor = (char*)glGetString(GL_VENDOR);
    State->Renderer = (char*)glGetString(GL_RENDERER);
    State->Version = (char*)glGetString(GL_VERSION);
    State->ShadingLanguageVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    State->CascadeShadowMapSize = 4096;
    State->CascadeBounds[0] = vec2(-0.1f, -20.f);
    State->CascadeBounds[1] = vec2(-15.f, -50.f);
    State->CascadeBounds[2] = vec2(-40.f, -120.f);
    State->CascadeBounds[3] = vec2(-100.f, -320.f);

    OpenGLInitLine(State);
    OpenGLInitRectangle(State);
    OpenGLInitShaders(State);
    OpenGLInitFramebuffers(State, WindowWidth, WindowHeight);

    // Shadow Map Configuration
    glGenFramebuffers(1, &State->CascadeShadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, State->CascadeShadowMapFBO);

    glGenTextures(4, State->CascadeShadowMaps);

    // todo: use Array Texture
    for (u32 TextureIndex = 0; TextureIndex < ArrayCount(State->CascadeShadowMaps); ++TextureIndex)
    {
        glBindTexture(GL_TEXTURE_2D, State->CascadeShadowMaps[TextureIndex]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, State->CascadeShadowMapSize, State->CascadeShadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        vec4 BorderColor = vec4(1.f);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor.Elements);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, State->CascadeShadowMaps[0], 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum FramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    Assert(FramebufferStatus == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //

    glGenBuffers(1, &State->ShaderStateUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(opengl_shader_state), NULL, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, State->ShaderStateUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // todo: cleanup
    bitmap WhiteTexture = {};
    WhiteTexture.Width = 1;
    WhiteTexture.Height = 1;
    WhiteTexture.Channels = 4;
    u32* WhitePixel = PushType(&State->Arena, u32);
    *WhitePixel = 0xFFFFFFFF;
    WhiteTexture.Pixels = WhitePixel;

    OpenGLAddTexture(State, 0, &WhiteTexture);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_MULTISAMPLE);

    // todo: use GL_ZERO_TO_ONE?
    glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
}

internal void
OpenGLPrepareScene(opengl_state *State, render_commands *Commands)
{
    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
            case RenderCommand_AddMesh:
            {
                render_command_add_mesh *Command = (render_command_add_mesh *) Entry;

                OpenGLAddMeshBuffer(
                    State, Command->MeshId, Command->VertexCount,
                    Command->Positions, Command->Normals, Command->Tangents, Command->Bitangents, Command->TextureCoords, Command->Weights, Command->JointIndices,
                    Command->IndexCount, Command->Indices, Command->MaxInstanceCount
                );

                break;
            }
            case RenderCommand_AddTexture:
            {
                render_command_add_texture *Command = (render_command_add_texture *)Entry;

                OpenGLAddTexture(State, Command->Id, Command->Bitmap);

                break;
            }
            case RenderCommand_AddSkinningBuffer:
            {
                render_command_add_skinning_buffer *Command = (render_command_add_skinning_buffer *) Entry;

                OpenGLAddSkinningBuffer(State, Command->SkinningBufferId, Command->SkinningMatrixCount);

                break;
            }
            /*default:
            {
                Assert(!"Render command is not supported");
            }*/
        }

        BaseAddress += Entry->Size;
    }
}

internal void
OpenGLRenderScene(opengl_state *State, render_commands *Commands, opengl_render_options *Options)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_CLAMP);
    //glDisable(GL_POLYGON_OFFSET_POINT);
    //glDisable(GL_POLYGON_OFFSET_LINE);
    //glDisable(GL_POLYGON_OFFSET_FILL);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    if (Options->RenderShadowMap)
    {
        glViewport(0, 0, State->CascadeShadowMapSize, State->CascadeShadowMapSize);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, State->CascadeShadowMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, State->CascadeShadowMaps[Options->CascadeIndex], 0);
        glEnable(GL_DEPTH_CLAMP);
        //glEnable(GL_POLYGON_OFFSET_POINT);
        //glEnable(GL_POLYGON_OFFSET_LINE);
        //glEnable(GL_POLYGON_OFFSET_FILL);
        //glPolygonOffset(1.f, 1.f);
        //glCullFace(GL_FRONT);
        //glDisable(GL_CULL_FACE);

        mat4 TransposeLightProjection = Transpose(Options->CascadeProjection);
        mat4 TransposeLightView = Transpose(Options->CascadeView);

        glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, Projection), sizeof(mat4), &TransposeLightProjection);
        glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, View), sizeof(mat4), &TransposeLightView);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    // todo: move commands to buckets (based on shader?)
    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header*)((u8*)Commands->RenderCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
            case RenderCommand_SetViewport:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_viewport *Command = (render_command_set_viewport *) Entry;

                    glViewport(Command->x, Command->y, Command->Width, Command->Height);
                }

                break;
            }
            case RenderCommand_SetOrthographicProjection:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_orthographic_projection *Command = (render_command_set_orthographic_projection *) Entry;

                    mat4 Projection = Orthographic(Command->Left, Command->Right, Command->Bottom, Command->Top, Command->Near, Command->Far);
                    mat4 TransposeProjection = Transpose(Projection);

                    glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                    glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, Projection), sizeof(mat4), &TransposeProjection);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                }

                break;
            }
            case RenderCommand_SetPerspectiveProjection:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_perspective_projection *Command = (render_command_set_perspective_projection *) Entry;

                    mat4 Projection = Perspective(Command->FovY, Command->Aspect, Command->Near, Command->Far);
                    mat4 TransposeProjection = Transpose(Projection);

                    glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                    glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, Projection), sizeof(mat4), &TransposeProjection);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                }

                break;
            }
            case RenderCommand_SetCamera:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_camera *Command = (render_command_set_camera *) Entry;

                    vec3 Target = Command->Position + Command->Direction;
                    vec3 Direction = -Command->Direction;

                    mat4 View = LookAt(Command->Position, Target, Command->Up);
                    mat4 TransposeView = Transpose(View);

                    glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                    glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, View), sizeof(mat4), &TransposeView);
                    glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, CameraPosition), sizeof(vec3), &Command->Position);
                    glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, CameraDirection), sizeof(vec3), &Direction);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                }

                break;
            }
            case RenderCommand_SetDirectionalLight:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_directional_light *Command = (render_command_set_directional_light *) Entry;

                    for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
                    {
                        opengl_shader *Shader = State->Shaders + ShaderIndex;

                        glUseProgram(Shader->Program);
                        glUniform3f(Shader->DirectionalLightDirectionUniformLocation, Command->Light.Direction.x, Command->Light.Direction.y, Command->Light.Direction.z);
                        glUniform3f(Shader->DirectionalLightColorUniformLocation, Command->Light.Color.r, Command->Light.Color.g, Command->Light.Color.b);
                        glUseProgram(0);
                    }
                }

                break;
            }
            case RenderCommand_SetPointLights:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_point_lights *Command = (render_command_set_point_lights *) Entry;

                    for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
                    {
                        opengl_shader *Shader = State->Shaders + ShaderIndex;

                        glUseProgram(Shader->Program);

                        glUniform1i(Shader->PointLightCountUniformLocation, Command->PointLightCount);

                        for (u32 PointLightIndex = 0; PointLightIndex < Command->PointLightCount; ++PointLightIndex)
                        {
                            point_light *PointLight = Command->PointLights + PointLightIndex;

                            char PositionUniformName[64];
                            FormatString(PositionUniformName, "u_PointLights[%d].Position", PointLightIndex);
                            GLint PositionUniformLocation = glGetUniformLocation(Shader->Program, PositionUniformName);

                            char ColorUniformName[64];
                            FormatString(ColorUniformName, "u_PointLights[%d].Color", PointLightIndex);
                            GLint ColorUniformLocation = glGetUniformLocation(Shader->Program, ColorUniformName);

                            char AttenuationConstantUniformName[64];
                            FormatString(AttenuationConstantUniformName, "u_PointLights[%d].Attenuation.Constant", PointLightIndex);
                            GLint AttenuationConstantUniformLocation = glGetUniformLocation(Shader->Program, AttenuationConstantUniformName);

                            char AttenuationLinearUniformName[64];
                            FormatString(AttenuationLinearUniformName, "u_PointLights[%d].Attenuation.Linear", PointLightIndex);
                            GLint AttenuationLinearUniformLocation = glGetUniformLocation(Shader->Program, AttenuationLinearUniformName);

                            char AttenuationQuadraticUniformName[64];
                            FormatString(AttenuationQuadraticUniformName, "u_PointLights[%d].Attenuation.Quadratic", PointLightIndex);
                            GLint AttenuationQuadraticUniformLocation = glGetUniformLocation(Shader->Program, AttenuationQuadraticUniformName);

                            glUniform3f(PositionUniformLocation, PointLight->Position.x, PointLight->Position.y, PointLight->Position.z);
                            glUniform3f(ColorUniformLocation, PointLight->Color.r, PointLight->Color.g, PointLight->Color.b);
                            glUniform1f(AttenuationConstantUniformLocation, PointLight->Attenuation.Constant);
                            glUniform1f(AttenuationLinearUniformLocation, PointLight->Attenuation.Linear);
                            glUniform1f(AttenuationQuadraticUniformLocation, PointLight->Attenuation.Quadratic);
                        }

                        glUseProgram(0);
                    }
                }
                
                break;
            }
            case RenderCommand_SetTime:
            {
                render_command_set_time *Command = (render_command_set_time *) Entry;

                glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, Time), sizeof(f32), &Command->Time);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                break;
            }
            // Actual render commands (prepass-safe)
            case RenderCommand_Clear:
            {
                render_command_clear *Command = (render_command_clear *)Entry;

                glClearColor(Command->Color.x, Command->Color.y, Command->Color.z, Command->Color.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                break;
            }
            case RenderCommand_DrawLine:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_line *Command = (render_command_draw_line *) Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                    glLineWidth(Command->Thickness);

                    glBindVertexArray(State->LineVAO);
                    glUseProgram(Shader->Program);
                    {
                        mat4 T = Translate(Command->Start);
                        mat4 S = Scale(Command->End - Command->Start);
                        mat4 Model = T * S;

                        glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *) Model.Elements);
                        glUniform4f(Shader->ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
                    }

                    glDrawArrays(GL_LINES, 0, 2);

                    glUseProgram(0);
                    glBindVertexArray(0);

                    glLineWidth(1.f);
                }

                break;
            }
            case RenderCommand_DrawRectangle:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_rectangle *Command = (render_command_draw_rectangle *) Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                    glBindVertexArray(State->RectangleVAO);
                    glUseProgram(Shader->Program);

                    mat4 Model = Transform(Command->Transform);
                    mat4 View = mat4(1.f);

                    glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                    glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, View), sizeof(mat4), &View);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);

                    glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *) Model.Elements);
                    glUniform4f(Shader->ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);

                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                    glUseProgram(0);
                    glBindVertexArray(0);
                }

                break;
            }
            case RenderCommand_DrawTextLine:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_text_line *Command = (render_command_draw_text_line *) Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_TEXT_SHADER_ID);

                    glBindVertexArray(State->RectangleVAO);
                    glUseProgram(Shader->Program);
                    glDisable(GL_DEPTH_TEST);

                    mat4 View = mat4(1.f);
                    mat4 Projection = Command->Projection;

                    font *Font = Command->Font;

                    glUniformMatrix4fv(glGetUniformLocation(Shader->Program, "u_TextView"), 1, GL_TRUE, (f32 *) View.Elements);
                    glUniformMatrix4fv(glGetUniformLocation(Shader->Program, "u_TextProjection"), 1, GL_TRUE, (f32 *) Projection.Elements);

                    glUniform4f(Shader->ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);

                    opengl_texture *Texture = OpenGLGetTexture(State, Font->TextureId);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, Texture->Handle);

                    f32 AtX = Command->Position.x;
                    f32 AtY = Command->Position.y;
                    f32 TextScale = Command->Scale;

                    // todo:
                    f32 PixelsPerUnit = 160.f;
                    f32 UnitsPerPixel = 1.f / PixelsPerUnit;
                    //

                    vec2 TextureAtlasSize = vec2((f32) Font->TextureAtlas.Width, (f32) Font->TextureAtlas.Height);

                    for (wchar *At = Command->Text; *At; ++At)
                    {
                        wchar Character = *At;
                        wchar NextCharacter = *(At + 1);

                        glyph *GlyphInfo = GetCharacterGlyph(Font, Character);

                        vec2 Position = vec2(AtX, AtY);
                        // todo: maybe I should do it inside assets builder?
                        vec2 SpriteSize = GlyphInfo->SpriteSize / TextureAtlasSize;
                        vec2 TextureOffset = GlyphInfo->UV;

                        vec2 Size = GlyphInfo->CharacterSize * TextScale * UnitsPerPixel;
                        vec2 Alignment = (GlyphInfo->Alignment) * TextScale * UnitsPerPixel;

                        // todo: Z coordinate
                        vec3 Translation = vec3(Position + Alignment, 0.f);
                        vec3 Scale = vec3(Size, 0.f);
                        quat Rotation = quat(0.f);

                        mat4 Model = Transform(CreateTransform(Translation, Scale, Rotation));

                        glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *) Model.Elements);
                        glUniform2f(glGetUniformLocation(Shader->Program, "u_SpriteSize"), SpriteSize.x, SpriteSize.y);
                        glUniform2f(glGetUniformLocation(Shader->Program, "u_TextureOffset"), TextureOffset.x, TextureOffset.y);

                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                        f32 HorizontalAdvance = GetHorizontalAdvanceForPair(Font, Character, NextCharacter);
                        AtX += HorizontalAdvance * TextScale * UnitsPerPixel;
                    }

                    glEnable(GL_DEPTH_TEST);
                    glUseProgram(0);
                    glBindVertexArray(0);
                }

                break;
            }
            case RenderCommand_DrawGround:
            {
                if (!Options->RenderShadowMap)
                {
                    // http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
                    // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders
                    render_command_draw_ground *Command = (render_command_draw_ground *) Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_GROUND_SHADER_ID);

                    glBindVertexArray(State->RectangleVAO);
                    glUseProgram(Shader->Program);

                    OpenGLBlinnPhongShading(State, Options, Shader, 0);

                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                    glUseProgram(0);
                    glBindVertexArray(0);
                }

                break;
            }
            case RenderCommand_DrawMesh:
            {
                render_command_draw_mesh *Command = (render_command_draw_mesh *)Entry;

                if ((Options->RenderShadowMap && Command->Material.CastShadow) || !Options->RenderShadowMap)
                {
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
                            glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *) Model.Elements);

                            OpenGLBlinnPhongShading(State, Options, Shader, Command->Material.MeshMaterial);

                            break;
                        }
                        case MaterialType_Unlit:
                        {
                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                            glUseProgram(Shader->Program);

                            mat4 Model = Transform(Command->Transform);
                            material Material = Command->Material;

                            glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *) Model.Elements);
                            glUniform4f(Shader->ColorUniformLocation, Material.Color.r, Material.Color.g, Material.Color.b, Material.Color.a);

                            break;
                        }
                        default:
                        {
                            Assert(!"Invalid material type");
                            break;
                        }
                    }

                    if (Command->Material.Wireframe)
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    }

                    glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);

                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                    glUseProgram(0);
                    glBindVertexArray(0);
                }

                break;
            }
            case RenderCommand_DrawSkinnedMesh:
            {
                render_command_draw_skinned_mesh *Command = (render_command_draw_skinned_mesh *) Entry;

                if ((Options->RenderShadowMap && Command->Material.CastShadow) || !Options->RenderShadowMap)
                {
                    opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);
                    opengl_skinning_buffer *SkinningBuffer = OpenGLGetSkinningBuffer(State, Command->SkinningBufferId);

                    glBindVertexArray(MeshBuffer->VAO);

                    switch (Command->Material.Type)
                    {
                        // todo: organize materials (https://threejs.org/docs/#api/en/materials/MeshPhongMaterial)
                        case MaterialType_BlinnPhong:
                        {
                            mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SKINNED_PHONG_SHADING_SHADER_ID);

                            glUseProgram(Shader->Program);

                            glBindBuffer(GL_TEXTURE_BUFFER, SkinningBuffer->SkinningTBO);
                            glBufferSubData(GL_TEXTURE_BUFFER, 0, Command->SkinningMatrixCount * sizeof(mat4), Command->SkinningMatrices);
                            glBindBuffer(GL_TEXTURE_BUFFER, 0);

                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_BUFFER, SkinningBuffer->SkinningTBOTexture);
                            glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, SkinningBuffer->SkinningTBO);

                            glUniform1i(Shader->SkinningMatricesSamplerUniformLocation, 0);

                            OpenGLBlinnPhongShading(State, Options, Shader, Command->Material.MeshMaterial);

                            break;
                        }
                        default:
                        {
                            Assert(!"Not Implemented");
                            break;
                        }
                    }

                    /*GLint PrevPolygonMode[2];
                    glGetIntegerv(GL_POLYGON_MODE, PrevPolygonMode);
                    glPolygonMode(GL_FRONT_AND_BACK, Command->Material.IsWireframe ? GL_LINE : GL_FILL);*/

                    glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);

                    //glPolygonMode(GL_FRONT_AND_BACK, PrevPolygonMode[0]);

                    glUseProgram(0);
                    glBindVertexArray(0);
                }

                break;
            }
            case RenderCommand_DrawMeshInstanced:
            {
                render_command_draw_mesh_instanced *Command = (render_command_draw_mesh_instanced *)Entry;

                if ((Options->RenderShadowMap && Command->Material.CastShadow) || !Options->RenderShadowMap)
                {
                    opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);

                    glBindVertexArray(MeshBuffer->VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, MeshBuffer->VBO);
                    glBufferSubData(GL_ARRAY_BUFFER, MeshBuffer->BufferSize, Command->InstanceCount * sizeof(render_instance), Command->Instances);

                    switch (Command->Material.Type)
                    {
                        case MaterialType_BlinnPhong:
                        {
                            mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_INSTANCED_PHONG_SHADING_SHADER_ID);

                            glUseProgram(Shader->Program);

                            OpenGLBlinnPhongShading(State, Options, Shader, Command->Material.MeshMaterial);

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

                            glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *) Model.Elements);
                            glUniform4f(Shader->ColorUniformLocation, Material.Color.r, Material.Color.g, Material.Color.b, Material.Color.a);
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
                   /* GLint PrevPolygonMode[2];
                    glGetIntegerv(GL_POLYGON_MODE, PrevPolygonMode);
                    glPolygonMode(GL_FRONT_AND_BACK, Command->Material.IsWireframe ? GL_LINE : GL_FILL);*/

                    glDrawElementsInstanced(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0, Command->InstanceCount);

                    //glPolygonMode(GL_FRONT_AND_BACK, PrevPolygonMode[0]);

                    glUseProgram(0);
                    glBindVertexArray(0);
                }

                break;
            }
           /* default:
            {
                Assert(!"Render command is not supported");
            }*/
        }

        BaseAddress += Entry->Size;
    }
}

internal void
OpenGLProcessRenderCommands(opengl_state *State, render_commands *Commands)
{
#if WIN32_RELOADABLE_SHADERS
    for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
    {
        opengl_shader *Shader = State->Shaders + ShaderIndex;

        b32 ShouldReload = false;

        for (u32 FileIndex = 0; FileIndex < ArrayCount(Shader->CommonShaders); ++FileIndex)
        {
            win32_shader_file *CommonShader = Shader->CommonShaders + FileIndex;

            FILETIME NewCommonShaderWriteTime = Win32GetLastWriteTime(CommonShader->FileName);
            if (CompareFileTime(&NewCommonShaderWriteTime, &CommonShader->LastWriteTime) != 0)
            {
                CommonShader->LastWriteTime = NewCommonShaderWriteTime;
                ShouldReload = true;
            }
        }

        FILETIME NewVertexShaderWriteTime = Win32GetLastWriteTime(Shader->VertexShader.FileName);
        if (CompareFileTime(&NewVertexShaderWriteTime, &Shader->VertexShader.LastWriteTime) != 0)
        {
            Shader->VertexShader.LastWriteTime = NewVertexShaderWriteTime;
            ShouldReload = true;
        }

        FILETIME NewFragmentShaderWriteTime = Win32GetLastWriteTime(Shader->FragmentShader.FileName);
        if (CompareFileTime(&NewFragmentShaderWriteTime, &Shader->FragmentShader.LastWriteTime) != 0)
        {
            Shader->FragmentShader.LastWriteTime = NewFragmentShaderWriteTime;
            ShouldReload = true;
        }

        if (ShouldReload)
        {
            OpenGLReloadShader(State, Shader->Id);
        }
    }
#endif

    render_commands_settings *RenderSettings = &Commands->Settings;

    if (State->WindowWidth != RenderSettings->WindowWidth || RenderSettings->WindowHeight != RenderSettings->WindowHeight)
    {
        OpenGLOnWindowResize(State, RenderSettings->WindowWidth, RenderSettings->WindowHeight);
    }

    OpenGLPrepareScene(State, Commands);

    game_camera *Camera = RenderSettings->Camera;

    f32 FocalLength = Camera->FocalLength;
    f32 AspectRatio = Camera->AspectRatio;

    mat4 CameraM = GetCameraTransform(Camera);
    mat4 CameraMInv = Inverse(CameraM);

    vec3 LightPosition = vec3(0.f);
    vec3 LightDirection = Normalize(RenderSettings->DirectionalLight->Direction);
    vec3 LightUp = vec3(0.f, 1.f, 0.f);

    mat4 LightM = LookAt(LightPosition, LightPosition + LightDirection, LightUp);

#if 1
    for (u32 CascadeIndex = 0; CascadeIndex < 4; ++CascadeIndex)
    {
        vec2 CascadeBounds = State->CascadeBounds[CascadeIndex];

        f32 Near = CascadeBounds.x;
        f32 Far = CascadeBounds.y;

        vec4 CameraSpaceFrustrumCorners[8] = {
            vec4(-Near * AspectRatio / FocalLength, -Near / FocalLength, Near, 1.f),
            vec4(Near * AspectRatio / FocalLength, -Near / FocalLength, Near, 1.f),
            vec4(-Near * AspectRatio / FocalLength, Near / FocalLength, Near, 1.f),
            vec4(Near * AspectRatio / FocalLength, Near / FocalLength, Near, 1.f),

            vec4(-Far * AspectRatio / FocalLength, -Far / FocalLength, Far, 1.f),
            vec4(Far * AspectRatio / FocalLength, -Far / FocalLength, Far, 1.f),
            vec4(-Far * AspectRatio / FocalLength, Far / FocalLength, Far, 1.f),
            vec4(Far * AspectRatio / FocalLength, Far / FocalLength, Far, 1.f),
        };

        f32 xMin = F32_MAX;
        f32 xMax = -F32_MAX;
        f32 yMin = F32_MAX;
        f32 yMax = -F32_MAX;
        f32 zMin = F32_MAX;
        f32 zMax = -F32_MAX;

        for (u32 CornerIndex = 0; CornerIndex < ArrayCount(CameraSpaceFrustrumCorners); ++CornerIndex)
        {
            vec4 LightSpaceFrustrumCorner = LightM * CameraMInv * CameraSpaceFrustrumCorners[CornerIndex];

            xMin = Min(xMin, LightSpaceFrustrumCorner.x);
            xMax = Max(xMax, LightSpaceFrustrumCorner.x);
            yMin = Min(yMin, LightSpaceFrustrumCorner.y);
            yMax = Max(yMax, LightSpaceFrustrumCorner.y);
            zMin = Min(zMin, LightSpaceFrustrumCorner.z);
            zMax = Max(zMax, LightSpaceFrustrumCorner.z);
        }

        // todo: could be calculated one time and saved
        i32 d = Ceil(Max(Magnitude(CameraSpaceFrustrumCorners[1] - CameraSpaceFrustrumCorners[6]), Magnitude(CameraSpaceFrustrumCorners[5] - CameraSpaceFrustrumCorners[6])));

        mat4 CascadeProjection = mat4(
            2.f / d, 0.f, 0.f, 0.f,
            0.f, 2.f / d, 0.f, 0.f,
            0.f, 0.f, -1.f / (zMax - zMin), 0.f,
            0.f, 0.f, 0.f, 1.f
        );

        f32 T = (f32)d / (f32)State->CascadeShadowMapSize;

        vec3 LightPosition = vec3(Floor((xMax + xMin) / (2.f * T)) * T, Floor((yMax + yMin) / (2.f * T)) * T, zMin);

        mat4 CascadeView = mat4(
            vec4(LightM[0].xyz, -LightPosition.x),
            vec4(LightM[1].xyz, -LightPosition.y),
            vec4(LightM[2].xyz, -LightPosition.z),
            vec4(0.f, 0.f, 0.f, 1.f)
        );

        State->CascadeViewProjection[CascadeIndex] = CascadeProjection * CascadeView;

        opengl_render_options RenderOptions = {};
        RenderOptions.RenderShadowMap = true;
        RenderOptions.CascadeIndex = CascadeIndex;
        RenderOptions.CascadeProjection = CascadeProjection;
        RenderOptions.CascadeView = CascadeView;
        OpenGLRenderScene(State, Commands, &RenderOptions);
    }
#endif

    opengl_render_options RenderOptions = {};
    RenderOptions.ShowCascades = RenderSettings->ShowCascades;
    OpenGLRenderScene(State, Commands, &RenderOptions);

#if 0
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_FRAMEBUFFER_SHADER_ID);

    glBindVertexArray(State->RectangleVAO);
    glUseProgram(Shader->Program);

    {
        GLint UniformLocation = glGetUniformLocation(Shader->Program, "u_zNear");
        glUniform1f(UniformLocation, State->CascadeBounds[0].x);
    }

    {
        GLint UniformLocation = glGetUniformLocation(Shader->Program, "u_zFar");
        glUniform1f(UniformLocation, State->CascadeBounds[3].y);
    }

    glViewport(512 * 0, 0, 512, 512);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, State->CascadeShadowMaps[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glViewport(512 * 1 + 10, 0, 512, 512);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, State->CascadeShadowMaps[1]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glViewport(512 * 2 + 20, 0, 512, 512);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, State->CascadeShadowMaps[2]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glViewport(512 * 3 + 30, 0, 512, 512);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, State->CascadeShadowMaps[3]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
}
