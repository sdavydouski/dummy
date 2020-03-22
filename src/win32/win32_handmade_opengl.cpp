#include <glad/glad.h>
#include <wglext.h>

#include "handmade_renderer.h"
#include "win32_handmade_opengl.h"

#define GladLoadGLLoader gladLoadGLLoader
#define GladSetPostCallback glad_set_post_callback

void GladPostCallback(const char *Name, void *FuncPtr, i32 LenArgs, ...) {
	GLenum ErrorCode;
	ErrorCode = glad_glGetError();

	if (ErrorCode != GL_NO_ERROR)
	{
		char OpenGLError[256];
		FormatString(OpenGLError, ArrayCount(OpenGLError), "OpenGL Error: %d in %s\n", ErrorCode, Name);

		Win32OutputString(OpenGLError);
	}
}

inline void *
GetOpenGLFuncAddress(char *Name)
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
Win32OpenGLSetVSync(opengl_state *State, b32 VSync)
{
	State->wglSwapIntervalEXT(VSync);
}

void internal
Win32InitOpenGL(opengl_state *State, HINSTANCE hInstance, HWND WindowHandle)
{
    WNDCLASS FakeWindowClass = {};
    FakeWindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    FakeWindowClass.lpfnWndProc = DefWindowProc;
    FakeWindowClass.hInstance = hInstance;
    FakeWindowClass.lpszClassName = L"Fake OpenGL Window Class";

    RegisterClass(&FakeWindowClass);

    HWND FakeWindowHandle = CreateWindowEx(0, FakeWindowClass.lpszClassName, L"Fake OpenGL Window", 0,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0
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
        if (SetPixelFormat(FakeWindowDC, FakePixelFormatIndex, &FakePFD)) {
            HGLRC FakeRC = wglCreateContext(FakeWindowDC);

            if (wglMakeCurrent(FakeWindowDC, FakeRC))
            {
                // OpenGL 1.1 is initialized

                HDC WindowDC = GetDC(WindowHandle);

                PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
                PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
                State->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

                i32 PixelAttribs[] = {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_ALPHA_BITS_ARB, 8,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                    WGL_SAMPLES_ARB, 0,
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
                        GladLoadGLLoader((GLADloadproc)GetOpenGLFuncAddress);
						GladSetPostCallback(GladPostCallback);

                        State->Vendor = (char *)glGetString(GL_VENDOR);
                        State->Renderer = (char *)glGetString(GL_RENDERER);
                        State->Version = (char *)glGetString(GL_VERSION);
                        State->ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
                    }
                }
            }
        }
    }
}

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
		glGetProgramInfoLog(Program, LogLength, nullptr, ErrorLog);

		//PrintError("Error: ", "Shader program linkage failed", ErrorLog);
	}
	Assert(IsProgramLinked);

	return Program;
}

char *SimpleVertexShader = (char *)"#version 450\nlayout(location = 0) in vec2 in_Position;\nuniform mat4 u_Projection; uniform mat4 u_Model;\nvoid main() { gl_Position = vec4(in_Position, 1.f, 1.f) * u_Model * u_Projection; }";
char *SimpleFragmentShader = (char *)"#version 450\nout vec4 out_Color;\nuniform vec4 u_Color;\nvoid main() { out_Color = u_Color; }";

internal void
OpenGLProcessRenderCommands(opengl_state *State, render_commands *Commands)
{
    for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

        switch (Entry->Type)
        {
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

			// todo: use uniform buffer
			glUseProgram(State->SimpleShaderProgram);
			{
				mat4 Projection = Orthographic(Command->Left, Command->Right, Command->Bottom, Command->Top, Command->Near, Command->Far);

				i32 ProjectionUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Projection");
				glUniformMatrix4fv(ProjectionUniformLocation, 1, GL_FALSE, &Projection.Elements[0][0]);
			}
			glUseProgram(0);

			BaseAddress += sizeof(*Command);
			break;
		}
        case RenderCommand_Clear:
        {
            render_command_clear *Command = (render_command_clear *)Entry;

            glClearColor(Command->Color.x, Command->Color.y, Command->Color.z, Command->Color.w);
            glClear(GL_COLOR_BUFFER_BIT);

            BaseAddress += sizeof(*Command);
            break;
        }
		case RenderCommand_InitRectangle:
		{
			render_command_init_rectangle *Command = (render_command_init_rectangle *)Entry;

			vec2 RectangleVertices[] = {
				vec2(-1.f, -1.f),
				vec2(1.f, -1.f),
				vec2(-1.f, 1.f),
				vec2(1.f, 1.f)
			};

			glGenVertexArrays(1, &State->RectangleVAO);
			glBindVertexArray(State->RectangleVAO);

			GLuint RectangleVBO;
			glGenBuffers(1, &RectangleVBO);
			glBindBuffer(GL_ARRAY_BUFFER, RectangleVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(RectangleVertices), RectangleVertices, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

			glBindVertexArray(0);

			GLuint VertexShader = CreateShader(GL_VERTEX_SHADER, SimpleVertexShader);
			GLuint FragmentShader = CreateShader(GL_FRAGMENT_SHADER, SimpleFragmentShader);

			State->SimpleShaderProgram = CreateProgram(VertexShader, FragmentShader);
		}
        case RenderCommand_DrawRectangle:
        {
            render_command_draw_rectangle *Command = (render_command_draw_rectangle *)Entry;

			glBindVertexArray(State->RectangleVAO);

			glUseProgram(State->SimpleShaderProgram);
			{
				mat4 Model = mat4(1.f);

				Model = Translate(Model, vec3(Command->Position, 0.f));
				Model = Scale(Model, Command->Size.x);

				i32 ModelUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Model");
				glUniformMatrix4fv(ModelUniformLocation, 1, GL_FALSE, (f32 *)Model.Elements);

				i32 ColorUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Color");
				glUniform4f(ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
			}
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			
			glBindVertexArray(0);

            BaseAddress += sizeof(*Command);
            break;
        }
        default:
            Assert(!"Render command is not supported");
        }
    }
}
