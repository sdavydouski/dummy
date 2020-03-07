#pragma once

struct opengl_state
{
	char *Vendor;
	char *Renderer;
	char *Version;
	char *ShadingLanguageVersion;

	GLuint RectangleVAO;
	GLuint SimpleShaderProgram;

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
};
