#include <glad/glad.h>
#include <wglext.h>

#include "sandbox_renderer.h"

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

void internal
Win32InitOpenGL(HINSTANCE hInstance, HWND WindowHandle, b32 VSync)
{
    WNDCLASS FakeWindowClass = {};
    FakeWindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    FakeWindowClass.lpfnWndProc = DefWindowProc;
    FakeWindowClass.hInstance = hInstance;
    FakeWindowClass.lpszClassName = L"Fake OpenGL Window Class";

    RegisterClass(&FakeWindowClass);

    HWND FakeWindowHandle = CreateWindowEx(
        0,                              // Optional window styles
        FakeWindowClass.lpszClassName,  // Window class
        L"Fake OpenGL Window",          // Window text
        0,                              // Window style
        CW_USEDEFAULT,                  // Window position x 
        CW_USEDEFAULT,                  // Window position y
        CW_USEDEFAULT,                  // Window width
        CW_USEDEFAULT,                  // Window height
        0,                              // Parent window    
        0,                              // Menu
        hInstance,                      // Instance handle
        0                               // Additional application data
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
                PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

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
                        gladLoadGLLoader((GLADloadproc)GetOpenGLFuncAddress);

                        char *OpenGLVendor = (char *)glGetString(GL_VENDOR);
                        char *OpenGLRenderer = (char *)glGetString(GL_RENDERER);
                        char *OpenGLVersion = (char *)glGetString(GL_VERSION);
                        char *OpenGLShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

                        Win32OutputString(OpenGLVendor);
                        Win32OutputString(OpenGLRenderer);
                        Win32OutputString(OpenGLVersion);
                        Win32OutputString(OpenGLShadingLanguageVersion);

                        wglSwapIntervalEXT(VSync ? 1 : 0);
                    }
                }
            }
        }
    }
}

internal void
OpenGLRender(render_buffer *Buffer)
{
    for (u32 BaseAddress = 0; BaseAddress < Buffer->Used;)
    {
        render_command_header *Entry = (render_command_header *)((u8 *)Buffer->Base + BaseAddress);

        switch (Entry->Type)
        {
        case RenderCommand_SetViewport:
        {
            render_command_set_viewport *Command = (render_command_set_viewport *)Entry;

            glViewport(Command->x, Command->y, Command->Width, Command->Height);

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
        case RenderCommand_DrawRectangle:
        {
            render_command_draw_rectangle *Command = (render_command_draw_rectangle *)Entry;

            glBegin(GL_TRIANGLES);

            glColor4f(Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);

            glVertex2f(Command->Min.x, Command->Min.y);
            glVertex2f(-Command->Min.x, Command->Min.y);
            glVertex2f(Command->Min.x, -Command->Min.y);

            glVertex2f(-Command->Max.x, Command->Max.y);
            glVertex2f(Command->Max.x, Command->Max.y);
            glVertex2f(Command->Max.x, -Command->Max.y);

            glEnd();

            BaseAddress += sizeof(*Command);
            break;
        }
        default:
            Assert(!"Render command is not supported");
        }
    }
}
