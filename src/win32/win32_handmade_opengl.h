#pragma once

struct opengl_state
{
	char *Vendor;
	char *Renderer;
	char *Version;
	char *ShadingLanguageVersion;

	GLuint RectangleVAO;
	GLuint BoxVAO;
	GLuint SimpleShaderProgram;

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
};
