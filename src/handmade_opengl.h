#pragma once

#define OPENGL_MAX_MESH_BUFFER_COUNT 256

struct opengl_box_vertex
{
	vec3 Position;
	vec3 Normal;
};

struct opengl_mesh_buffer
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
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

	u32 MeshCount;
	opengl_mesh_buffer *MeshBuffers;

	// todo: remove this
	GLuint TempVAO;
	GLuint TempVBO;
	GLuint TempEBO;

	GLuint SimpleShaderProgram;
	GLuint GridShaderProgram;
	GLuint ForwardShadingShaderProgram;

	// Win32
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
};
