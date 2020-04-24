#pragma once

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

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
};
