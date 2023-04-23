#include <glad.c>

#include "dummy.h"
#include "win32_dummy.h"
#include "win32_dummy_opengl.h"

FILETIME Win32GetLastWriteTime(char *FileName);
void OpenGLInitRenderer(opengl_state *State, i32 WindowWidth, i32 WindowHeight, u32 Samples);

#define GladLoadGLLoader gladLoadGLLoader

#ifdef DEBUG
#define GladSetPostCallback glad_set_post_callback
#else
#define GladSetPostCallback(...)
#endif

// todo: deprecated in favor of OpenGLLogMessage (glDebugMessageCallback)?
void Win32GladPostCallback(const char *Name, void *FuncPtr, i32 LenArgs, ...) {
#if 1
    GLenum ErrorCode;
    ErrorCode = glad_glGetError();

    if (ErrorCode != GL_NO_ERROR)
    {
        char OpenGLError[256];
        FormatString(OpenGLError, "OpenGL Error: %d in %s\n", ErrorCode, Name);

        Assert(!OpenGLError);
    }
#endif
}

// https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions
inline void *
Win32GetOpenGLFuncAddress(char *Name)
{
    void *Result = (void *)wglGetProcAddress(Name);

    if (Result == 0 || (Result == (void *)0x1) || (Result == (void *)0x2) || (Result == (void *)0x3) || (Result == (void *)-1))
    {
        HMODULE OpenGLModule = LoadLibraryA("opengl32.dll");
        if (OpenGLModule)
        {
            Result = (void *)GetProcAddress(OpenGLModule, Name);
        }
    }

    return Result;
}

inline void
Win32OpenGLPresentFrame(opengl_state *State, bool32 VSync)
{
    State->wglSwapIntervalEXT(VSync);
    SwapBuffers(State->WindowDC);
}

dummy_internal void
Win32InitOpenGL(opengl_state *State, win32_platform_state *PlatformState)
{
    RECT WindowRect;
    GetClientRect(PlatformState->WindowHandle, &WindowRect);

    State->WindowWidth = WindowRect.right - WindowRect.left;
    State->WindowHeight = WindowRect.bottom - WindowRect.top;

    State->WindowDC = GetDC(PlatformState->WindowHandle);

    WNDCLASS FakeWindowClass = {};
    FakeWindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    FakeWindowClass.lpfnWndProc = DefWindowProc;
    FakeWindowClass.hInstance = PlatformState->hInstance;
    FakeWindowClass.lpszClassName = L"Fake OpenGL Window Class";

    RegisterClass(&FakeWindowClass);

    HWND FakeWindowHandle = CreateWindowEx(0, FakeWindowClass.lpszClassName, L"Fake OpenGL Window", 0,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, PlatformState->hInstance, 0
    );
    HDC FakeWindowDC = GetDC(FakeWindowHandle);

    PIXELFORMATDESCRIPTOR FakePFD = {};
    FakePFD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    FakePFD.nVersion = 1;
    FakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    FakePFD.iPixelType = PFD_TYPE_RGBA;
    FakePFD.cColorBits = 32;
    FakePFD.cAlphaBits = 8;
    FakePFD.cDepthBits = 24;

    i32 FakePixelFormatIndex = ChoosePixelFormat(FakeWindowDC, &FakePFD);

    if (FakePixelFormatIndex)
    {
        if (SetPixelFormat(FakeWindowDC, FakePixelFormatIndex, &FakePFD))
        {
            HGLRC FakeRC = wglCreateContext(FakeWindowDC);

            if (wglMakeCurrent(FakeWindowDC, FakeRC))
            {
                // OpenGL 1.1 is initialized
                HDC WindowDC = GetDC(PlatformState->WindowHandle);

                PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
                PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
                State->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
                State->wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

                i32 PixelAttribs[] = 
                {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    WGL_ALPHA_BITS_ARB, 8,
                    WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                    WGL_SAMPLES_ARB, (i32) PlatformState->Samples,
                    0
                };

                i32 PixelFormatIndex;
                u32 NumFormats;
                wglChoosePixelFormatARB(WindowDC, PixelAttribs, 0, 1, &PixelFormatIndex, &NumFormats);

                PIXELFORMATDESCRIPTOR PFD = {};
                DescribePixelFormat(WindowDC, PixelFormatIndex, sizeof(PFD), &PFD);
                
                if (SetPixelFormat(WindowDC, PixelFormatIndex, &PFD))
                {
                    i32 OpenGLVersionMajor = 4;
                    i32 OpenGLVersionMinor = 5;

                    i32 ContextAttribs[] = {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, OpenGLVersionMajor,
                        WGL_CONTEXT_MINOR_VERSION_ARB, OpenGLVersionMinor,
                        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                        0
                    };

                    HGLRC RC = wglCreateContextAttribsARB(WindowDC, 0, ContextAttribs);

                    wglMakeCurrent(0, 0);
                    wglDeleteContext(FakeRC);
                    ReleaseDC(FakeWindowHandle, FakeWindowDC);
                    DestroyWindow(FakeWindowHandle);

                    if (wglMakeCurrent(WindowDC, RC))
                    {
                        // OpenGL 4.5 is initialized
                        GladLoadGLLoader((GLADloadproc)Win32GetOpenGLFuncAddress);
                        GladSetPostCallback(Win32GladPostCallback);

                        OpenGLInitRenderer(State, PlatformState->WindowWidth, PlatformState->WindowHeight, PlatformState->Samples);
                    }
                    else
                    {
                        DWORD Error = GetLastError();
                        Assert(!"wglMakeCurrent failed");
                    }
                }
                else
                {
                    DWORD Error = GetLastError();
                    Assert(!"SetPixelFormat failed");
                }
            }
            else
            {
                DWORD Error = GetLastError();
                Assert(!"wglMakeCurrent failed");
            }
        }
        else
        {
            DWORD Error = GetLastError();
            Assert(!"SetPixelFormat failed");
        }
    }
    else
    {
        DWORD Error = GetLastError();
        Assert(!"ChoosePixelFormat failed");
    }
}

dummy_internal void
Win32ShutdownOpenGL(opengl_state *State)
{

}

dummy_internal GLuint
OpenGLCreateShader(GLenum Type, GLsizei Count, char **Source, bool32 CrashIfError = true)
{
    GLuint Shader = glCreateShader(Type);
    glShaderSource(Shader, Count, Source, NULL);
    glCompileShader(Shader);

    GLint Status;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Status);

    if (Status != GL_TRUE)
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

dummy_internal GLuint
OpenGLCreateProgram(u32 ShaderCount, GLuint *Shaders, bool32 CrashIfError = true)
{
    GLuint Program = glCreateProgram();

    for (u32 ShaderIndex = 0; ShaderIndex < ShaderCount; ++ShaderIndex)
    {
        glAttachShader(Program, Shaders[ShaderIndex]);
    }

    glLinkProgram(Program);

    for (u32 ShaderIndex = 0; ShaderIndex < ShaderCount; ++ShaderIndex)
    {
        glDetachShader(Program, Shaders[ShaderIndex]);
        glDeleteShader(Shaders[ShaderIndex]);
    }

    GLint Status;
    glGetProgramiv(Program, GL_LINK_STATUS, &Status);

    if (Status == GL_TRUE)
    {
        glValidateProgram(Program);
        glGetProgramiv(Program, GL_VALIDATE_STATUS, &Status);
    }

    if (Status != GL_TRUE)
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

dummy_internal void
OpenGLInitLine(opengl_state *State)
{
    vec3 LineVertices[] =
    {
        vec3(0.f, 0.f, 0.f),
        vec3(1.f, 1.f, 1.f),
    };

    glCreateVertexArrays(1, &State->Line.VAO);

    glCreateBuffers(1, &State->Line.VBO);
    glNamedBufferStorage(State->Line.VBO, sizeof(LineVertices), LineVertices, 0);

    OpenGLVertexAttribute(State->Line.VAO, State->Line.VBO, 0, 3, 0, 0, sizeof(vec3));
}

dummy_internal void
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

    glCreateVertexArrays(1, &State->Rectangle.VAO);

    glCreateBuffers(1, &State->Rectangle.VBO);
    glNamedBufferStorage(State->Rectangle.VBO, sizeof(RectangleVertices), RectangleVertices, 0);

    OpenGLVertexAttribute(State->Rectangle.VAO, State->Rectangle.VBO, 0, 3, 0, 0, Stride);
    OpenGLVertexAttribute(State->Rectangle.VAO, State->Rectangle.VBO, 1, 2, 0, sizeof(vec3), Stride);
}

dummy_internal void
OpenGLInitBox(opengl_state *State)
{
    vec3 BoxVertices[] =
    {
        vec3(-1.0f, 1.0f, 1.0f),
        vec3(-1.0f, -1.0f, 1.0f),
        vec3(1.0f, -1.0f, 1.0f),
        vec3(1.0f, -1.0f, 1.0f),
        vec3(1.0f, 1.0f, 1.0f),
        vec3(-1.0f, 1.0f, 1.0f),

        vec3(1.0f, -1.0f, 1.0f),
        vec3(1.0f, -1.0f, -1.0f),
        vec3(1.0f, 1.0f, -1.0f),
        vec3(1.0f, 1.0f, -1.0f),
        vec3(1.0f, 1.0f, 1.0f),
        vec3(1.0f, -1.0f, 1.0f),

        vec3(-1.0f, -1.0f, -1.0f),
        vec3(-1.0f, -1.0f, 1.0f),
        vec3(-1.0f, 1.0f, 1.0f),
        vec3(-1.0f, 1.0f, 1.0f),
        vec3(-1.0f, 1.0f, -1.0f),
        vec3(-1.0f, -1.0f, -1.0f),

        vec3(-1.0f, -1.0f, -1.0f),
        vec3(-1.0f, 1.0f, -1.0f),
        vec3(1.0f, 1.0f, -1.0f),
        vec3(1.0f, 1.0f, -1.0f),
        vec3(1.0f, -1.0f, -1.0f),
        vec3(-1.0f, -1.0f, -1.0f),

        vec3(-1.0f, -1.0f, -1.0f),
        vec3(1.0f, -1.0f, -1.0f),
        vec3(1.0f, -1.0f, 1.0f),
        vec3(1.0f, -1.0f, 1.0f),
        vec3(-1.0f, -1.0f, 1.0f),
        vec3(-1.0f, -1.0f, -1.0f),

        vec3(-1.0f, 1.0f, -1.0f),
        vec3(-1.0f, 1.0f, 1.0f),
        vec3(1.0f, 1.0f, -1.0f),
        vec3(1.0f, 1.0f, -1.0f),
        vec3(-1.0f, 1.0f, 1.0f),
        vec3(1.0f, 1.0f, 1.0f)
    };

    glCreateVertexArrays(1, &State->Box.VAO);

    glCreateBuffers(1, &State->Box.VBO);
    glNamedBufferStorage(State->Box.VBO, sizeof(BoxVertices), BoxVertices, 0);

    OpenGLVertexAttribute(State->Box.VAO, State->Box.VBO, 0, 3, 0, 0, sizeof(vec3));
}

dummy_internal void
OpenGLInitText(opengl_state *State)
{
    u32 MaxCharacterLength = 1024;

    glCreateVertexArrays(1, &State->Text.VAO);
    glCreateBuffers(1, &State->Text.VBO);
    glNamedBufferStorage(State->Text.VBO, MaxCharacterLength * sizeof(opengl_character_point), 0, GL_DYNAMIC_STORAGE_BIT);

    OpenGLVertexAttribute(State->Text.VAO, State->Text.VBO, 0, 3, 0, StructOffset(opengl_character_point, Position), sizeof(opengl_character_point));
    OpenGLVertexAttribute(State->Text.VAO, State->Text.VBO, 1, 2, 0, StructOffset(opengl_character_point, Size), sizeof(opengl_character_point));
    OpenGLVertexAttribute(State->Text.VAO, State->Text.VBO, 2, 2, 0, StructOffset(opengl_character_point, SpriteSize), sizeof(opengl_character_point));
    OpenGLVertexAttribute(State->Text.VAO, State->Text.VBO, 3, 2, 0, StructOffset(opengl_character_point, SpriteOffset), sizeof(opengl_character_point));
}

dummy_internal void
OpenGLInitParticles(opengl_state *State)
{
    u32 MaxParticleCount = 4096;

    glCreateVertexArrays(1, &State->Particle.VAO);
    glCreateBuffers(1, &State->Particle.VBO);
    glNamedBufferStorage(State->Particle.VBO, MaxParticleCount * sizeof(opengl_particle), 0, GL_DYNAMIC_STORAGE_BIT);

    OpenGLVertexAttribute(State->Particle.VAO, State->Particle.VBO, 0, 3, 0, StructOffset(opengl_particle, Position), sizeof(opengl_particle));
    OpenGLVertexAttribute(State->Particle.VAO, State->Particle.VBO, 1, 2, 0, StructOffset(opengl_particle, Size), sizeof(opengl_particle));
    OpenGLVertexAttribute(State->Particle.VAO, State->Particle.VBO, 2, 4, 0, StructOffset(opengl_particle, Color), sizeof(opengl_particle));
}

inline opengl_mesh_buffer *
OpenGLGetMeshBuffer(opengl_state *State, u32 Id)
{
    opengl_mesh_buffer *Result = HashTableLookup(&State->MeshBuffers, Id);

    Assert(Result);

    return Result;
}

inline opengl_skinning_buffer *
OpenGLGetSkinningBuffer(opengl_state *State, u32 Id)
{
    opengl_skinning_buffer *Result = HashTableLookup(&State->SkinningBuffers, Id);

    Assert(Result);

    return Result;
}

inline opengl_texture *
OpenGLGetTexture(opengl_state *State, u32 Id)
{
    opengl_texture *Result = HashTableLookup(&State->Textures, Id);

    Assert(Result);

    return Result;
}

inline opengl_shader *
OpenGLGetShader(opengl_state *State, u32 Id)
{
    opengl_shader *Result = HashTableLookup(&State->Shaders, Id);

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

dummy_internal void
OpenGLAddSkinningBuffer(opengl_state *State, u32 Id, u32 SkinningMatrixCount)
{
    opengl_skinning_buffer *SkinningBuffer = HashTableLookup(&State->SkinningBuffers, Id);

    if (IsSlotEmpty(SkinningBuffer->Key))
    {
        SkinningBuffer->Key = Id;

        glCreateBuffers(1, &SkinningBuffer->SkinningTBO);
        glNamedBufferData(SkinningBuffer->SkinningTBO, SkinningMatrixCount * sizeof(mat4), 0, GL_STREAM_DRAW);

        glCreateTextures(GL_TEXTURE_BUFFER, 1, &SkinningBuffer->SkinningTBOTexture);
    }
}

dummy_internal void
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
    opengl_mesh_buffer *MeshBuffer = HashTableLookup(&State->MeshBuffers, MeshId);

    Assert(IsSlotEmpty(MeshBuffer->Key));

    u32 BufferSize = GetMeshVerticesSize(VertexCount, Positions, Normals, Tangents, Bitangents, TextureCoords, Weights, JointIndices);

    MeshBuffer->Key = MeshId;
    MeshBuffer->VertexCount = VertexCount;
    MeshBuffer->IndexCount = IndexCount;
    MeshBuffer->BufferSize = BufferSize;
    MeshBuffer->InstanceCount = 0;

    glCreateVertexArrays(1, &MeshBuffer->VAO);

    glCreateBuffers(1, &MeshBuffer->VertexBuffer);
    glNamedBufferData(MeshBuffer->VertexBuffer, BufferSize, 0, GL_STATIC_DRAW);

    // per-vertex attributes
    u32 Offset = 0;
    if (Positions)
    {
        OpenGLVertexAttribute(MeshBuffer->VAO, MeshBuffer->VertexBuffer, 0, 3, Offset, 0, sizeof(vec3));
        glNamedBufferSubData(MeshBuffer->VertexBuffer, Offset, VertexCount * sizeof(vec3), Positions);
        Offset += VertexCount * sizeof(vec3);

        //
        scoped_memory ScopedMemory(State->Arena);
        vec4 *PositionsVec4 = PushArray(ScopedMemory.Arena, VertexCount, vec4);

        for (u32 VertexIndex = 0; VertexIndex < VertexCount; ++VertexIndex)
        {
            vec3 Position = Positions[VertexIndex];
            vec4 *PositionVec4 = PositionsVec4 + VertexIndex;

            PositionVec4->x = Position.x;
            PositionVec4->y = Position.y;
            PositionVec4->z = Position.z;
            PositionVec4->w = 1.f;
        }

        glCreateBuffers(1, &MeshBuffer->PositionsBuffer);
        glNamedBufferData(MeshBuffer->PositionsBuffer, VertexCount * sizeof(vec4), PositionsVec4, GL_STATIC_DRAW);
        //
    }

    if (Normals)
    {
        OpenGLVertexAttribute(MeshBuffer->VAO, MeshBuffer->VertexBuffer, 1, 3, Offset, 0, sizeof(vec3));
        glNamedBufferSubData(MeshBuffer->VertexBuffer, Offset, VertexCount * sizeof(vec3), Normals);
        Offset += VertexCount * sizeof(vec3);
    }

    if (Tangents)
    {
        OpenGLVertexAttribute(MeshBuffer->VAO, MeshBuffer->VertexBuffer, 2, 3, Offset, 0, sizeof(vec3));
        glNamedBufferSubData(MeshBuffer->VertexBuffer, Offset, VertexCount * sizeof(vec3), Tangents);
        Offset += VertexCount * sizeof(vec3);
    }

    if (Bitangents)
    {
        OpenGLVertexAttribute(MeshBuffer->VAO, MeshBuffer->VertexBuffer, 3, 3, Offset, 0, sizeof(vec3));
        glNamedBufferSubData(MeshBuffer->VertexBuffer, Offset, VertexCount * sizeof(vec3), Bitangents);
        Offset += VertexCount * sizeof(vec3);
    }

    if (TextureCoords)
    {
        OpenGLVertexAttribute(MeshBuffer->VAO, MeshBuffer->VertexBuffer, 4, 2, Offset, 0, sizeof(vec2));
        glNamedBufferSubData(MeshBuffer->VertexBuffer, Offset, VertexCount * sizeof(vec2), TextureCoords);
        Offset += VertexCount * sizeof(vec2);
    }

    if (Weights)
    {
        OpenGLVertexAttribute(MeshBuffer->VAO, MeshBuffer->VertexBuffer, 5, 4, Offset, 0, sizeof(vec4));
        glNamedBufferSubData(MeshBuffer->VertexBuffer, Offset, VertexCount * sizeof(vec4), Weights);
        Offset += VertexCount * sizeof(vec4);

        glCreateBuffers(1, &MeshBuffer->WeightsBuffer);
        glNamedBufferData(MeshBuffer->WeightsBuffer, VertexCount * sizeof(vec4), Weights, GL_STATIC_DRAW);
    }

    if (JointIndices)
    {
        OpenGLVertexAttributeInteger(MeshBuffer->VAO, MeshBuffer->VertexBuffer, 6, 4, Offset, 0, sizeof(ivec4));
        glNamedBufferSubData(MeshBuffer->VertexBuffer, Offset, VertexCount * sizeof(ivec4), JointIndices);
        Offset += VertexCount * sizeof(ivec4);

        glCreateBuffers(1, &MeshBuffer->JointIndicesBuffer);
        glNamedBufferData(MeshBuffer->JointIndicesBuffer, VertexCount * sizeof(ivec4), JointIndices, GL_STATIC_DRAW);
    }

    if (Weights && JointIndices)
    {
        glCreateBuffers(1, &MeshBuffer->SkinningMatricesBuffer);
        glNamedBufferData(MeshBuffer->SkinningMatricesBuffer, VertexCount * sizeof(mat4), 0, GL_STREAM_DRAW);
    }

    glCreateBuffers(1, &MeshBuffer->InstanceBuffer);
    glNamedBufferData(MeshBuffer->InstanceBuffer, 0, 0, GL_STREAM_DRAW);

    // per-instance attributes
    OpenGLInstanceAttribute(MeshBuffer->VAO, MeshBuffer->InstanceBuffer, 7, 4, 0, (StructOffset(mesh_instance, Model) + 0 * sizeof(vec4)), sizeof(mesh_instance));
    OpenGLInstanceAttribute(MeshBuffer->VAO, MeshBuffer->InstanceBuffer, 8, 4, 0, (StructOffset(mesh_instance, Model) + 1 * sizeof(vec4)), sizeof(mesh_instance));
    OpenGLInstanceAttribute(MeshBuffer->VAO, MeshBuffer->InstanceBuffer, 9, 4, 0, (StructOffset(mesh_instance, Model) + 2 * sizeof(vec4)), sizeof(mesh_instance));
    OpenGLInstanceAttribute(MeshBuffer->VAO, MeshBuffer->InstanceBuffer, 10, 4, 0, (StructOffset(mesh_instance, Model) + 3 * sizeof(vec4)), sizeof(mesh_instance));
    OpenGLInstanceAttribute(MeshBuffer->VAO, MeshBuffer->InstanceBuffer, 11, 3, 0, StructOffset(mesh_instance, Color), sizeof(mesh_instance));

    glCreateBuffers(1, &MeshBuffer->IndexBuffer);
    glNamedBufferStorage(MeshBuffer->IndexBuffer, IndexCount * sizeof(u32), Indices, 0);

    glVertexArrayElementBuffer(MeshBuffer->VAO, MeshBuffer->IndexBuffer);
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
    if (Bitmap->IsHDR) return GL_RGB16F;
    if (Bitmap->Channels == 1) return GL_R8;
    if (Bitmap->Channels == 2) return GL_RG8;
    if (Bitmap->Channels == 3) return GL_RGB8;
    if (Bitmap->Channels == 4) return GL_RGBA8;

    Assert(!"Invalid number of channels");

    return -1;
}

dummy_internal void
OpenGLAddTexture(opengl_state *State, u32 Id, bitmap *Bitmap)
{
    GLint Format = OpenGLGetTextureFormat(Bitmap);
    GLint InternalFormat = OpenGLGetTextureInternalFormat(Bitmap);

    GLuint TextureHandle;

    glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);

    glTextureParameteri(TextureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(TextureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(TextureHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(TextureHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTextureStorage2D(TextureHandle, 1, InternalFormat, Bitmap->Width, Bitmap->Height);

    if (Bitmap->IsHDR)
    {
        glTextureSubImage2D(TextureHandle, 0, 0, 0, Bitmap->Width, Bitmap->Height, Format, GL_FLOAT, Bitmap->Pixels);
    }
    else
    {
        glTextureSubImage2D(TextureHandle, 0, 0, 0, Bitmap->Width, Bitmap->Height, Format, GL_UNSIGNED_BYTE, Bitmap->Pixels);
    }

    //glGenerateTextureMipmap(TextureHandle);

    opengl_texture *Texture = HashTableLookup(&State->Textures, Id);

    Assert(IsSlotEmpty(Texture->Key));

    Texture->Key = Id;
    Texture->Handle = TextureHandle;
    Texture->Width = Bitmap->Width;
    Texture->Height = Bitmap->Height;
}

inline opengl_uniform *
OpengLGetUniform(opengl_shader *Shader, const char *UniformName)
{
    opengl_uniform *Uniform = HashTableLookup(&Shader->Uniforms, (char *)UniformName);
    return Uniform;
}

inline GLint
OpengLGetUniformLocation(opengl_shader *Shader, const char *UniformName)
{
    opengl_uniform *Uniform = HashTableLookup(&Shader->Uniforms, (char *)UniformName);

    return Uniform->Location;
}

dummy_internal void
OpenGLLoadShaderUniforms(opengl_shader *Shader, memory_arena *Arena)
{
    GLuint Program = Shader->Program;

    GLint UniformCount;
    glGetProgramiv(Program, GL_ACTIVE_UNIFORMS, &UniformCount);

    Assert(UniformCount <= OPENGL_UNIFORM_MAX_COUNT);

    GLint UniformMaxLength;
    glGetProgramiv(Program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &UniformMaxLength);

    Assert(UniformMaxLength <= OPENGL_UNIFORM_MAX_LENGTH);

    Shader->UniformCount = UniformCount;
    Shader->Uniforms.Count = OPENGL_UNIFORM_MAX_COUNT;
    Shader->Uniforms.Values = PushArray(Arena, Shader->Uniforms.Count, opengl_uniform);

    for (i32 UniformIndex = 0; UniformIndex < UniformCount; ++UniformIndex)
    {
        GLsizei UniformLength;
        GLint UniformSize;
        GLenum 	UniformType;
        char UniformName[OPENGL_UNIFORM_MAX_LENGTH];
        glGetActiveUniform(Program, UniformIndex, UniformMaxLength, &UniformLength, &UniformSize, &UniformType, UniformName);

        GLint UniformLocation = glGetUniformLocation(Program, UniformName);

        opengl_uniform *Uniform = HashTableLookup(&Shader->Uniforms, UniformName);
        CopyString(UniformName, Uniform->Key);
        Uniform->Location = UniformLocation;
        Uniform->Size = UniformSize;
    }
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
        OPENGL_MAX_JOINT_COUNT,
        OPENGL_MAX_WEIGHT_COUNT
    );

    return Result;
}

dummy_internal char **
OpenGLLoadShaderFile(opengl_state *State, u32 Id, const char *ShaderFileName, u32 Count, memory_arena *Arena)
{
    char **Sources = PushArray(Arena, Count, char *);

    for (u32 FileIndex = 0; FileIndex < OPENGL_COMMON_SHADER_COUNT; ++FileIndex)
    {
        read_file_result CommonShaderFile = State->Platform->ReadFile((char *)OpenGLCommonShaders[FileIndex], Arena, ReadText());
        Sources[FileIndex] = OpenGLPreprocessShader((char *)CommonShaderFile.Contents, CommonShaderFile.Size, Arena);
    }

    read_file_result ShaderFile = State->Platform->ReadFile((char *)ShaderFileName, Arena, ReadText());
    Sources[OPENGL_COMMON_SHADER_COUNT] = (char *)ShaderFile.Contents;

    return Sources;
}

dummy_internal void
OpenGLLoadShader(opengl_state *State, opengl_load_shader_params Params)
{
    opengl_shader *Shader = HashTableLookup(&State->Shaders, Params.ShaderId);

    Assert(IsSlotEmpty(Shader->Key));

    {
        scoped_memory ScopedMemory(State->Arena);

        u32 Count = OPENGL_COMMON_SHADER_COUNT + 1;

        u32 ShaderCount = 0;
        GLuint Shaders[4] = {};

        if (Params.VertexShaderFileName)
        {
            char **Source = OpenGLLoadShaderFile(State, Params.ShaderId, Params.VertexShaderFileName, Count, ScopedMemory.Arena);
            Shaders[ShaderCount++] = OpenGLCreateShader(GL_VERTEX_SHADER, Count, Source);

            CopyString(Params.VertexShaderFileName, Shader->VertexShader.FileName);
            Shader->VertexShader.LastWriteTime = Win32GetLastWriteTime(Shader->VertexShader.FileName);
        }

        if (Params.GeometryShaderFileName)
        {
            char **Source = OpenGLLoadShaderFile(State, Params.ShaderId, Params.GeometryShaderFileName, Count, ScopedMemory.Arena);
            Shaders[ShaderCount++] = OpenGLCreateShader(GL_GEOMETRY_SHADER, Count, Source);

            CopyString(Params.GeometryShaderFileName, Shader->GeometryShader.FileName);
            Shader->GeometryShader.LastWriteTime = Win32GetLastWriteTime(Shader->GeometryShader.FileName);
        }

        if (Params.FragmentShaderFileName)
        {
            char **Source = OpenGLLoadShaderFile(State, Params.ShaderId, Params.FragmentShaderFileName, Count, ScopedMemory.Arena);
            Shaders[ShaderCount++] = OpenGLCreateShader(GL_FRAGMENT_SHADER, Count, Source);

            CopyString(Params.FragmentShaderFileName, Shader->FragmentShader.FileName);
            Shader->FragmentShader.LastWriteTime = Win32GetLastWriteTime(Shader->FragmentShader.FileName);
        }

        if (Params.ComputeShaderFileName)
        {
            char **Source = OpenGLLoadShaderFile(State, Params.ShaderId, Params.ComputeShaderFileName, Count, ScopedMemory.Arena);
            Shaders[ShaderCount++] = OpenGLCreateShader(GL_COMPUTE_SHADER, Count, Source);

            CopyString(Params.ComputeShaderFileName, Shader->ComputeShader.FileName);
            Shader->ComputeShader.LastWriteTime = Win32GetLastWriteTime(Shader->ComputeShader.FileName);
        }

        GLuint Program = OpenGLCreateProgram(ShaderCount, Shaders);

        Shader->Key = Params.ShaderId;
        Shader->Program = Program;
    }

    for (u32 FileIndex = 0; FileIndex < ArrayCount(Shader->CommonShaders); ++FileIndex)
    {
        win32_shader_file *CommonShader = Shader->CommonShaders + FileIndex;

        CopyString(OpenGLCommonShaders[FileIndex], CommonShader->FileName);
        CommonShader->LastWriteTime = Win32GetLastWriteTime(CommonShader->FileName);
    }

    OpenGLLoadShaderUniforms(Shader, State->Arena);
}

dummy_internal void
OpenGLReloadShader(opengl_state *State, u32 ShaderId)
{
    opengl_shader *Shader = HashTableLookup(&State->Shaders, ShaderId);

    Assert(!IsSlotEmpty(Shader->Key));

    GLuint Program = 0;

    {
        scoped_memory ScopedMemory(State->Arena);

        u32 Count = OPENGL_COMMON_SHADER_COUNT + 1;

        u32 ShaderCount = 0;
        GLuint Shaders[4] = {};

        if (!StringEquals(Shader->VertexShader.FileName, ""))
        {
            char **Source = OpenGLLoadShaderFile(State, ShaderId, Shader->VertexShader.FileName, Count, ScopedMemory.Arena);
            Shaders[ShaderCount++] = OpenGLCreateShader(GL_VERTEX_SHADER, Count, Source);
        }

        if (!StringEquals(Shader->GeometryShader.FileName, ""))
        {
            char **Source = OpenGLLoadShaderFile(State, ShaderId, Shader->GeometryShader.FileName, Count, ScopedMemory.Arena);
            Shaders[ShaderCount++] = OpenGLCreateShader(GL_GEOMETRY_SHADER, Count, Source);
        }

        if (!StringEquals(Shader->FragmentShader.FileName, ""))
        {
            char **Source = OpenGLLoadShaderFile(State, ShaderId, Shader->FragmentShader.FileName, Count, ScopedMemory.Arena);
            Shaders[ShaderCount++] = OpenGLCreateShader(GL_FRAGMENT_SHADER, Count, Source);
        }

        if (!StringEquals(Shader->ComputeShader.FileName, ""))
        {
            char **Source = OpenGLLoadShaderFile(State, ShaderId, Shader->ComputeShader.FileName, Count, ScopedMemory.Arena);
            Shaders[ShaderCount++] = OpenGLCreateShader(GL_COMPUTE_SHADER, Count, Source);
        }

        Program = OpenGLCreateProgram(ShaderCount, Shaders);
    }

    if (Program)
    {
        glDeleteProgram(Shader->Program);
        Shader->Program = Program;

        OpenGLLoadShaderUniforms(Shader, State->Arena);
    }
}

dummy_internal void
Win32GetShaderWriteTime(win32_shader_file *ShaderFile, bool32 *ShouldReload)
{
    if (!StringEquals(ShaderFile->FileName, ""))
    {
        FILETIME NewShaderWriteTime = Win32GetLastWriteTime(ShaderFile->FileName);
        if (CompareFileTime(&NewShaderWriteTime, &ShaderFile->LastWriteTime) != 0)
        {
            ShaderFile->LastWriteTime = NewShaderWriteTime;
            *ShouldReload = true;
        }
    }
}

dummy_internal void
OpenGLInitShaders(opengl_state *State)
{
    for (u32 ShaderIndex = 0; ShaderIndex < ArrayCount(OpenGLShaders); ++ShaderIndex)
    {
        OpenGLLoadShader(State, OpenGLShaders[ShaderIndex]);
    }
}

dummy_internal void
OpenGLBlinnPhongShading(opengl_state *State, opengl_render_options *Options, opengl_shader *Shader, mesh_material *MeshMaterial)
{
    // todo: magic numbers
    opengl_texture *Texture = OpenGLGetTexture(State, 0);
    glBindTextureUnit(0, Texture->Handle);

    for (u32 CascadeIndex = 0; CascadeIndex < 4; ++CascadeIndex)
    {
        // todo: magic 16?
        u32 TextureIndex = CascadeIndex + 16;

        // Cascasde Shadow Map
        glBindTextureUnit(TextureIndex, State->CascadeShadowMaps[CascadeIndex]);
        glUniform1i(OpengLGetUniformLocation(Shader, "u_CascadeShadowMaps[0]") + CascadeIndex, TextureIndex);

        // Cascade Bounds
        vec2 CascadeBounds = State->CascadeBounds[CascadeIndex];
        glUniform2f(OpengLGetUniformLocation(Shader, "u_CascadeBounds[0]") + CascadeIndex, CascadeBounds.x, CascadeBounds.y);

        // Cascade View Projection
        mat4 CascadeViewProjection = State->CascadeViewProjection[CascadeIndex];
        glUniformMatrix4fv(OpengLGetUniformLocation(Shader, "u_CascadeViewProjection[0]") + CascadeIndex, 1, GL_TRUE, (f32 *)CascadeViewProjection.Elements);
    }

    glUniform1i(OpengLGetUniformLocation(Shader, "u_EnableShadows"), Options->EnableShadows);
    glUniform1i(OpengLGetUniformLocation(Shader, "u_ShowCascades"), Options->ShowCascades);

    if (MeshMaterial)
    {
        glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.HasDiffuseMap"), false);
        glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.HasSpecularMap"), false);
        glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.HasShininessMap"), false);
        glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.HasNormalMap"), false);

        // default values
        vec3 DefaultAmbient = vec3(0.2f);
        f32 DefaultSpecularShininess = 30.f;
        glUniform3f(OpengLGetUniformLocation(Shader, "u_Material.AmbientColor"), DefaultAmbient.x, DefaultAmbient.y, DefaultAmbient.z);
        glUniform1f(OpengLGetUniformLocation(Shader, "u_Material.SpecularShininess"), DefaultSpecularShininess);

        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MeshMaterial->PropertyCount; ++MaterialPropertyIndex)
        {
            material_property *MaterialProperty = MeshMaterial->Properties + MaterialPropertyIndex;

            switch (MaterialProperty->Type)
            {
                case MaterialProperty_Float_Shininess:
                {
                    glUniform1f(OpengLGetUniformLocation(Shader, "u_Material.SpecularShininess"), MaterialProperty->Value);
                    break;
                }
                case MaterialProperty_Color_Ambient:
                {
                    if (MaterialProperty->Color.x == 0.f && MaterialProperty->Color.y == 0.f && MaterialProperty->Color.z == 0.f)
                    {
                        MaterialProperty->Color.rgb = DefaultAmbient;
                    }

                    glUniform3f(
                        OpengLGetUniformLocation(Shader, "u_Material.AmbientColor"),
                        MaterialProperty->Color.r,
                        MaterialProperty->Color.g,
                        MaterialProperty->Color.b
                    );
                    break;
                }
                case MaterialProperty_Color_Diffuse:
                {
                    glUniform3f(
                        OpengLGetUniformLocation(Shader, "u_Material.DiffuseColor"),
                        MaterialProperty->Color.r,
                        MaterialProperty->Color.g,
                        MaterialProperty->Color.b
                    );
                    break;
                }
                case MaterialProperty_Color_Specular:
                {
                    glUniform3f(
                        OpengLGetUniformLocation(Shader, "u_Material.SpecularColor"),
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

                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.HasDiffuseMap"), true);
                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.DiffuseMap"), MaterialPropertyIndex);
                    break;
                }
                case MaterialProperty_Texture_Specular:
                {
                    opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->TextureId);

                    glBindTextureUnit(MaterialPropertyIndex, Texture->Handle);

                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.HasSpecularMap"), true);
                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.SpecularMap"), MaterialPropertyIndex);
                    break;
                }
                case MaterialProperty_Texture_Shininess:
                {
                    opengl_texture *Texture = OpenGLGetTexture(State, MaterialProperty->TextureId);

                    glBindTextureUnit(MaterialPropertyIndex, Texture->Handle);

                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.HasShininessMap"), true);
                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.ShininessMap"), MaterialPropertyIndex);
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

                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.HasNormalMap"), true);
                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Material.NormalMap"), MaterialPropertyIndex);
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

dummy_internal void
OpenGLInitFramebuffers(opengl_state *State, i32 WindowWidth, i32 WindowHeight, u32 Samples)
{
    State->SourceFramebuffer = {};
    State->DestFramebuffer = {};
    State->EditorFramebuffer = {};

    State->SourceFramebuffer.Width = WindowWidth;
    State->SourceFramebuffer.Height = WindowHeight;
    State->SourceFramebuffer.Samples = Samples;

    State->DestFramebuffer.Width = WindowWidth;
    State->DestFramebuffer.Height = WindowHeight;

    State->EditorFramebuffer.Width = WindowWidth;
    State->EditorFramebuffer.Height = WindowHeight;

    // Source (multisampled) Framebuffer
    glCreateFramebuffers(1, &State->SourceFramebuffer.Handle);

    glCreateRenderbuffers(1, &State->SourceFramebuffer.ColorTarget);
    glNamedRenderbufferStorageMultisample(State->SourceFramebuffer.ColorTarget, Samples, GL_RGBA16F, WindowWidth, WindowHeight);
    glNamedFramebufferRenderbuffer(State->SourceFramebuffer.Handle, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, State->SourceFramebuffer.ColorTarget);

    glCreateRenderbuffers(1, &State->SourceFramebuffer.DepthStencilTarget);
    glNamedRenderbufferStorageMultisample(State->SourceFramebuffer.DepthStencilTarget, Samples, GL_DEPTH24_STENCIL8, WindowWidth, WindowHeight);
    glNamedFramebufferRenderbuffer(State->SourceFramebuffer.Handle, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, State->SourceFramebuffer.DepthStencilTarget);

    GLenum MultiSampledFramebufferStatus = glCheckNamedFramebufferStatus(State->SourceFramebuffer.Handle, GL_FRAMEBUFFER);
    Assert(MultiSampledFramebufferStatus == GL_FRAMEBUFFER_COMPLETE);

    // Dest Framebuffer
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

    // Editor Framebuffer
    glCreateFramebuffers(1, &State->EditorFramebuffer.Handle);

    glCreateTextures(GL_TEXTURE_2D, 1, &State->EditorFramebuffer.ColorTarget);
    glTextureStorage2D(State->EditorFramebuffer.ColorTarget, 1, GL_RGBA16F, WindowWidth, WindowHeight);

    glTextureParameteri(State->EditorFramebuffer.ColorTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(State->EditorFramebuffer.ColorTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(State->EditorFramebuffer.ColorTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(State->EditorFramebuffer.ColorTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(State->EditorFramebuffer.Handle, GL_COLOR_ATTACHMENT0, State->EditorFramebuffer.ColorTarget, 0);

    GLenum FinalFramebufferStatus = glCheckNamedFramebufferStatus(State->EditorFramebuffer.Handle, GL_FRAMEBUFFER);
    Assert(FinalFramebufferStatus == GL_FRAMEBUFFER_COMPLETE);
}

dummy_internal void
OpenGLOnWindowResize(opengl_state *State, i32 WindowWidth, i32 WindowHeight, u32 Samples)
{
    State->WindowWidth = WindowWidth;
    State->WindowHeight = WindowHeight;

    glDeleteFramebuffers(1, &State->SourceFramebuffer.Handle);
    glDeleteRenderbuffers(1, &State->SourceFramebuffer.ColorTarget);
    glDeleteRenderbuffers(1, &State->SourceFramebuffer.DepthStencilTarget);

    glDeleteFramebuffers(1, &State->DestFramebuffer.Handle);
    glDeleteTextures(1, &State->DestFramebuffer.ColorTarget);

    glDeleteFramebuffers(1, &State->EditorFramebuffer.Handle);
    glDeleteTextures(1, &State->EditorFramebuffer.ColorTarget);

    OpenGLInitFramebuffers(State, WindowWidth, WindowHeight, Samples);
}

dummy_internal void
OpenGLLogMessage(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, const GLchar *Message, const void *UserParam)
{
    stream *Stream = (stream *)UserParam;

    char SourceString[32];
    char TypeString[32];
    char SeverityString[32];

    switch (Source)
    {
        case GL_DEBUG_SOURCE_API:
        {
            CopyString("API", SourceString);
            break;
        };
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        {
            CopyString("WINDOW SYSTEM", SourceString);
            break;
        };
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        {
            CopyString("SHADER COMPILER", SourceString);
            break;
        };
        case GL_DEBUG_SOURCE_THIRD_PARTY:
        {
            CopyString("THIRD PARTY", SourceString);
            break;
        };
        case GL_DEBUG_SOURCE_APPLICATION:
        {
            CopyString("APPLICATION", SourceString);
        };
        case GL_DEBUG_SOURCE_OTHER:
        {
            CopyString("OTHER", SourceString);
            break;
        };
    }

    switch (Type)
    {
        case GL_DEBUG_TYPE_ERROR:
        {
            CopyString("ERROR", TypeString);
            break;
        };
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        {
            CopyString("DEPRECATED_BEHAVIOR", TypeString);
            break;
        };
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        {
            CopyString("UNDEFINED_BEHAVIOR", TypeString);
            break;
        };
        case GL_DEBUG_TYPE_PORTABILITY:
        {
            CopyString("PORTABILITY", TypeString);
            break;
        };
        case GL_DEBUG_TYPE_PERFORMANCE:
        {
            CopyString("PERFORMANCE", TypeString);
            break;
        };
        case GL_DEBUG_TYPE_MARKER:
        {
            CopyString("MARKER", TypeString);
            break;
        };
        case GL_DEBUG_TYPE_OTHER:
        {
            CopyString("OTHER", TypeString);
            break;
        };
    }

    switch (Severity)
    {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        {
            CopyString("NOTIFICATION", SeverityString);
            break;
        };
        case GL_DEBUG_SEVERITY_LOW:
        {
            CopyString("LOW", SeverityString);
            break;
        };
        case GL_DEBUG_SEVERITY_MEDIUM:
        {
            CopyString("MEDIUM", SeverityString);
            break;
        };
        case GL_DEBUG_SEVERITY_HIGH:
        {
            CopyString("HIGH", SeverityString);
            break;
        };
    }

    Out(Stream, "%s: %s - %s: %s", SourceString, TypeString, SeverityString, Message);
}

dummy_internal void
OpenGLInitRenderer(opengl_state *State, i32 WindowWidth, i32 WindowHeight, u32 Samples)
{
    State->Vendor = (char *)glGetString(GL_VENDOR);
    State->Renderer = (char *)glGetString(GL_RENDERER);
    State->Version = (char *)glGetString(GL_VERSION);
    State->ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

    InitHashTable(&State->MeshBuffers, 1021, State->Arena);
    InitHashTable(&State->SkinningBuffers, 509, State->Arena);
    InitHashTable(&State->Textures, 127, State->Arena);
    InitHashTable(&State->Shaders, 61, State->Arena);

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

    glCreateBuffers(1, &State->TransformUBO);
    glNamedBufferStorage(State->TransformUBO, sizeof(opengl_uniform_buffer_transform), 0, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, State->TransformUBO);

    glCreateBuffers(1, &State->ShadingUBO);
    glNamedBufferStorage(State->ShadingUBO, sizeof(opengl_uniform_buffer_shading), 0, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, State->ShadingUBO);

    // todo: cleanup
    bitmap WhiteTexture = {};
    WhiteTexture.Width = 1;
    WhiteTexture.Height = 1;
    WhiteTexture.Channels = 4;
    u32 *WhitePixel = PushType(State->Arena, u32);
    *WhitePixel = 0xFFFFFFFF;
    WhiteTexture.Pixels = WhitePixel;

    OpenGLAddTexture(State, 0, &WhiteTexture);

#if 0
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
#endif

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_MULTISAMPLE);

    // todo: use GL_ZERO_TO_ONE?
    glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);

#if DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, GL_FALSE);
    glDebugMessageCallback(OpenGLLogMessage, State->Stream);
#endif
}

dummy_internal void
OpenGLPrepareScene(opengl_state *State, render_commands *Commands)
{
    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
            case RenderCommand_AddMesh:
            {
                render_command_add_mesh *Command = (render_command_add_mesh *)Entry;

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
                render_command_add_skinning_buffer *Command = (render_command_add_skinning_buffer *)Entry;

                OpenGLAddSkinningBuffer(State, Command->SkinningBufferId, Command->SkinningMatrixCount);

                break;
            }
            case RenderCommand_AddSkybox:
            {
                render_command_add_skybox *Command = (render_command_add_skybox *)Entry;

                bitmap EquirectBitmap = Command->EquirectEnvMap->Bitmap;

                GLuint EquirectEnvTexture;
                glCreateTextures(GL_TEXTURE_2D, 1, &EquirectEnvTexture);
                glTextureStorage2D(EquirectEnvTexture, 1, GL_RGB16F, EquirectBitmap.Width, EquirectBitmap.Height);
                glTextureSubImage2D(EquirectEnvTexture, 0, 0, 0, EquirectBitmap.Width, EquirectBitmap.Height, GL_RGB, GL_FLOAT, EquirectBitmap.Pixels);
                glTextureParameteri(EquirectEnvTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(EquirectEnvTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                GLuint EnvTexture;
                glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &EnvTexture);
                glTextureStorage2D(EnvTexture, 1, GL_RGBA16F, Command->EnvMapSize, Command->EnvMapSize);
                glTextureParameteri(EnvTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(EnvTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                opengl_shader *Equirect2CubemapShader = OpenGLGetShader(State, OPENGL_EQUIRECT_TO_CUBEMAP_SHADER_ID);

                glUseProgram(Equirect2CubemapShader->Program);
                glBindTextureUnit(0, EquirectEnvTexture);
                glBindImageTexture(0, EnvTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
                glDispatchCompute(Command->EnvMapSize / 32, Command->EnvMapSize / 32, 6);

                glDeleteTextures(1, &EquirectEnvTexture);

                opengl_texture *Texture = HashTableLookup(&State->Textures, Command->SkyboxId);
                Assert(IsSlotEmpty(Texture->Key));

                Texture->Key = Command->SkyboxId;
                Texture->Handle = EnvTexture;

                break;
            }
            case RenderCommand_DrawSkinnedMesh:
            {
                render_command_draw_skinned_mesh *Command = (render_command_draw_skinned_mesh *) Entry;

                opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);
                opengl_skinning_buffer *SkinningBuffer = OpenGLGetSkinningBuffer(State, Command->SkinningBufferId);

                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SKINNED_SHADER_ID);

                glUseProgram(Shader->Program);

                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, MeshBuffer->PositionsBuffer);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, MeshBuffer->WeightsBuffer);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, MeshBuffer->JointIndicesBuffer);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, MeshBuffer->SkinningMatricesBuffer);

                glNamedBufferSubData(SkinningBuffer->SkinningTBO, 0, Command->SkinningMatrixCount * sizeof(mat4), Command->SkinningMatrices);
                glBindTextureUnit(0, SkinningBuffer->SkinningTBOTexture);
                glTextureBuffer(SkinningBuffer->SkinningTBOTexture, GL_RGBA32F, SkinningBuffer->SkinningTBO);

                glUniform1i(OpengLGetUniformLocation(Shader, "u_SkinningMatricesSampler"), 0);

                glDispatchCompute(MeshBuffer->VertexCount, 1, 1);

                break;
            }
            case RenderCommand_DrawSkinnedMeshInstanced:
            {
                render_command_draw_skinned_mesh_instanced *Command = (render_command_draw_skinned_mesh_instanced *) Entry;

                opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);
                opengl_skinning_buffer *SkinningBuffer = OpenGLGetSkinningBuffer(State, Command->SkinningBufferId);
                scoped_memory ScopedMemory(State->Arena);

                mat4 *SkinningMatrices = PushArray(ScopedMemory.Arena, Command->InstanceCount * OPENGL_MAX_JOINT_COUNT, mat4, Align(16));

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
                    glNamedBufferData(SkinningBuffer->SkinningTBO, MeshBuffer->InstanceCount * OPENGL_MAX_JOINT_COUNT * sizeof(mat4), 0, GL_STREAM_DRAW);
                    glNamedBufferData(MeshBuffer->SkinningMatricesBuffer, MeshBuffer->InstanceCount * MeshBuffer->VertexCount * sizeof(mat4), 0, GL_STREAM_DRAW);
                }

                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SKINNED_INSTANCED_SHADER_ID);

                glUseProgram(Shader->Program);

                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, MeshBuffer->PositionsBuffer);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, MeshBuffer->WeightsBuffer);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, MeshBuffer->JointIndicesBuffer);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, MeshBuffer->SkinningMatricesBuffer);

                glNamedBufferSubData(SkinningBuffer->SkinningTBO, 0, Command->InstanceCount * OPENGL_MAX_JOINT_COUNT * sizeof(mat4), SkinningMatrices);
                glBindTextureUnit(0, SkinningBuffer->SkinningTBOTexture);
                glTextureBuffer(SkinningBuffer->SkinningTBOTexture, GL_RGBA32F, SkinningBuffer->SkinningTBO);

                glUniform1i(OpengLGetUniformLocation(Shader, "u_SkinningMatricesSampler"), 0);

                glDispatchCompute(MeshBuffer->VertexCount, Command->InstanceCount, 1);

                break;
            }
        }

        BaseAddress += Entry->Size;
    }

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

dummy_internal void
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

        mat4 LightViewProjection = Options->CascadeProjection * Options->CascadeView;
        mat4 TransposeLightViewProjection = Transpose(LightViewProjection);

        glNamedBufferSubData(State->TransformUBO, StructOffset(opengl_uniform_buffer_transform, ViewProjection), sizeof(mat4), &TransposeLightViewProjection);
    }

    if (Options->WireframeMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
            case RenderCommand_SetViewport:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_viewport *Command = (render_command_set_viewport *)Entry;

                    glViewport(Command->x, Command->y, Command->Width, Command->Height);
                }

                break;
            }
            case RenderCommand_SetScreenProjection:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_screen_projection *Command = (render_command_set_screen_projection *)Entry;

                    mat4 Projection = OrthographicProjection(Command->Left, Command->Right, Command->Bottom, Command->Top, Command->Near, Command->Far);
                    mat4 TransposeProjection = Transpose(Projection);

                    glNamedBufferSubData(State->TransformUBO, StructOffset(opengl_uniform_buffer_transform, ScreenProjection), sizeof(mat4), &TransposeProjection);
                }

                break;
            }
            case RenderCommand_SetViewProjection:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_view_projection *Command = (render_command_set_view_projection *)Entry;

                    game_camera *Camera = Command->Camera;

                    vec3 CameraPosition = Camera->Position;
                    vec3 CameraDirection = -Camera->Direction;
                    vec3 CameraTarget = CameraPosition + Camera->Direction;

                    mat4 Projection = FrustrumProjection(Camera->FieldOfView, Camera->AspectRatio, Camera->NearClipPlane, Camera->FarClipPlane);
                    mat4 View = LookAt(CameraPosition, CameraTarget, Camera->Up);

                    mat4 ViewProjection = Projection * View;
                    mat4 TransposeViewProjection = Transpose(ViewProjection);

                    mat4 ViewRotation = RemoveTranslation(View);
                    mat4 SkyProjection = Projection * ViewRotation;
                    mat4 TransposeSkyProjection = Transpose(SkyProjection);

                    glNamedBufferSubData(State->TransformUBO, StructOffset(opengl_uniform_buffer_transform, ViewProjection), sizeof(mat4), &TransposeViewProjection);
                    glNamedBufferSubData(State->TransformUBO, StructOffset(opengl_uniform_buffer_transform, SkyProjection), sizeof(mat4), &TransposeSkyProjection);
                    glNamedBufferSubData(State->TransformUBO, StructOffset(opengl_uniform_buffer_transform, CameraPosition), sizeof(vec3), &CameraPosition);
                    glNamedBufferSubData(State->TransformUBO, StructOffset(opengl_uniform_buffer_transform, CameraDirection), sizeof(vec3), &CameraDirection);
                }

                break;
            }
            case RenderCommand_SetDirectionalLight:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_directional_light *Command = (render_command_set_directional_light *)Entry;

                    opengl_directional_light DirectinalLight = {};
                    DirectinalLight.LightDirection = Command->Light.Direction;
                    DirectinalLight.LightColor = Command->Light.Color;

                    glNamedBufferSubData(State->ShadingUBO, StructOffset(opengl_uniform_buffer_shading, DirectinalLight), sizeof(opengl_directional_light), &DirectinalLight);
                }

                break;
            }
            case RenderCommand_SetPointLights:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_set_point_lights *Command = (render_command_set_point_lights *)Entry;

                    scoped_memory ScopedMemory(State->Arena);

                    opengl_point_light *PointLights = PushArray(ScopedMemory.Arena, Command->PointLightCount, opengl_point_light);

                    for (u32 PointLightIndex = 0; PointLightIndex < Command->PointLightCount; ++PointLightIndex)
                    {
                        point_light *Source = Command->PointLights + PointLightIndex;
                        opengl_point_light *Dest = PointLights + PointLightIndex;

                        Dest->Position = Source->Position;
                        Dest->Color = Source->Color;
                        Dest->Attenuation = vec3(Source->Attenuation.Constant, Source->Attenuation.Linear, Source->Attenuation.Quadratic);
                    }

                    glNamedBufferSubData(State->ShadingUBO, StructOffset(opengl_uniform_buffer_shading, PointLightCount), sizeof(u32), &Command->PointLightCount);
                    glNamedBufferSubData(State->ShadingUBO, StructOffset(opengl_uniform_buffer_shading, PointLights), Command->PointLightCount * sizeof(opengl_point_light), PointLights);
                }

                break;
            }
            case RenderCommand_SetTime:
            {
                render_command_set_time *Command = (render_command_set_time *)Entry;

                glNamedBufferSubData(State->TransformUBO, StructOffset(opengl_uniform_buffer_transform, Time), sizeof(f32), &Command->Time);

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
                    render_command_draw_point *Command = (render_command_draw_point *)Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                    glPointSize(Command->Size);

                    // todo: use separate PointVAO ?
                    glBindVertexArray(State->Line.VAO);
                    glUseProgram(Shader->Program);
                    {
                        mat4 Model = Translate(Command->Position);

                        // todo: world mode?
                        glUniform1i(OpengLGetUniformLocation(Shader, "u_Mode"), OPENGL_WORLD_SPACE_MODE);
                        glUniformMatrix4fv(OpengLGetUniformLocation(Shader, "u_Model"), 1, GL_TRUE, (f32 *)Model.Elements);
                        glUniform4f(OpengLGetUniformLocation(Shader, "u_Color"), Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
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
                    render_command_draw_line *Command = (render_command_draw_line *)Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                    glLineWidth(Command->Thickness);

                    glBindVertexArray(State->Line.VAO);
                    glUseProgram(Shader->Program);
                    {
                        mat4 T = Translate(Command->Start);
                        mat4 S = Scale(Command->End - Command->Start);
                        mat4 Model = T * S;

                        // todo: world mode?
                        glUniform1i(OpengLGetUniformLocation(Shader, "u_Mode"), OPENGL_WORLD_SPACE_MODE);
                        glUniformMatrix4fv(OpengLGetUniformLocation(Shader, "u_Model"), 1, GL_TRUE, (f32 *)Model.Elements);
                        glUniform4f(OpengLGetUniformLocation(Shader, "u_Color"), Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
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
                    render_command_draw_rectangle *Command = (render_command_draw_rectangle *)Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                    glBindVertexArray(State->Rectangle.VAO);
                    glUseProgram(Shader->Program);

                    mat4 Model = Transform(Command->Transform);

                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Mode"), OPENGL_SCREEN_SPACE_MODE);
                    glUniformMatrix4fv(OpengLGetUniformLocation(Shader, "u_Model"), 1, GL_TRUE, (f32 *)Model.Elements);
                    glUniform4f(OpengLGetUniformLocation(Shader, "u_Color"), Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);

                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }

                break;
            }
            case RenderCommand_DrawBox:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_box *Command = (render_command_draw_box *)Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                    glBindVertexArray(State->Box.VAO);
                    glUseProgram(Shader->Program);

                    mat4 Model = Transform(Command->Transform);

                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Mode"), OPENGL_WORLD_SPACE_MODE);
                    glUniformMatrix4fv(OpengLGetUniformLocation(Shader, "u_Model"), 1, GL_TRUE, (f32 *)Model.Elements);
                    glUniform4f(OpengLGetUniformLocation(Shader, "u_Color"), Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);

                    // todo
                    if (!Options->WireframeMode)
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    }

                    glDrawArrays(GL_TRIANGLES, 0, 36);

                    if (!Options->WireframeMode)
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    }
                }

                break;
            }
            case RenderCommand_DrawText:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_text *Command = (render_command_draw_text *)Entry;
                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_TEXT_SHADER_ID);
                    scoped_memory ScopedMemory(State->Arena);

                    glBindVertexArray(State->Text.VAO);
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

                    glNamedBufferSubData(State->Text.VBO, 0, CharacterCount * sizeof(opengl_character_point), Points);

                    opengl_texture *Texture = OpenGLGetTexture(State, Font->TextureId);
                    glBindTextureUnit(0, Texture->Handle);

                    glUniform1i(OpengLGetUniformLocation(Shader, "u_Mode"), Mode);
                    glUniform4f(OpengLGetUniformLocation(Shader, "u_Color"), Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
                    glUniform3f(OpengLGetUniformLocation(Shader, "u_CameraXAxis"), CameraXAsis.x, CameraXAsis.y, CameraXAsis.z);
                    glUniform3f(OpengLGetUniformLocation(Shader, "u_CameraYAxis"), CameraYAxis.x, CameraYAxis.y, CameraYAxis.z);

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
                    render_command_draw_particles *Command = (render_command_draw_particles *)Entry;
                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PARTICLE_SHADER_ID);
                    scoped_memory ScopedMemory(State->Arena);

                    glBindVertexArray(State->Particle.VAO);
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

                    glNamedBufferSubData(State->Particle.VBO, 0, Command->ParticleCount * sizeof(opengl_particle), Particles);

                    if (Command->Texture)
                    {
                        opengl_texture *Texture = OpenGLGetTexture(State, Command->Texture->TextureId);
                        glBindTextureUnit(0, Texture->Handle);
                    }

                    glUniform3f(OpengLGetUniformLocation(Shader, "u_CameraXAxis"), CameraXAsis.x, CameraXAsis.y, CameraXAsis.z);
                    glUniform3f(OpengLGetUniformLocation(Shader, "u_CameraYAxis"), CameraYAxis.x, CameraYAxis.y, CameraYAxis.z);

                    glDrawArrays(GL_POINTS, 0, Command->ParticleCount);

                    glBindTextureUnit(0, 0);
                }

                break;
            }
            case RenderCommand_DrawTexturedQuad:
            {
                render_command_draw_textured_quad *Command = (render_command_draw_textured_quad *)Entry;
                opengl_shader *Shader = OpenGLGetShader(State, OPENGL_TEXTURED_QUAD_SHADER_ID);

                glBindVertexArray(State->Rectangle.VAO);
                glUseProgram(Shader->Program);

                mat4 Model = Transform(Command->Transform);

                glUniformMatrix4fv(OpengLGetUniformLocation(Shader, "u_Model"), 1, GL_TRUE, (f32 *)Model.Elements);
                // todo: param
                glUniform1i(OpengLGetUniformLocation(Shader, "u_Mode"), OPENGL_WORLD_SPACE_MODE);

                opengl_texture *Texture = OpenGLGetTexture(State, Command->Texture->TextureId);
                glBindTextureUnit(0, Texture->Handle);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glBindTextureUnit(0, 0);

                break;
            }
            case RenderCommand_DrawBillboard:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_billboard *Command = (render_command_draw_billboard *)Entry;
                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_BILLBOARD_SHADER_ID);

                    glBindVertexArray(State->Rectangle.VAO);
                    glUseProgram(Shader->Program);

                    mat4 WorldToCamera = Commands->Settings.WorldToCamera;
                    vec3 CameraXAsis = WorldToCamera[0].xyz;
                    vec3 CameraYAxis = WorldToCamera[1].xyz;

                    glUniform3f(OpengLGetUniformLocation(Shader, "u_Position"), Command->Position.x, Command->Position.y, Command->Position.z);
                    glUniform2f(OpengLGetUniformLocation(Shader, "u_Size"), Command->Size.x, Command->Size.y);
                    glUniform4f(OpengLGetUniformLocation(Shader, "u_Color"), Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
                    glUniform3f(OpengLGetUniformLocation(Shader, "u_CameraXAxis"), CameraXAsis.x, CameraXAsis.y, CameraXAsis.z);
                    glUniform3f(OpengLGetUniformLocation(Shader, "u_CameraYAxis"), CameraYAxis.x, CameraYAxis.y, CameraYAxis.z);
                    glUniform1i(OpengLGetUniformLocation(Shader, "u_HasTexture"), 0);

                    if (Command->Texture)
                    {
                        opengl_texture *Texture = OpenGLGetTexture(State, Command->Texture->TextureId);
                        glBindTextureUnit(0, Texture->Handle);
                        glUniform1i(OpengLGetUniformLocation(Shader, "u_HasTexture"), 1);
                    }

                    glDrawArrays(GL_POINTS, 0, 1);

                    glBindTextureUnit(0, 0);
                }

                break;
            }
            case RenderCommand_DrawGrid:
            {
                if (!Options->RenderShadowMap)
                {
                    // http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
                    // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders
                    render_command_draw_grid *Command = (render_command_draw_grid *)Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_GRID_SHADER_ID);

                    glBindVertexArray(State->Rectangle.VAO);
                    glUseProgram(Shader->Program);

                    OpenGLBlinnPhongShading(State, Options, Shader, 0);

                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
                        case MaterialType_Basic:
                        {
                            material Material = Command->Material;

                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_COLOR_SHADER_ID);

                            mat4 Model = Transform(Command->Transform);

                            glUseProgram(Shader->Program);
                            glUniformMatrix4fv(OpengLGetUniformLocation(Shader, "u_Model"), 1, GL_TRUE, (f32 *)Model.Elements);
                            glUniform4f(OpengLGetUniformLocation(Shader, "u_Color"), Material.Color.r, Material.Color.g, Material.Color.b, Material.Color.a);

                            break;
                        }
                        case MaterialType_Phong:
                        {
                            mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                            mat4 Model = Transform(Command->Transform);

                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_SHADER_ID);

                            glUseProgram(Shader->Program);
                            glUniformMatrix4fv(OpengLGetUniformLocation(Shader, "u_Model"), 1, GL_TRUE, (f32 *)Model.Elements);
                            OpenGLBlinnPhongShading(State, Options, Shader, Command->Material.MeshMaterial);

                            break;
                        }
                        default:
                        {
                            Assert(!"Not Implemented");
                            break;
                        }
                    }

                    glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
                }

                break;
            }
            case RenderCommand_DrawMeshInstanced:
            {
                render_command_draw_mesh_instanced *Command = (render_command_draw_mesh_instanced *)Entry;

                if ((Options->RenderShadowMap && Command->Material.CastShadow) || !Options->RenderShadowMap)
                {
                    opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);
                    mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                    glBindVertexArray(MeshBuffer->VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, MeshBuffer->InstanceBuffer);

                    if (MeshBuffer->InstanceCount < Command->InstanceCount)
                    {
                        MeshBuffer->InstanceCount = (u32)(Command->InstanceCount * 1.5f);
                        glNamedBufferData(MeshBuffer->InstanceBuffer, MeshBuffer->InstanceCount * sizeof(mesh_instance), 0, GL_STREAM_DRAW);
                    }

                    glNamedBufferSubData(MeshBuffer->InstanceBuffer, 0, Command->InstanceCount * sizeof(mesh_instance), Command->Instances);

                    switch (Command->Material.Type)
                    {
                        case MaterialType_Phong:
                        {
                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_INSTANCED_SHADER_ID);

                            glUseProgram(Shader->Program);
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
                }

                break;
            }
            case RenderCommand_DrawSkinnedMesh:
            {
                render_command_draw_skinned_mesh *Command = (render_command_draw_skinned_mesh *)Entry;

                if ((Options->RenderShadowMap && Command->Material.CastShadow) || !Options->RenderShadowMap)
                {
                    opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);
                    mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                    glBindVertexArray(MeshBuffer->VAO);

                    switch (Command->Material.Type)
                    {
                        case MaterialType_Phong:
                        {
                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_SKINNED_SHADER_ID);

                            glUseProgram(Shader->Program);
                            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, MeshBuffer->SkinningMatricesBuffer);
                            OpenGLBlinnPhongShading(State, Options, Shader, Command->Material.MeshMaterial);

                            break;
                        }
                        default:
                        {
                            Assert(!"Not Implemented");
                            break;
                        }
                    }

                    glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
                }

                break;
            }
            case RenderCommand_DrawSkinnedMeshInstanced:
            {
                render_command_draw_skinned_mesh_instanced *Command = (render_command_draw_skinned_mesh_instanced *)Entry;

                if ((Options->RenderShadowMap && Command->Material.CastShadow) || !Options->RenderShadowMap)
                {
                    opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->MeshId);
                    mesh_material *MeshMaterial = Command->Material.MeshMaterial;

                    glBindVertexArray(MeshBuffer->VAO);

                    switch (Command->Material.Type)
                    {
                        case MaterialType_Phong:
                        {
                            opengl_shader *Shader = OpenGLGetShader(State, OPENGL_PHONG_SKINNED_INSTANCED_SHADER_ID);

                            glUseProgram(Shader->Program);
                            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, MeshBuffer->SkinningMatricesBuffer);
                            glUniform1i(OpengLGetUniformLocation(Shader, "u_VertexCount"), MeshBuffer->VertexCount);
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
                }

                break;
            }
            case RenderCommand_DrawSkybox:
            {
                if (!Options->RenderShadowMap)
                {
                    render_command_draw_skybox *Command = (render_command_draw_skybox *)Entry;

                    opengl_shader *Shader = OpenGLGetShader(State, OPENGL_SKYBOX_SHADER_ID);
                    opengl_texture *Texture = OpenGLGetTexture(State, Command->SkyboxId);

                    glDisable(GL_CULL_FACE);
                    glDisable(GL_DEPTH_TEST);

                    glUseProgram(Shader->Program);
                    glBindTextureUnit(0, Texture->Handle);

                    glBindVertexArray(State->Box.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, 36);

                    glEnable(GL_CULL_FACE);
                    glEnable(GL_DEPTH_TEST);
                }

                break;
            }
        }

        BaseAddress += Entry->Size;
    }

    if (Options->WireframeMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

dummy_internal void
OpenGLProcessRenderCommands(opengl_state *State, render_commands *Commands)
{
    //PROFILE(State->Profiler, "OpenGLProcessRenderCommands");

    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);
        //Out(&State->Stream, "RenderCommand::%s", RenderCommandNames[Entry->Type]);

        BaseAddress += Entry->Size;
    }

#if OPENGL_RELOADABLE_SHADERS
    {
        PROFILE(State->Profiler, "OpenGLReloadableShaders");

        for (u32 ShaderIndex = 0; ShaderIndex < State->Shaders.Count; ++ShaderIndex)
        {
            opengl_shader *Shader = State->Shaders.Values + ShaderIndex;

            if (!IsSlotEmpty(Shader->Key))
            {
                bool32 ShouldReload = false;

                for (u32 FileIndex = 0; FileIndex < ArrayCount(Shader->CommonShaders); ++FileIndex)
                {
                    win32_shader_file *CommonShader = Shader->CommonShaders + FileIndex;
                    Win32GetShaderWriteTime(CommonShader, &ShouldReload);
                }

                Win32GetShaderWriteTime(&Shader->VertexShader, &ShouldReload);
                Win32GetShaderWriteTime(&Shader->GeometryShader, &ShouldReload);
                Win32GetShaderWriteTime(&Shader->FragmentShader, &ShouldReload);
                Win32GetShaderWriteTime(&Shader->ComputeShader, &ShouldReload);

                if (ShouldReload)
                {
                    OpenGLReloadShader(State, Shader->Key);
                }
            }
        }
    }
#endif

    render_commands_settings *RenderSettings = &Commands->Settings;

    if (State->WindowWidth != RenderSettings->WindowWidth || State->WindowHeight != RenderSettings->WindowHeight)
    {
        OpenGLOnWindowResize(State, RenderSettings->WindowWidth, RenderSettings->WindowHeight, RenderSettings->Samples);
    }

    {
        PROFILE(State->Profiler, "OpenGLPrepareScene");
        OpenGLPrepareScene(State, Commands);
    }

    if (RenderSettings->EnableShadows)
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
    }

    {
        PROFILE(State->Profiler, "OpenGLRenderScene");

        opengl_render_options RenderOptions = {};
        RenderOptions.ShowCascades = RenderSettings->ShowCascades;
        RenderOptions.EnableShadows = RenderSettings->EnableShadows;
        RenderOptions.WireframeMode = RenderSettings->WireframeMode;
        OpenGLRenderScene(State, Commands, &RenderOptions);
    }

    {
        PROFILE(State->Profiler, "OpenGLResolveFramebuffer");

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
        //glInvalidateNamedFramebufferData(State->SourceFramebuffer.Handle, ArrayCount(Attachments), Attachments);

        // Draw a full screen triangle for postprocessing
#if EDITOR
        glBindFramebuffer(GL_FRAMEBUFFER, State->EditorFramebuffer.Handle);

        opengl_shader *Shader = OpenGLGetShader(State, OPENGL_FRAMEBUFFER_SHADER_ID);

        glUseProgram(Shader->Program);
        glBindTextureUnit(0, State->DestFramebuffer.ColorTarget);
        glBindVertexArray(State->Rectangle.VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, RenderSettings->WindowWidth, RenderSettings->WindowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, RenderSettings->WindowWidth, RenderSettings->WindowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        opengl_shader *Shader = OpenGLGetShader(State, OPENGL_FRAMEBUFFER_SHADER_ID);

        glUseProgram(Shader->Program);
        glBindTextureUnit(0, State->DestFramebuffer.ColorTarget);
        glBindVertexArray(State->Rectangle.VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
    }
}
