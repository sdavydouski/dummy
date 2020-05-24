#pragma once

struct box_vertex
{
	vec3 Position;
	vec3 Normal;
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
	GLuint BoxVAO;
	GLuint GridVAO;

	GLuint SimpleShaderProgram;
	GLuint GridShaderProgram;
	GLuint ForwardShadingShaderProgram;

	// Win32
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
};
