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
    u32 *Indices
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
    glBufferData(GL_ARRAY_BUFFER, BufferSize, 0, GL_STATIC_DRAW);

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
    //

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

internal void
OpenGLAddMeshBufferInstanced(
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

    // per-instance attributes
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *)(BufferSize + StructOffset(render_instance, Model) + 0));
    glVertexAttribDivisor(7, 1);

    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *)(BufferSize + StructOffset(render_instance, Model) + sizeof(vec4)));
    glVertexAttribDivisor(8, 1);

    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *)(BufferSize + StructOffset(render_instance, Model) + 2 * sizeof(vec4)));
    glVertexAttribDivisor(9, 1);

    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(render_instance), (void *)(BufferSize + StructOffset(render_instance, Model) + 3 * sizeof(vec4)));
    glVertexAttribDivisor(10, 1);

#if 0
    glEnableVertexAttribArray(11);
    glVertexAttribIPointer(11, 1, GL_UNSIGNED_INT, sizeof(render_instance), (void *)(VertexCount * sizeof(skinned_vertex) + StructOffset(render_instance, Flags)));
    glVertexAttribDivisor(11, 1);
#endif

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
    Shader->BlinkUniformLocation = glGetUniformLocation(Program, "u_Blink");
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
    // Common
    read_file_result VersionShaderFile = State->Platform->ReadFile((char *)"..\\src\\renderers\\OpenGL\\shaders\\common\\version.glsl", Arena, true);
    char *VersionShaderSource = (char *)VersionShaderFile.Contents;

    read_file_result MathShaderFile = State->Platform->ReadFile((char *)"..\\src\\renderers\\OpenGL\\shaders\\common\\math.glsl", Arena, true);
    char *MathShaderSource = (char *)MathShaderFile.Contents;

    read_file_result BlinnPhongShaderFile = State->Platform->ReadFile((char *)"..\\src\\renderers\\OpenGL\\shaders\\common\\blinn_phong.glsl", Arena, true);
    char *BlinnPhongShaderSource = OpenGLPreprocessShader((char *)BlinnPhongShaderFile.Contents, BlinnPhongShaderFile.Size, Arena);
    //

    read_file_result ShaderFile = State->Platform->ReadFile(ShaderFileName, Arena, true);
    char *ShaderSource = (char *)ShaderFile.Contents;

    char **Sources = PushArray(Arena, Count, char *);

    // todo:
    Sources[0] = VersionShaderSource;
    Sources[1] = MathShaderSource;
    Sources[2] = BlinnPhongShaderSource;
    Sources[3] = ShaderSource;

    return Sources;
}

internal void
OpenGLLoadShader(opengl_state *State, u32 Id, char *VertexShaderFileName, char *FragmentShaderFileName)
{
    opengl_shader *Shader = State->Shaders + State->CurrentShaderCount++;

    scoped_memory ScopedMemory(&State->Arena);

    u32 Count = 4;
    char **VertexSource = OpenGLLoadShaderFile(State, Id, VertexShaderFileName, Count, ScopedMemory.Arena);
    char **FragmentSource = OpenGLLoadShaderFile(State, Id, FragmentShaderFileName, Count, ScopedMemory.Arena);

    GLuint VertexShader = OpenGLCreateShader(GL_VERTEX_SHADER, Count, VertexSource);
    GLuint FragmentShader = OpenGLCreateShader(GL_FRAGMENT_SHADER, Count, FragmentSource);
    GLuint Program = OpenGLCreateProgram(VertexShader, FragmentShader);

    Shader->Id = Id;
    Shader->Program = Program;

    CopyString(VertexShaderFileName, Shader->VertexShaderFileName);
    CopyString(FragmentShaderFileName, Shader->FragmentShaderFileName);

#if WIN32_RELOADABLE_SHADERS
    Shader->LastVertexShaderWriteTime = Win32GetLastWriteTime(Shader->VertexShaderFileName);
    Shader->LastFragmentShaderWriteTime = Win32GetLastWriteTime(Shader->FragmentShaderFileName);
#endif

    OpenGLLoadShaderUniforms(Shader);
}

internal void
OpenGLReloadShader(opengl_state *State, u32 Id)
{
    opengl_shader *Shader = OpenGLGetShader(State, Id);

    scoped_memory ScopedMemory(&State->Arena);

    u32 Count = 4;
    char **VertexSource = OpenGLLoadShaderFile(State, Id, Shader->VertexShaderFileName, Count, ScopedMemory.Arena);
    char **FragmentSource = OpenGLLoadShaderFile(State, Id, Shader->FragmentShaderFileName, Count, ScopedMemory.Arena);

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
}

internal void
OpenGLBlinnPhongShading(opengl_state *State, opengl_shader *Shader, mesh_material *MeshMaterial)
{
    glUniform1i(Shader->MaterialHasDiffuseMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasSpecularMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasShininessMapUniformLocation, false);
    glUniform1i(Shader->MaterialHasNormalMapUniformLocation, false);

    // todo: magic numbers
    opengl_texture *Texture = OpenGLGetTexture(State, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, Texture->Handle);

    if (MeshMaterial)
    {
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
OpenGLProcessRenderCommands(opengl_state *State, render_commands *Commands)
{
#if WIN32_RELOADABLE_SHADERS
    for (u32 ShaderIndex = 0; ShaderIndex < State->CurrentShaderCount; ++ShaderIndex)
    {
        opengl_shader *Shader = State->Shaders + ShaderIndex;

        b32 ShouldReload = false;

        FILETIME NewVertexShaderWriteTime = Win32GetLastWriteTime(Shader->VertexShaderFileName);
        if (CompareFileTime(&NewVertexShaderWriteTime, &Shader->LastVertexShaderWriteTime) != 0)
        {
            Shader->LastVertexShaderWriteTime = NewVertexShaderWriteTime;
            ShouldReload = true;
        }

        FILETIME NewFragmentShaderWriteTime = Win32GetLastWriteTime(Shader->FragmentShaderFileName);
        if (CompareFileTime(&NewFragmentShaderWriteTime, &Shader->LastFragmentShaderWriteTime) != 0)
        {
            Shader->LastFragmentShaderWriteTime = NewFragmentShaderWriteTime;
            ShouldReload = true;
        }

        if (ShouldReload)
        {
            OpenGLReloadShader(State, Shader->Id);
        }
    }
#endif

    if (State->WindowWidth != Commands->WindowWidth || State->WindowHeight != Commands->WindowHeight)
    {
        OpenGLOnWindowResize(State, Commands->WindowWidth, Commands->WindowHeight);
    }

    // todo: move commands to buckets (based on shader?)
    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

        // todo: use RenderTarget somehow?
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, State->MultiSampledFBO);

        u32 ShadowMapWidth = 2048;
        u32 ShadowMapHeight = 2048;

#if 0
        // todo: if initialized
        if (State->ShaderStateUBO)
        {
            mat4 Projection = Orthographic(-10.f, 10.f, -10.f, 10.f, -10.f, 10.f);
            mat4 TransposeProjection = Transpose(Projection);
            mat4 View = LookAt(vec3(-2.0f, 4.0f, -1.0f), vec3(0.f), vec3(0.f, 1.f, 0.f));
            mat4 TransposeView = Transpose(View);

            glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, Projection), sizeof(mat4), &TransposeProjection);
            glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, View), sizeof(mat4), &TransposeView);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
#endif

        switch (Entry->Type)
        {
            case RenderCommand_InitRenderer:
            {
                render_command_init_renderer *Command = (render_command_init_renderer *)Entry;

                OpenGLInitLine(State);
                OpenGLInitRectangle(State);
                OpenGLInitShaders(State);
                OpenGLInitFramebuffers(State, Commands->WindowWidth, Commands->WindowHeight);

                // Shadow Map Configuration
                glGenFramebuffers(1, &State->DepthMapFBO);
                glBindFramebuffer(GL_FRAMEBUFFER, State->DepthMapFBO);

                glGenTextures(1, &State->DepthMapTexture);
                glBindTexture(GL_TEXTURE_2D, State->DepthMapTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowMapWidth, ShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, State->DepthMapTexture, 0);

                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);

                GLenum FramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                Assert(FramebufferStatus == GL_FRAMEBUFFER_COMPLETE);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                //

                glGenBuffers(1, &State->SkinningTBO);
                glBindBuffer(GL_TEXTURE_BUFFER, State->SkinningTBO);
                // todo: size?
                glBufferData(GL_TEXTURE_BUFFER, Kilobytes(32), 0, GL_STREAM_DRAW);
                glBindBuffer(GL_TEXTURE_BUFFER, 0);

                glGenTextures(1, &State->SkinningTBOTexture);

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
                //glEnable(GL_FRAMEBUFFER_SRGB);

                // todo: use GL_ZERO_TO_ONE?
                glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
                
                break;
            }
            case RenderCommand_AddMesh:
            {
                render_command_add_mesh *Command = (render_command_add_mesh *)Entry;

                // todo: maybe split into two separate commands (AddMesh/AddMeshInstanced)?
                if (Command->MaxInstanceCount > 0)
                {
                    OpenGLAddMeshBufferInstanced(
                        State, Command->MeshId, Command->VertexCount, 
                        Command->Positions, Command->Normals, Command->Tangents, Command->Bitangents, Command->TextureCoords, Command->Weights, Command->JointIndices, 
                        Command->IndexCount, Command->Indices, Command->MaxInstanceCount
                    );
                }
                else
                {
                    OpenGLAddMeshBuffer(
                        State, Command->MeshId, Command->VertexCount, 
                        Command->Positions, Command->Normals, Command->Tangents, Command->Bitangents, Command->TextureCoords, Command->Weights, Command->JointIndices,
                        Command->IndexCount, Command->Indices
                    );
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
                mat4 TransposeProjection = Transpose(Projection);

                glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, Projection), sizeof(mat4), &TransposeProjection);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                break;
            }
            case RenderCommand_SetPerspectiveProjection:
            {
                render_command_set_perspective_projection *Command = (render_command_set_perspective_projection *)Entry;

                mat4 Projection = Perspective(Command->FovY, Command->Aspect, Command->Near, Command->Far);
                mat4 TransposeProjection = Transpose(Projection);

                glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, Projection), sizeof(mat4), &TransposeProjection);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                break;
            }
            case RenderCommand_SetCamera:
            {
                render_command_set_camera *Command = (render_command_set_camera *)Entry;

                mat4 View = LookAt(Command->Position, Command->Target, Command->Up);
                mat4 TransposeView = Transpose(View);

                glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, View), sizeof(mat4), &TransposeView);
                glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, CameraPosition), sizeof(vec3), &Command->Position);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                break;
            }
            case RenderCommand_SetTime:
            {
                render_command_set_time *Command = (render_command_set_time *)Entry;

                glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, Time), sizeof(f32), &Command->Time);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

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

                glDisable(GL_DEPTH_TEST);

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

                glEnable(GL_DEPTH_TEST);
                
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

                glBindBuffer(GL_UNIFORM_BUFFER, State->ShaderStateUBO);
                glBufferSubData(GL_UNIFORM_BUFFER, StructOffset(opengl_shader_state, View), sizeof(mat4), &View);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);
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

                OpenGLBlinnPhongShading(State, Shader, 0);

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
            case RenderCommand_SetPointLights:
            {
                render_command_set_point_lights *Command = (render_command_set_point_lights *)Entry;

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

                        OpenGLBlinnPhongShading(State, Shader, Command->Material.MeshMaterial);

                        break;
                    }
                    case MaterialType_Unlit:
                    {
                        opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);

                        glUseProgram(Shader->Program);

                        mat4 Model = Transform(Command->Transform);
                        material Material = Command->Material;

                        glUniformMatrix4fv(Shader->ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);
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

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SIMPLE_SHADER_ID);
                    glUniform1i(Shader->BlinkUniformLocation, true);
                }

                glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);

                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

                        OpenGLBlinnPhongShading(State, Shader, Command->Material.MeshMaterial);

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

                /*GLint PrevPolygonMode[2];
                glGetIntegerv(GL_POLYGON_MODE, PrevPolygonMode);
                glPolygonMode(GL_FRONT_AND_BACK, Command->Material.IsWireframe ? GL_LINE : GL_FILL);*/

                glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);

                //glPolygonMode(GL_FRONT_AND_BACK, PrevPolygonMode[0]);

                glUseProgram(0);
                glBindVertexArray(0);

                break;
            }
            case RenderCommand_DrawMeshInstanced:
            {
                render_command_draw_mesh_instanced *Command = (render_command_draw_mesh_instanced *)Entry;

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

                        OpenGLBlinnPhongShading(State, Shader, Command->Material.MeshMaterial);

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
    // todo: still aliasing :(
    glBindFramebuffer(GL_READ_FRAMEBUFFER, State->MultiSampledFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, State->SingleSampledFBO);
    glBlitFramebuffer(
        0, 0, Commands->WindowWidth, Commands->WindowHeight, 
        0, 0, Commands->WindowWidth, Commands->WindowHeight, 
        GL_COLOR_BUFFER_BIT, 
        GL_LINEAR
    );

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glViewport(0, 0, Commands->WindowWidth, Commands->WindowHeight);

    glClearColor(1.f, 0.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_FRAMEBUFFER_SHADER_ID);

    glBindVertexArray(State->RectangleVAO);
    glUseProgram(Shader->Program);

    glBindTexture(GL_TEXTURE_2D, State->SingleSampledColorTexture);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#else
    glBindFramebuffer(GL_READ_FRAMEBUFFER, State->MultiSampledFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // todo: performance
    glBlitFramebuffer(
        0, 0, Commands->WindowWidth, Commands->WindowHeight,
        0, 0, Commands->WindowWidth, Commands->WindowHeight,
        GL_COLOR_BUFFER_BIT,
        GL_LINEAR
    );
#endif
}
