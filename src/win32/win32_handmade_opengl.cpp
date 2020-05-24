#include <glad/glad.h>
#include <wglext.h>

#include "handmade_opengl.h"

#define GladLoadGLLoader gladLoadGLLoader
#define GladSetPostCallback glad_set_post_callback

void Win32GladPostCallback(const char *Name, void *FuncPtr, i32 LenArgs, ...) {
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
Win32GetOpenGLFuncAddress(char *Name)
{
	// https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions

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
				State->wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

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
                        GladLoadGLLoader((GLADloadproc)Win32GetOpenGLFuncAddress);
						GladSetPostCallback(Win32GladPostCallback);

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
