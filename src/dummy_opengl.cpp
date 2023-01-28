// https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions

internal GLuint
OpenGLCreateShader(GLenum Type, GLsizei Count, char **Source, bool32 CrashIfError = true)
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
OpenGLCreateProgram(opengl_create_program_params Params, bool32 CrashIfError = true)
{
    GLuint Program = glCreateProgram();
    glAttachShader(Program, Params.VertexShader);
    glAttachShader(Program, Params.FragmentShader);

    if (Params.GeometryShader)
    {
        glAttachShader(Program, Params.GeometryShader);
    }

    glLinkProgram(Program);

    glDeleteShader(Params.VertexShader);
    glDeleteShader(Params.FragmentShader);

    if (Params.GeometryShader)
    {
        glDeleteShader(Params.GeometryShader);
    }

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

inline void
OpenGLVertexAttribute(GLuint VAO, GLuint VBO, u32 AttributeIndex, u32 ElementCount, u32 Offset, u32 RelativeOffset, u32 Stride)
{
    glEnableVertexArrayAttrib(VAO, AttributeIndex);
    glVertexArrayAttribFormat(VAO, AttributeIndex, ElementCount, GL_FLOAT, GL_FALSE, RelativeOffset);
    glVertexArrayVertexBuffer(VAO, AttributeIndex, VBO, Offset, Stride);
    glVertexArrayAttribBinding(VAO, AttributeIndex, AttributeIndex);
}

inline void
OpenGLVertexAttributeInteger(GLuint VAO, GLuint VBO, u32 AttributeIndex, u32 ElementCount, u32 Offset, u32 RelativeOffset, u32 Stride)
{
    glEnableVertexArrayAttrib(VAO, AttributeIndex);
    glVertexArrayAttribIFormat(VAO, AttributeIndex, ElementCount, GL_UNSIGNED_INT, RelativeOffset);
    glVertexArrayVertexBuffer(VAO, AttributeIndex, VBO, Offset, Stride);
    glVertexArrayAttribBinding(VAO, AttributeIndex, AttributeIndex);
}

inline void
OpenGLInstanceAttribute(GLuint VAO, GLuint VBO, u32 AttributeIndex, u32 ElementCount, u32 Offset, u32 RelativeOffset, u32 Stride)
{
    glEnableVertexArrayAttrib(VAO, AttributeIndex);
    glVertexArrayAttribFormat(VAO, AttributeIndex, ElementCount, GL_FLOAT, GL_FALSE, RelativeOffset);
    glVertexArrayVertexBuffer(VAO, AttributeIndex, VBO, Offset, Stride);
    glVertexArrayAttribBinding(VAO, AttributeIndex, AttributeIndex);
    glVertexArrayBindingDivisor(VAO, AttributeIndex, 1);
}

internal void
OpenGLInitLine(opengl_state *State)
{
    vec3 LineVertices[] =
    {
        vec3(0.f, 0.f, 0.f),
        vec3(1.f, 1.f, 1.f),
    };

    glCreateVertexArrays(1, &State->LineVAO);

    GLuint LineVBO;
    glCreateBuffers(1, &LineVBO);
    glNamedBufferStorage(LineVBO, sizeof(LineVertices), LineVertices, 0);

    OpenGLVertexAttribute(State->LineVAO, LineVBO, 0, 3, 0, 0, sizeof(vec3));
}

internal void
OpenGLInitRectangle(opengl_state *State)
{
    f32 RectangleVertices[] = 
    {
        // potisions     // uvs
        -1.f, -1.f, 0.f, 0.f, 0.f,
        1.f, -1.f, 0.f,  1.f, 0.f,
        -1.f, 1.f, 0.f,  0.f, 1.f,
        1.f, 1.f, 0.f,   1.f, 1.f
    };

    GLsizei Stride = sizeof(vec3) + sizeof(vec2);

    glCreateVertexArrays(1, &State->RectangleVAO);

    GLuint RectangleVBO;
    glCreateBuffers(1, &RectangleVBO);
    glNamedBufferStorage(RectangleVBO, sizeof(RectangleVertices), RectangleVertices, 0);

    OpenGLVertexAttribute(State->RectangleVAO, RectangleVBO, 0, 3, 0, 0, Stride);
    OpenGLVertexAttribute(State->RectangleVAO, RectangleVBO, 1, 2, 0, sizeof(vec3), Stride);
}

internal void
OpenGLInitBox(opengl_state *State)
{
    vec3 BoxVertices[] = 
    {
#if 1
        vec3(-1.f, 2.f, -1.f),
        vec3(1.f, 2.f, -1.f),
        vec3(-1.f, 0.f, -1.f),
        vec3(1.f, 0.f, -1.f),
        vec3(1.f, 0.f, 1.f),
        vec3(1.f, 2.f, -1.f),
        vec3(1.f, 2.f, 1.f),
        vec3(-1.f, 2.f, -1.f),
        vec3(-1.f, 2.f, 1.f),
        vec3(-1.f, 0.f, -1.f),
        vec3(-1.f, 0.f, 1.f),
        vec3(1.f, 0.f, 1.f),
        vec3(-1.f, 2.f, 1.f),
        vec3(1.f, 2.f, 1.f)
#else
        vec3(-1.f, 1.f, -1.f),
        vec3(1.f, 1.f, -1.f),
        vec3(-1.f, -1.f, -1.f),
        vec3(1.f, -1.f, -1.f),
        vec3(1.f, -1.f, 1.f),
        vec3(1.f, 1.f, -1.f),
        vec3(1.f, 1.f, 1.f),
        vec3(-1.f, 1.f, -1.f),
        vec3(-1.f, 1.f, 1.f),
        vec3(-1.f, -1.f, -1.f),
        vec3(-1.f, -1.f, 1.f),
        vec3(1.f, -1.f, 1.f),
        vec3(-1.f, 1.f, 1.f),
        vec3(1.f, 1.f, 1.f)
#endif
    };

    glCreateVertexArrays(1, &State->BoxVAO);

    GLuint BoxVBO;
    glCreateBuffers(1, &BoxVBO);
    glNamedBufferStorage(BoxVBO, sizeof(BoxVertices), BoxVertices, 0);

    OpenGLVertexAttribute(State->BoxVAO, BoxVBO, 0, 3, 0, 0, sizeof(vec3));
}

internal void
OpenGLInitText(opengl_state *State)
{
    u32 MaxCharacterLength = 1024;

    glCreateVertexArrays(1, &State->TextVAO);
    glCreateBuffers(1, &State->TextVBO);
    glNamedBufferStorage(State->TextVBO, MaxCharacterLength * sizeof(opengl_character_point), 0, GL_DYNAMIC_STORAGE_BIT);

    OpenGLVertexAttribute(State->TextVAO, State->TextVBO, 0, 3, 0, StructOffset(opengl_character_point, Position), sizeof(opengl_character_point));
    OpenGLVertexAttribute(State->TextVAO, State->TextVBO, 1, 2, 0, StructOffset(opengl_character_point, Size), sizeof(opengl_character_point));
    OpenGLVertexAttribute(State->TextVAO, State->TextVBO, 2, 2, 0, StructOffset(opengl_character_point, SpriteSize), sizeof(opengl_character_point));
    OpenGLVertexAttribute(State->TextVAO, State->TextVBO, 3, 2, 0, StructOffset(opengl_character_point, SpriteOffset), sizeof(opengl_character_point));
}

internal void
OpenGLInitParticles(opengl_state *State)
{
    u32 MaxParticleCount = 4096;

    glCreateVertexArrays(1, &State->ParticleVAO);
    glCreateBuffers(1, &State->ParticleVBO);
    glNamedBufferStorage(State->ParticleVBO, MaxParticleCount * sizeof(opengl_particle), 0, GL_DYNAMIC_STORAGE_BIT);

    OpenGLVertexAttribute(State->ParticleVAO, State->ParticleVBO, 0, 3, 0, StructOffset(opengl_particle, Position), sizeof(opengl_particle));
    OpenGLVertexAttribute(State->ParticleVAO, State->ParticleVBO, 1, 2, 0, StructOffset(opengl_particle, Size), sizeof(opengl_particle));
    OpenGLVertexAttribute(State->ParticleVAO, State->ParticleVBO, 2, 4, 0, StructOffset(opengl_particle, Color), sizeof(opengl_particle));
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
    ivec4 *JointIndices
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
        Size += VertexCount * sizeof(ivec4);
    }

    return Size;
}

internal void
OpenGLAddSkinningBuffer(opengl_state *State, u32 BufferId, u32 SkinningMatrixCount)
{
    opengl_skinning_buffer *SkinningBuffer = State->SkinningBuffers + State->CurrentSkinningBufferCount++;
    SkinningBuffer->Id = BufferId;

    glCreateBuffers(1, &SkinningBuffer->SkinningTBO);
    glNamedBufferData(SkinningBuffer->SkinningTBO, SkinningMatrixCount * sizeof(mat4), 0, GL_STREAM_DRAW);

    glCreateTextures(GL_TEXTURE_BUFFER, 1, &SkinningBuffer->SkinningTBOTexture);
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
    ivec4 *JointIndices,
    u32 IndexCount, 
    u32 *Indices
)
{
    Assert(State->CurrentMeshBufferCount < OPENGL_MAX_MESH_BUFFER_COUNT);

    GLuint VAO;
    GLuint VertexBuffer;
    GLuint InstanceBuffer;
    GLuint IndexBuffer;

    u32 BufferSize = GetMeshVerticesSize(VertexCount, Positions, Normals, Tangents, Bitangents, TextureCoords, Weights, JointIndices);

    glCreateVertexArrays(1, &VAO);

    glCreateBuffers(1, &VertexBuffer);
    glNamedBufferData(VertexBuffer, BufferSize, 0, GL_STATIC_DRAW);

    // per-vertex attributes
    u32 Offset = 0;
    if (Positions)
    {
        OpenGLVertexAttribute(VAO, VertexBuffer, 0, 3, Offset, 0, sizeof(vec3));

        glNamedBufferSubData(VertexBuffer, Offset, VertexCount * sizeof(vec3), Positions);

        Offset += VertexCount * sizeof(vec3);
    }

    if (Normals)
    {
        OpenGLVertexAttribute(VAO, VertexBuffer, 1, 3, Offset, 0, sizeof(vec3));

        glNamedBufferSubData(VertexBuffer, Offset, VertexCount * sizeof(vec3), Normals);

        Offset += VertexCount * sizeof(vec3);
    }

    if (Tangents)
    {
        OpenGLVertexAttribute(VAO, VertexBuffer, 2, 3, Offset, 0, sizeof(vec3));

        glNamedBufferSubData(VertexBuffer, Offset, VertexCount * sizeof(vec3), Tangents);

        Offset += VertexCount * sizeof(vec3);
    }

    if (Bitangents)
    {
        OpenGLVertexAttribute(VAO, VertexBuffer, 3, 3, Offset, 0, sizeof(vec3));

        glNamedBufferSubData(VertexBuffer, Offset, VertexCount * sizeof(vec3), Bitangents);
        Offset += VertexCount * sizeof(vec3);
    }

    if (TextureCoords)
    {
        OpenGLVertexAttribute(VAO, VertexBuffer, 4, 2, Offset, 0, sizeof(vec2));

        glNamedBufferSubData(VertexBuffer, Offset, VertexCount * sizeof(vec2), TextureCoords);
        Offset += VertexCount * sizeof(vec2);
    }

    if (Weights)
    {
        OpenGLVertexAttribute(VAO, VertexBuffer, 5, 4, Offset, 0, sizeof(vec4));

        glNamedBufferSubData(VertexBuffer, Offset, VertexCount * sizeof(vec4), Weights);
        Offset += VertexCount * sizeof(vec4);
    }

    if (JointIndices)
    {
        OpenGLVertexAttributeInteger(VAO, VertexBuffer, 6, 4, Offset, 0, sizeof(ivec4));

        glNamedBufferSubData(VertexBuffer, Offset, VertexCount * sizeof(ivec4), JointIndices);
        Offset += VertexCount * sizeof(ivec4);
    }

    glCreateBuffers(1, &InstanceBuffer);
    glNamedBufferData(InstanceBuffer, 0, 0, GL_STREAM_DRAW);

    // per-instance attributes
    OpenGLInstanceAttribute(VAO, InstanceBuffer, 7, 4, 0, (StructOffset(mesh_instance, Model) + 0 * sizeof(vec4)), sizeof(mesh_instance));
    OpenGLInstanceAttribute(VAO, InstanceBuffer, 8, 4, 0, (StructOffset(mesh_instance, Model) + 1 * sizeof(vec4)), sizeof(mesh_instance));
    OpenGLInstanceAttribute(VAO, InstanceBuffer, 9, 4, 0, (StructOffset(mesh_instance, Model) + 2 * sizeof(vec4)), sizeof(mesh_instance));
    OpenGLInstanceAttribute(VAO, InstanceBuffer, 10, 4, 0, (StructOffset(mesh_instance, Model) + 3 * sizeof(vec4)), sizeof(mesh_instance));
    OpenGLInstanceAttribute(VAO, InstanceBuffer, 11, 3, 0, StructOffset(mesh_instance, Color), sizeof(mesh_instance));

    glCreateBuffers(1, &IndexBuffer);
    glNamedBufferStorage(IndexBuffer, IndexCount * sizeof(u32), Indices, 0);

    glVertexArrayElementBuffer(VAO, IndexBuffer);

    opengl_mesh_buffer *MeshBuffer = State->MeshBuffers + State->CurrentMeshBufferCount++;
    MeshBuffer->Id = MeshId;
    MeshBuffer->VertexCount = VertexCount;
    MeshBuffer->IndexCount = IndexCount;
    MeshBuffer->VAO = VAO;
    MeshBuffer->VertexBuffer = VertexBuffer;
    MeshBuffer->InstanceBuffer = InstanceBuffer;
    MeshBuffer->IndexBuffer = IndexBuffer;

    MeshBuffer->BufferSize = BufferSize;
    MeshBuffer->InstanceCount = 0;
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

inline GLint
OpenGLGetTextureInternalFormat(bitmap *Bitmap)
{
    if (Bitmap->Channels == 1) return GL_R8;
    if (Bitmap->Channels == 2) return GL_RG8;
    if (Bitmap->Channels == 3) return GL_RGB8;
    if (Bitmap->Channels == 4) return GL_RGBA8;

    Assert(!"Invalid number of channels");

    return -1;
}

internal void
OpenGLAddTexture(opengl_state *State, u32 Id, bitmap *Bitmap)
{
    Assert(State->CurrentTextureCount < OPENGL_MAX_TEXTURE_COUNT);

    GLuint TextureHandle;

    glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);

    glTextureParameteri(TextureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(TextureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(TextureHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(TextureHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLint Format = OpenGLGetTextureFormat(Bitmap);
    GLint InternalFormat = OpenGLGetTextureInternalFormat(Bitmap);

    glTextureStorage2D(TextureHandle, 1, InternalFormat, Bitmap->Width, Bitmap->Height);
    glTextureSubImage2D(TextureHandle, 0, 0, 0, Bitmap->Width, Bitmap->Height, Format, GL_UNSIGNED_BYTE, Bitmap->Pixels);
    glGenerateTextureMipmap(TextureHandle);

    opengl_texture *Texture = State->Textures + State->CurrentTextureCount++;
    Texture->Id = Id;
    Texture->Handle = TextureHandle;
}

// todo: https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#ideal-way-of-retrieving-all-uniform-names
internal void
OpenGLLoadShaderUniforms(opengl_shader *Shader)
{
    GLuint Program = Shader->Program;

    Shader->ModelUniform = glGetUniformLocation(Program, "u_Model");
    Shader->ModeUniform = glGetUniformLocation(Program, "u_Mode");
    Shader->SkinningMatricesSamplerUniform = glGetUniformLocation(Program, "u_SkinningMatricesSampler");

    Shader->ColorUniform = glGetUniformLocation(Program, "u_Color");

    Shader->MaterialSpecularShininessUniform = glGetUniformLocation(Program, "u_Material.SpecularShininess");

    Shader->MaterialAmbientColorUniform = glGetUniformLocation(Program, "u_Material.AmbientColor");
    Shader->MaterialDiffuseColorUniform = glGetUniformLocation(Program, "u_Material.DiffuseColor");
    Shader->MaterialSpecularColorUniform = glGetUniformLocation(Program, "u_Material.SpecularColor");

    Shader->MaterialDiffuseMapUniform = glGetUniformLocation(Program, "u_Material.DiffuseMap");
    Shader->MaterialSpecularMapUniform = glGetUniformLocation(Program, "u_Material.SpecularMap");
    Shader->MaterialShininessMapUniform = glGetUniformLocation(Program, "u_Material.ShininessMap");
    Shader->MaterialNormalMapUniform = glGetUniformLocation(Program, "u_Material.NormalMap");

    Shader->MaterialHasDiffuseMapUniform = glGetUniformLocation(Program, "u_Material.HasDiffuseMap");
    Shader->MaterialHasSpecularMapUniform = glGetUniformLocation(Program, "u_Material.HasSpecularMap");
    Shader->MaterialHasShininessMapUniform = glGetUniformLocation(Program, "u_Material.HasShininessMap");
    Shader->MaterialHasNormalMapUniform = glGetUniformLocation(Program, "u_Material.HasNormalMap");

    Shader->DirectionalLightDirectionUniform = glGetUniformLocation(Program, "u_DirectionalLight.Direction");
    Shader->DirectionalLightColorUniform = glGetUniformLocation(Program, "u_DirectionalLight.Color");
    Shader->PointLightCountUniform = glGetUniformLocation(Program, "u_PointLightCount");

    Shader->CameraXAxisUniform = glGetUniformLocation(Program, "u_CameraXAxis");
    Shader->CameraYAxisUniform = glGetUniformLocation(Program, "u_CameraYAxis");

    Shader->ScreenTextureUniform = glGetUniformLocation(Program, "u_ScreenTexture");
}

inline char *
OpenGLPreprocessShader(char *ShaderSource, u32 InitialSize, memory_arena *Arena)
{
    u32 Size = InitialSize + 256;
    char *Result = PushString(Arena, Size);
    FormatString_(Result, Size, ShaderSource, 
        OPENGL_MAX_POINT_LIGHT_COUNT, 
        OPENGL_WORLD_SPACE_MODE, 
        OPENGL_SCREEN_SPACE_MODE, 
        OPENGL_MAX_JOINT_COUNT
    );

    return Result;
}

internal char **
OpenGLLoadShaderFile(opengl_state *State, u32 Id, const char *ShaderFileName, u32 Count, memory_arena *Arena)
{
    char **Sources = PushArray(Arena, Count, char *);

    for (u32 FileIndex = 0; FileIndex < OPENGL_COMMON_SHADER_COUNT; ++FileIndex)
    {
        read_file_result CommonShaderFile = State->Platform->ReadFile((char *) OpenGLCommonShaders[FileIndex], Arena, ReadText());
        Sources[FileIndex] = OpenGLPreprocessShader((char *) CommonShaderFile.Contents, CommonShaderFile.Size, Arena);
    }

    read_file_result ShaderFile = State->Platform->ReadFile((char *) ShaderFileName, Arena, ReadText());
    Sources[OPENGL_COMMON_SHADER_COUNT] = (char *) ShaderFile.Contents;

    return Sources;
}

internal void
OpenGLLoadShader(opengl_state *State, opengl_load_shader_params Params)
{
    opengl_shader *Shader = State->Shaders + State->CurrentShaderCount++;

    scoped_memory ScopedMemory(&State->Arena);

    u32 Count = OPENGL_COMMON_SHADER_COUNT + 1;
    char **VertexSource = OpenGLLoadShaderFile(State, Params.ShaderId, Params.VertexShaderFileName, Count, ScopedMemory.Arena);
    char **FragmentSource = OpenGLLoadShaderFile(State, Params.ShaderId, Params.FragmentShaderFileName, Count, ScopedMemory.Arena);

    opengl_create_program_params CreateProgramParams = {};
    CreateProgramParams.VertexShader = OpenGLCreateShader(GL_VERTEX_SHADER, Count, VertexSource);
    CreateProgramParams.FragmentShader = OpenGLCreateShader(GL_FRAGMENT_SHADER, Count, FragmentSource);

    if (Params.GeometryShaderFileName)
    {
        char **GeometrySource = OpenGLLoadShaderFile(State, Params.ShaderId, Params.GeometryShaderFileName, Count, ScopedMemory.Arena);
        CreateProgramParams.GeometryShader = OpenGLCreateShader(GL_GEOMETRY_SHADER, Count, GeometrySource);
    }

    GLuint Program = OpenGLCreateProgram(CreateProgramParams);

    Shader->Id = Params.ShaderId;
    Shader->Program = Program;

#if WIN32_RELOADABLE_SHADERS
    for (u32 FileIndex = 0; FileIndex < ArrayCount(Shader->CommonShaders); ++FileIndex)
    {
        win32_shader_file *CommonShader = Shader->CommonShaders + FileIndex;

        CopyString(OpenGLCommonShaders[FileIndex], CommonShader->FileName);
        CommonShader->LastWriteTime = Win32GetLastWriteTime(CommonShader->FileName);
    }

    CopyString(Params.VertexShaderFileName, Shader->VertexShader.FileName);
    Shader->VertexShader.LastWriteTime = Win32GetLastWriteTime(Shader->VertexShader.FileName);

    CopyString(Params.FragmentShaderFileName, Shader->FragmentShader.FileName);
    Shader->FragmentShader.LastWriteTime = Win32GetLastWriteTime(Shader->FragmentShader.FileName);

    if (Params.GeometryShaderFileName)
    {
        CopyString(Params.GeometryShaderFileName, Shader->GeometryShader.FileName);
        Shader->GeometryShader.LastWriteTime = Win32GetLastWriteTime(Shader->GeometryShader.FileName);
    }
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

    opengl_create_program_params CreateProgramParams = {};
    CreateProgramParams.VertexShader = OpenGLCreateShader(GL_VERTEX_SHADER, Count, VertexSource, false);
    CreateProgramParams.FragmentShader = OpenGLCreateShader(GL_FRAGMENT_SHADER, Count, FragmentSource, false);

    if (!StringEquals(Shader->GeometryShader.FileName, ""))
    {
        char **GeometrySource = OpenGLLoadShaderFile(State, Id, Shader->GeometryShader.FileName, Count, ScopedMemory.Arena);
        CreateProgramParams.GeometryShader = OpenGLCreateShader(GL_GEOMETRY_SHADER, Count, GeometrySource);
    }

    GLuint Program = 0;

    if (CreateProgramParams.VertexShader && CreateProgramParams.FragmentShader)
    {
        Program = OpenGLCreateProgram(CreateProgramParams, false);
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
    for (u32 ShaderIndex = 0; ShaderIndex < ArrayCount(OpenGLShaders); ++ShaderIndex)
    {
        OpenGLLoadShader(State, OpenGLShaders[ShaderIndex]);
    }
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

    glUniform1i(Shader->MaterialHasDiffuseMapUniform, false);
    glUniform1i(Shader->MaterialHasSpecularMapUniform, false);
    glUniform1i(Shader->MaterialHasShininessMapUniform, false);
    glUniform1i(Shader->MaterialHasNormalMapUniform, false);

    // todo: magic numbers
    opengl_texture *Texture = OpenGLGetTexture(State, 0);
    glBindTextureUnit(0, Texture->Handle);

    if (!Options->RenderShadowMap)
    {
        for (u32 CascadeIndex = 0; CascadeIndex < 4; ++CascadeIndex)
        {
            u32 TextureIndex = CascadeIndex + 16;

            // Cascasde Shadow Map
            char ShadowMapUniformName[64];
            FormatString(ShadowMapUniformName, "u_CascadeShadowMaps[%d]", CascadeIndex);
            GLint ShadowMapUniformLocation = glGetUniformLocation(Shader->Program, ShadowMapUniformName);

            glBindTextureUnit(TextureIndex, State->CascadeShadowMaps[CascadeIndex]);
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
        vec3 DefaultAmbient = vec3(0.2f);
        glUniform3f(Shader->MaterialAmbientColorUniform, DefaultAmbient.x, DefaultAmbient.y, DefaultAmbient.z);
        glUniform1f(Shader->MaterialSpecularShininessUniform, 1.f);

        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MeshMaterial->PropertyCount; ++MaterialPropertyIndex)
        {
            material_property *MaterialProperty = MeshMaterial->Properties + MaterialPropertyIndex;

            switch (MaterialProperty->Type)
            {
                case MaterialProperty_Float_Shininess:
                {
                    glUniform1f(Shader->MaterialSpecularShininessUniform, MaterialProperty->Value);
                    break;
                }
                case MaterialProperty_Color_Ambient:
                {
                    if (MaterialProperty->Color.x == 0.f && MaterialProperty->Color.y == 0.f && MaterialProperty->Color.z == 0.f)
                    {
                        MaterialProperty->Color.rgb = DefaultAmbient;
                    }

                    glUniform3f(
                        Shader->MaterialAmbientColorUniform,
                        MaterialProperty->Color.r,
                        MaterialProperty->Color.g,
                        MaterialProperty->Color.b
                    );
                    break;
                }
                case MaterialProperty_Color_Diffuse:
                {
                    glUniform3f(
                        Shader->MaterialDiffuseColorUniform,
                        MaterialProperty->Color.r,
                        MaterialProperty->Color.g,
                        MaterialProperty->Color.b
                    );
                    break;
                }
                case MaterialProperty_Color_Specular:
                {
                    glUniform3f(
                        Shader->MaterialSpecularColorUniform,
                        MaterialProperty->Color.r,
                        MaterialProperty->Color.g,
                        MaterialProperty->Color.b
                    );
                    break;
                }
                case MaterialProperty_Texture_Diffuse:
                {
                    opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->TextureId);

                    glBindTextureUnit(MaterialPropertyIndex, Texture->Handle);

                    glUniform1i(Shader->MaterialHasDiffuseMapUniform, true);
                    glUniform1i(Shader->MaterialDiffuseMapUniform, MaterialPropertyIndex);
                    break;
                }
                case MaterialProperty_Texture_Specular:
                {
                    opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->TextureId);

                    glBindTextureUnit(MaterialPropertyIndex, Texture->Handle);

                    glUniform1i(Shader->MaterialHasSpecularMapUniform, true);
                    glUniform1i(Shader->MaterialSpecularMapUniform, MaterialPropertyIndex);
                    break;
                }
                case MaterialProperty_Texture_Shininess:
                {
                    opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->TextureId);

                    glBindTextureUnit(MaterialPropertyIndex, Texture->Handle);

                    glUniform1i(Shader->MaterialHasShininessMapUniform, true);
                    glUniform1i(Shader->MaterialShininessMapUniform, MaterialPropertyIndex);
                    break;
                }
                case MaterialProperty_Texture_Metalness:
                {
                    opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->TextureId);

                    //glBindTextureUnit(MaterialPropertyIndex, Texture->Handle);

                    //glUniform1i(Shader->MaterialHasShininessMapUniform, true);
                    //glUniform1i(Shader->MaterialShininessMapUniform, MaterialPropertyIndex);
                    break;
                }
                case MaterialProperty_Texture_Normal:
                {
                    opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->TextureId);

                    glBindTextureUnit(MaterialPropertyIndex, Texture->Handle);

                    glUniform1i(Shader->MaterialHasNormalMapUniform, true);
                    glUniform1i(Shader->MaterialNormalMapUniform, MaterialPropertyIndex);
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
OpenGLInitFramebuffers(opengl_state *State, i32 WindowWidth, i32 WindowHeight, u32 Samples)
{
    State->SourceFramebuffer = {};
    State->DestFramebuffer = {};

    State->SourceFramebuffer.Width = WindowWidth;
    State->SourceFramebuffer.Height = WindowHeight;
    State->SourceFramebuffer.Samples = Samples;

    State->DestFramebuffer.Width = WindowWidth;
    State->DestFramebuffer.Height = WindowHeight;

    glCreateFramebuffers(1, &State->SourceFramebuffer.Handle);

    glCreateRenderbuffers(1, &State->SourceFramebuffer.ColorTarget);
    glNamedRenderbufferStorageMultisample(State->SourceFramebuffer.ColorTarget, Samples, GL_RGBA16F, WindowWidth, WindowHeight);
    glNamedFramebufferRenderbuffer(State->SourceFramebuffer.Handle, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, State->SourceFramebuffer.ColorTarget);

    glCreateRenderbuffers(1, &State->SourceFramebuffer.DepthStencilTarget);
    glNamedRenderbufferStorageMultisample(State->SourceFramebuffer.DepthStencilTarget, Samples, GL_DEPTH24_STENCIL8, WindowWidth, WindowHeight);
    glNamedFramebufferRenderbuffer(State->SourceFramebuffer.Handle, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, State->SourceFramebuffer.DepthStencilTarget);

    GLenum MultiSampledFramebufferStatus = glCheckNamedFramebufferStatus(State->SourceFramebuffer.Handle, GL_FRAMEBUFFER);
    Assert(MultiSampledFramebufferStatus == GL_FRAMEBUFFER_COMPLETE);

    glCreateFramebuffers(1, &State->DestFramebuffer.Handle);

    glCreateTextures(GL_TEXTURE_2D, 1, &State->DestFramebuffer.ColorTarget);
    glTextureStorage2D(State->DestFramebuffer.ColorTarget, 1, GL_RGBA16F, WindowWidth, WindowHeight);

    glTextureParameteri(State->DestFramebuffer.ColorTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(State->DestFramebuffer.ColorTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(State->DestFramebuffer.ColorTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(State->DestFramebuffer.ColorTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(State->DestFramebuffer.Handle, GL_COLOR_ATTACHMENT0, State->DestFramebuffer.ColorTarget, 0);

    GLenum SingleSampledFramebufferStatus = glCheckNamedFramebufferStatus(State->DestFramebuffer.Handle, GL_FRAMEBUFFER);
    Assert(SingleSampledFramebufferStatus == GL_FRAMEBUFFER_COMPLETE);
}

internal void
OpenGLOnWindowResize(opengl_state *State, i32 WindowWidth, i32 WindowHeight, u32 Samples)
{
    State->WindowWidth = WindowWidth;
    State->WindowHeight = WindowHeight;

    glDeleteFramebuffers(1, &State->SourceFramebuffer.Handle);
    glDeleteRenderbuffers(1, &State->SourceFramebuffer.ColorTarget);
    glDeleteRenderbuffers(1, &State->SourceFramebuffer.DepthStencilTarget);

    glDeleteFramebuffers(1, &State->DestFramebuffer.Handle);
    glDeleteTextures(1, &State->DestFramebuffer.ColorTarget);

    OpenGLInitFramebuffers(State, WindowWidth, WindowHeight, Samples);
}

internal void
OpenGLInitRenderer(opengl_state *State, i32 WindowWidth, i32 WindowHeight, u32 Samples)
{
    State->Vendor = (char*)glGetString(GL_VENDOR);
    State->Renderer = (char*)glGetString(GL_RENDERER);
    State->Version = (char*)glGetString(GL_VERSION);
    State->ShadingLanguageVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    State->Stream = CreateStream(SubMemoryArena(&State->Arena, Megabytes(4)));

    State->CascadeShadowMapSize = 4096;
    // todo: set by the game
    State->CascadeBounds[0] = vec2(-0.1f, -5.f);
    State->CascadeBounds[1] = vec2(-3.f, -15.f);
    State->CascadeBounds[2] = vec2(-10.f, -40.f);
    State->CascadeBounds[3] = vec2(-30.f, -120.f);

    OpenGLInitLine(State);
    OpenGLInitRectangle(State);
    OpenGLInitBox(State);
    OpenGLInitText(State);
    OpenGLInitParticles(State);

    OpenGLInitShaders(State);
    OpenGLInitFramebuffers(State, WindowWidth, WindowHeight, Samples);

    // Shadow Map Configuration
    glCreateFramebuffers(1, &State->CascadeShadowMapFBO);
    glCreateTextures(GL_TEXTURE_2D, 4, State->CascadeShadowMaps);

    // todo: use Array Texture
    for (u32 TextureIndex = 0; TextureIndex < ArrayCount(State->CascadeShadowMaps); ++TextureIndex)
    {
        glTextureStorage2D(State->CascadeShadowMaps[TextureIndex], 1, GL_DEPTH_COMPONENT32F, State->CascadeShadowMapSize, State->CascadeShadowMapSize);

        glTextureParameteri(State->CascadeShadowMaps[TextureIndex], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(State->CascadeShadowMaps[TextureIndex], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(State->CascadeShadowMaps[TextureIndex], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(State->CascadeShadowMaps[TextureIndex], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameteri(State->CascadeShadowMaps[TextureIndex], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTextureParameteri(State->CascadeShadowMaps[TextureIndex], GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        vec4 BorderColor = vec4(1.f);
        glTextureParameterfv(State->CascadeShadowMaps[TextureIndex], GL_TEXTURE_BORDER_COLOR, BorderColor.Elements);
    }

    glNamedFramebufferTexture(State->CascadeShadowMapFBO, GL_DEPTH_ATTACHMENT, State->CascadeShadowMaps[0], 0);

    glNamedFramebufferDrawBuffer(State->CascadeShadowMapFBO, GL_NONE);
    glNamedFramebufferReadBuffer(State->CascadeShadowMapFBO, GL_NONE);

    GLenum FramebufferStatus = glCheckNamedFramebufferStatus(State->CascadeShadowMapFBO, GL_FRAMEBUFFER);
    Assert(FramebufferStatus == GL_FRAMEBUFFER_COMPLETE);
    //

    glCreateBuffers(1, &State->ShaderStateUBO);
    glNamedBufferStorage(State->ShaderStateUBO, sizeof(opengl_shader_state), 0, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, State->ShaderStateUBO);

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
    //glFrontFace(GL_CCW);

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
                    Command->Positions, Command->Normals, Command->Tangents, Command->Bitangents, Command->TextureCoords, 
                    Command->Weights, Command->JointIndices, Command->IndexCount, Command->Indices
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
    glBindFramebuffer(GL_FRAMEBUFFER, State->SourceFramebuffer.Handle);
    glDisable(GL_DEPTH_CLAMP);

    if (Options->RenderShadowMap)
    {
        glViewport(0, 0, State->CascadeShadowMapSize, State->CascadeShadowMapSize);
        glBindFramebuffer(GL_FRAMEBUFFER, State->CascadeShadowMapFBO);
        glNamedFramebufferTexture(State->CascadeShadowMapFBO, GL_DEPTH_ATTACHMENT, State->CascadeShadowMaps[Options->CascadeIndex], 0);
        glEnable(GL_DEPTH_CLAMP);

        mat4 TransposeLightProjection = Transpose(Options->CascadeProjection);
        mat4 TransposeLightView = Transpose(Options->CascadeView);

        glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, WorldProjection), sizeof(mat4), &TransposeLightProjection);
        glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, View), sizeof(mat4), &TransposeLightView);
    }

    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header*)((u8*)Commands->RenderCommandsBuffer + BaseAddress);

        if (!Options->RenderShadowMap)
        {
            Out(&State->Stream, "Renderer::%s", RenderCommandNames[Entry->Type]);
        }

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
            case RenderCommand_SetScreenProjection:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_screen_projection *Command = (render_command_set_screen_projection *) Entry;

                    mat4 Projection = OrthographicProjection(Command->Left, Command->Right, Command->Bottom, Command->Top, Command->Near, Command->Far);
                    mat4 TransposeProjection = Transpose(Projection);

                    glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, ScreenProjection), sizeof(mat4), &TransposeProjection);
                }

                break;
            }
            case RenderCommand_SetWorldProjection:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_world_projection *Command = (render_command_set_world_projection *) Entry;

                    mat4 Projection = FrustrumProjection(Command->FovY, Command->Aspect, Command->Near, Command->Far);
                    mat4 TransposeProjection = Transpose(Projection);

                    glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, WorldProjection), sizeof(mat4), &TransposeProjection);
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

                    glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, View), sizeof(mat4), &TransposeView);
                    glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, CameraPosition), sizeof(vec3), &Command->Position);
                    glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, CameraDirection), sizeof(vec3), &Direction);
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
                        glUniform3f(Shader->DirectionalLightDirectionUniform, Command->Light.Direction.x, Command->Light.Direction.y, Command->Light.Direction.z);
                        glUniform3f(Shader->DirectionalLightColorUniform, Command->Light.Color.r, Command->Light.Color.g, Command->Light.Color.b);
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

                        glUniform1i(Shader->PointLightCountUniform, Command->PointLightCount);

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

                glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, Time), sizeof(f32), &Command->Time);

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
            case RenderCommand_DrawPoint:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_point *Command = (render_command_draw_point *) Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                    glPointSize(Command->Size);

                    // todo: use separate PointVAO ?
                    glBindVertexArray(State->LineVAO);
                    glUseProgram(Shader->Program);
                    {
                        mat4 Model = Translate(Command->Position);

                        // todo: world mode?
                        glUniform1i(Shader->ModeUniform, OPENGL_WORLD_SPACE_MODE);
                        glUniformMatrix4fv(Shader->ModelUniform, 1, GL_TRUE, (f32 *) Model.Elements);
                        glUniform4f(Shader->ColorUniform, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
                    }

                    glDrawArrays(GL_POINTS, 0, 1);

                    glPointSize(1.f);
                }

                break;
            }
            case RenderCommand_DrawLine:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_line *Command = (render_command_draw_line *) Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                    glLineWidth(Command->Thickness);

                    glBindVertexArray(State->LineVAO);
                    glUseProgram(Shader->Program);
                    {
                        mat4 T = Translate(Command->Start);
                        mat4 S = Scale(Command->End - Command->Start);
                        mat4 Model = T * S;

                        // todo: world mode?
                        glUniform1i(Shader->ModeUniform, OPENGL_WORLD_SPACE_MODE);
                        glUniformMatrix4fv(Shader->ModelUniform, 1, GL_TRUE, (f32 *) Model.Elements);
                        glUniform4f(Shader->ColorUniform, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
                    }

                    glDrawArrays(GL_LINES, 0, 2);

                    glLineWidth(1.f);
                }

                break;
            }
            case RenderCommand_DrawRectangle:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_rectangle *Command = (render_command_draw_rectangle *) Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                    glBindVertexArray(State->RectangleVAO);
                    glUseProgram(Shader->Program);

                    mat4 Model = Transform(Command->Transform);
                    mat4 View = mat4(1.f);

                    // todo: do smth better
                    glNamedBufferSubData(State->ShaderStateUBO, StructOffset(opengl_shader_state, View), sizeof(mat4), &View);

                    glUniform1i(Shader->ModeUniform, OPENGL_SCREEN_SPACE_MODE);
                    glUniformMatrix4fv(Shader->ModelUniform, 1, GL_TRUE, (f32 *) Model.Elements);
                    glUniform4f(Shader->ColorUniform, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);

                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }

                break;
            }
            case RenderCommand_DrawBox:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_box *Command = (render_command_draw_box *) Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                    glBindVertexArray(State->BoxVAO);
                    glUseProgram(Shader->Program);

                    mat4 Model = Transform(Command->Transform);

                    glUniform1i(Shader->ModeUniform, OPENGL_WORLD_SPACE_MODE);
                    glUniformMatrix4fv(Shader->ModelUniform, 1, GL_TRUE, (f32 *) Model.Elements);
                    glUniform4f(Shader->ColorUniform, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);

                    // todo: configurable
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);

                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

                break;
            }
            case RenderCommand_DrawText:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_text *Command = (render_command_draw_text *) Entry;
                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_TEXT_SHADER_ID);
                    scoped_memory ScopedMemory(&State->Arena);

                    glBindVertexArray(State->TextVAO);
                    glUseProgram(Shader->Program);

                    if (!Command->DepthEnabled)
                    {
                        glDisable(GL_DEPTH_TEST);
                    }

                    font *Font = Command->Font;
                    f32 TextScale = Command->Scale;
                    f32 UnitsPerPixel = Commands->Settings.UnitsPerPixel;
                    u32 CharacterCount = StringLength(Command->Text);

                    u32 Mode = OPENGL_SCREEN_SPACE_MODE;

                    vec3 CameraXAsis = vec3(1.f, 0.f, 0.f);
                    vec3 CameraYAxis = vec3(0.f, 1.f, 0.f);

                    mat4 CameraToWorld = mat4(1.f);
                    vec4 CameraSpacePosition = vec4(Command->Position, 1.f);

                    if (Command->Mode == DrawText_WorldSpace)
                    {
                        Mode = OPENGL_WORLD_SPACE_MODE;

                        mat4 WorldToCamera = Commands->Settings.WorldToCamera;

                        CameraXAsis = WorldToCamera[0].xyz;
                        CameraYAxis = WorldToCamera[1].xyz;

                        CameraToWorld = Commands->Settings.CameraToWorld;
                        CameraSpacePosition = WorldToCamera * vec4(Command->Position, 1.f);
                    }

                    vec2 Position = GetTextStartPosition(Command->Text, Command->Font, Command->Alignment, CameraSpacePosition.xy, TextScale, UnitsPerPixel);

                    opengl_character_point *Points = PushArray(ScopedMemory.Arena, CharacterCount, opengl_character_point);

                    u32 GlyphPositionIndex = 0;

                    for (wchar *At = Command->Text; *At; ++At)
                    {
                        wchar Character = *At;
                        wchar NextCharacter = *(At + 1);

                        glyph *GlyphInfo = GetCharacterGlyph(Font, Character);

                        vec2 SpriteSize = GlyphInfo->SpriteSize;
                        vec2 SpriteOffset = GlyphInfo->UV;
                        vec2 Size = GlyphInfo->CharacterSize * TextScale * UnitsPerPixel;
                        vec2 HalfSize = Size / 2.f;
                        vec2 Alignment = GlyphInfo->Alignment * TextScale * UnitsPerPixel;

                        vec4 WorldSpacePosition = CameraToWorld * vec4(Position + Alignment + HalfSize, CameraSpacePosition.z, 1.f);

                        opengl_character_point *Point = Points + GlyphPositionIndex++;

                        Point->Position = WorldSpacePosition.xyz;
                        Point->Size = Size;
                        Point->SpriteSize = SpriteSize;
                        Point->SpriteOffset = SpriteOffset;

                        f32 HorizontalAdvance = GetHorizontalAdvanceForPair(Font, Character, NextCharacter);
                        Position.x += HorizontalAdvance * TextScale * UnitsPerPixel;
                    }

                    glNamedBufferSubData(State->TextVBO, 0, CharacterCount * sizeof(opengl_character_point), Points);

                    opengl_texture *Texture = OpenGLGetTexture(State, Font->TextureId);
                    glBindTextureUnit(0, Texture->Handle);

                    glUniform1i(Shader->ModeUniform, Mode);
                    glUniform4f(Shader->ColorUniform, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
                    glUniform3f(Shader->CameraXAxisUniform, CameraXAsis.x, CameraXAsis.y, CameraXAsis.z);
                    glUniform3f(Shader->CameraYAxisUniform, CameraYAxis.x, CameraYAxis.y, CameraYAxis.z);

                    glDrawArrays(GL_POINTS, 0, CharacterCount);

                    if (!Command->DepthEnabled)
                    {
                        glEnable(GL_DEPTH_TEST);
                    }
                }

                break;
            }
            case RenderCommand_DrawParticles:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_particles *Command = (render_command_draw_particles *) Entry;
                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PARTICLE_SHADER_ID);
                    scoped_memory ScopedMemory(&State->Arena);

                    glBindVertexArray(State->ParticleVAO);
                    glUseProgram(Shader->Program);

                    mat4 WorldToCamera = Commands->Settings.WorldToCamera;
                    vec3 CameraXAsis = WorldToCamera[0].xyz;
                    vec3 CameraYAxis = WorldToCamera[1].xyz;

                    opengl_particle *Particles = PushArray(ScopedMemory.Arena, Command->ParticleCount, opengl_particle);

                    for (u32 ParticleIndex = 0; ParticleIndex < Command->ParticleCount; ++ParticleIndex)
                    {
                        particle *SourceParticle = Command->Particles + ParticleIndex;
                        opengl_particle *DestParticle = Particles + ParticleIndex;

                        DestParticle->Position = SourceParticle->Position;
                        DestParticle->Size = SourceParticle->Size;
                        DestParticle->Color = SourceParticle->Color;
                    }

                    glNamedBufferSubData(State->ParticleVBO, 0, Command->ParticleCount * sizeof(opengl_particle), Particles);

                    if (Command->Texture)
                    {
                        opengl_texture *Texture = OpenGLGetTexture(State, Command->Texture->TextureId);
                        glBindTextureUnit(0, Texture->Handle);
                    }

                    glUniform3f(Shader->CameraXAxisUniform, CameraXAsis.x, CameraXAsis.y, CameraXAsis.z);
                    glUniform3f(Shader->CameraYAxisUniform, CameraYAxis.x, CameraYAxis.y, CameraYAxis.z);

                    glDrawArrays(GL_POINTS, 0, Command->ParticleCount);
                }

                break;
            }
            case RenderCommand_DrawTexturedQuad:
            {
                render_command_draw_textured_quad *Command = (render_command_draw_textured_quad *) Entry;
                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_TEXTURED_QUAD_SHADER_ID);

                glBindVertexArray(State->RectangleVAO);
                glUseProgram(Shader->Program);

                mat4 Model = Transform(Command->Transform);

                glUniformMatrix4fv(Shader->ModelUniform, 1, GL_TRUE, (f32 *) Model.Elements);
                // todo: param
                glUniform1i(Shader->ModeUniform, OPENGL_WORLD_SPACE_MODE);

                opengl_texture *Texture = OpenGLGetTexture(State, Command->Texture->TextureId);
                glBindTextureUnit(0, Texture->Handle);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glUseProgram(0);
                glBindVertexArray(0);

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
                        case MaterialType_Phong:
                        {
                            mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_SHADER_ID);

                            glUseProgram(Shader->Program);

                            mat4 Model = Transform(Command->Transform);
                            glUniformMatrix4fv(Shader->ModelUniform, 1, GL_TRUE, (f32 *) Model.Elements);

                            OpenGLBlinnPhongShading(State, Options, Shader, Command->Material.MeshMaterial);

                            break;
                        }
                        case MaterialType_Basic:
                        {
                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                            glUseProgram(Shader->Program);

                            mat4 Model = Transform(Command->Transform);
                            material Material = Command->Material;

                            glUniform1i(Shader->ModeUniform, OPENGL_WORLD_SPACE_MODE);
                            glUniformMatrix4fv(Shader->ModelUniform, 1, GL_TRUE, (f32 *) Model.Elements);
                            glUniform4f(Shader->ColorUniform, Material.Color.r, Material.Color.g, Material.Color.b, Material.Color.a);

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
                        case MaterialType_Phong:
                        {
                            mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_SKINNED_SHADER_ID);

                            glUseProgram(Shader->Program);

                            glNamedBufferSubData(SkinningBuffer->SkinningTBO, 0, Command->SkinningMatrixCount * sizeof(mat4), Command->SkinningMatrices);

                            glBindTextureUnit(0, SkinningBuffer->SkinningTBOTexture);
                            glTextureBuffer(SkinningBuffer->SkinningTBOTexture, GL_RGBA32F, SkinningBuffer->SkinningTBO);

                            glUniform1i(Shader->SkinningMatricesSamplerUniform, 0);

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
            case RenderCommand_DrawSkinnedMeshInstanced:
            {
                render_command_draw_skinned_mesh_instanced *Command = (render_command_draw_skinned_mesh_instanced *) Entry;

                if ((Options->RenderShadowMap && Command->Material.CastShadow) || !Options->RenderShadowMap)
                {
                    opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);
                    opengl_skinning_buffer *SkinningBuffer = OpenGLGetSkinningBuffer(State, Command->SkinningBufferId);
                    scoped_memory ScopedMemory(&State->Arena);

                    mat4 *SkinningMatrices = PushArray(ScopedMemory.Arena, Command->InstanceCount * OPENGL_MAX_JOINT_COUNT, mat4);

                    u32 SkinningMatricesIndex = 0;
                    for (u32 InstanceIndex = 0; InstanceIndex < Command->InstanceCount; ++InstanceIndex)
                    {
                        skinned_mesh_instance *Instance = Command->Instances + InstanceIndex;

                        Assert(Instance->SkinningMatrixCount < OPENGL_MAX_JOINT_COUNT);

                        for (u32 JointIndex = 0; JointIndex < OPENGL_MAX_JOINT_COUNT; ++JointIndex)
                        {
                            mat4 *Dest = SkinningMatrices + SkinningMatricesIndex++;
                            mat4 Source = mat4(0.f);

                            if (JointIndex < Instance->SkinningMatrixCount)
                            {
                                Source = Instance->SkinningMatrices[JointIndex];
                            }

                            *Dest = Source;
                        }
                    }

                    glBindVertexArray(MeshBuffer->VAO);

                    if (MeshBuffer->InstanceCount < Command->InstanceCount)
                    {
                        MeshBuffer->InstanceCount = (u32)(Command->InstanceCount * 1.5f);
                        glNamedBufferData(SkinningBuffer->SkinningTBO, MeshBuffer->InstanceCount *OPENGL_MAX_JOINT_COUNT * sizeof(mat4), 0, GL_STREAM_DRAW);
                    }

                    glNamedBufferSubData(SkinningBuffer->SkinningTBO, 0, Command->InstanceCount * OPENGL_MAX_JOINT_COUNT * sizeof(mat4), SkinningMatrices);

                    switch (Command->Material.Type)
                    {
                        case MaterialType_Phong:
                        {
                            mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_SKINNED_INSTANCED_SHADER_ID);

                            glUseProgram(Shader->Program);

                            glBindTextureUnit(0, SkinningBuffer->SkinningTBOTexture);
                            glTextureBuffer(SkinningBuffer->SkinningTBOTexture, GL_RGBA32F, SkinningBuffer->SkinningTBO);

                            glUniform1i(Shader->SkinningMatricesSamplerUniform, 0);

                            OpenGLBlinnPhongShading(State, Options, Shader, Command->Material.MeshMaterial);

                            break;
                        }
                        default:
                        {
                            Assert(!"Not Implemented");
                            break;
                        }
                    }

                    glDrawElementsInstanced(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0, Command->InstanceCount);

                    glUseProgram(0);
                    glBindVertexArray(0);
                }

                break;
            }
            case RenderCommand_DrawMeshInstanced:
            {
                render_command_draw_mesh_instanced *Command = (render_command_draw_mesh_instanced *) Entry;

                if ((Options->RenderShadowMap && Command->Material.CastShadow) || !Options->RenderShadowMap)
                {
                    opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);

                    glBindVertexArray(MeshBuffer->VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, MeshBuffer->InstanceBuffer);

                    if (MeshBuffer->InstanceCount < Command->InstanceCount)
                    {
                        MeshBuffer->InstanceCount = (u32) (Command->InstanceCount * 1.5f);
                        glNamedBufferData(MeshBuffer->InstanceBuffer, MeshBuffer->InstanceCount * sizeof(mesh_instance), 0, GL_STREAM_DRAW);
                    }

                    glNamedBufferSubData(MeshBuffer->InstanceBuffer, 0, Command->InstanceCount * sizeof(mesh_instance), Command->Instances);

                    switch (Command->Material.Type)
                    {
                        case MaterialType_Phong:
                        {
                            mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_INSTANCED_SHADER_ID);

                            glUseProgram(Shader->Program);

                            OpenGLBlinnPhongShading(State, Options, Shader, Command->Material.MeshMaterial);

                            break;
                        }
                        case MaterialType_Basic:
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
    //PROFILE(State->Profiler, "OpenGLProcessRenderCommands");

#if WIN32_RELOADABLE_SHADERS
    {
        PROFILE(State->Profiler, "OpenGLReloadableShaders");

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

            if (!StringEquals(Shader->GeometryShader.FileName, ""))
            {
                FILETIME NewGeometryShaderWriteTime = Win32GetLastWriteTime(Shader->GeometryShader.FileName);
                if (CompareFileTime(&NewGeometryShaderWriteTime, &Shader->GeometryShader.LastWriteTime) != 0)
                {
                    Shader->GeometryShader.LastWriteTime = NewGeometryShaderWriteTime;
                    ShouldReload = true;
                }
            }

            if (ShouldReload)
            {
                OpenGLReloadShader(State, Shader->Id);
            }
        }
    }
#endif

    render_commands_settings *RenderSettings = &Commands->Settings;

    if (State->WindowWidth != RenderSettings->WindowWidth || RenderSettings->WindowHeight != RenderSettings->WindowHeight)
    {
        OpenGLOnWindowResize(State, RenderSettings->WindowWidth, RenderSettings->WindowHeight, RenderSettings->Samples);
    }

    {
        PROFILE(State->Profiler, "OpenGLPrepareScene");
        OpenGLPrepareScene(State, Commands);
    }

#if 1
    if (RenderSettings->EnableCascadedShadowMaps)
    {
        PROFILE(State->Profiler, "OpenGLCascadedShadowMaps");

        game_camera *Camera = RenderSettings->Camera;

        f32 FocalLength = Camera->FocalLength;
        f32 AspectRatio = Camera->AspectRatio;

        mat4 WorldToCamera = RenderSettings->WorldToCamera;
        mat4 CameraToWorld = RenderSettings->CameraToWorld;

        vec3 LightPosition = vec3(0.f);
        vec3 LightDirection = Normalize(RenderSettings->DirectionalLight->Direction);
        vec3 LightUp = vec3(0.f, 1.f, 0.f);

        mat4 LightM = LookAt(LightPosition, LightPosition + LightDirection, LightUp);

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
                vec4 LightSpaceFrustrumCorner = LightM * CameraToWorld * CameraSpaceFrustrumCorners[CornerIndex];

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

            f32 T = (f32) d / (f32) State->CascadeShadowMapSize;

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
    }
#endif

    {
        PROFILE(State->Profiler, "OpenGLRenderScene");

        opengl_render_options RenderOptions = {};
        RenderOptions.ShowCascades = RenderSettings->ShowCascades;
        OpenGLRenderScene(State, Commands, &RenderOptions);
    }

    // Resolve multisample framebuffer
    GLenum Attachments[] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_STENCIL_ATTACHMENT };

    glBlitNamedFramebuffer(
        State->SourceFramebuffer.Handle, 
        State->DestFramebuffer.Handle, 
        0, 0, 
        State->SourceFramebuffer.Width,
        State->SourceFramebuffer.Height,
        0, 0, 
        State->DestFramebuffer.Width,
        State->DestFramebuffer.Height,
        GL_COLOR_BUFFER_BIT, 
        GL_NEAREST
    );
    glInvalidateNamedFramebufferData(State->SourceFramebuffer.Handle, ArrayCount(Attachments), Attachments);

    // Draw a full screen triangle for postprocessing
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, RenderSettings->WindowWidth, RenderSettings->WindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_FRAMEBUFFER_SHADER_ID);

    glUseProgram(Shader->Program);
    glBindTextureUnit(0, State->DestFramebuffer.ColorTarget);
    glBindVertexArray(State->RectangleVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
