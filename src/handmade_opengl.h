#pragma once

#define OPENGL_MAX_MESH_BUFFER_COUNT 256

struct opengl_box_vertex
{
	vec3 Position;
	vec3 Normal;
};

struct opengl_mesh_buffer
{
	u32 Id;
	u32 IndexCount;
	primitive_type PrimitiveType;

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
	GLuint GridVAO;

	u32 CurrentMeshBufferCount;
	opengl_mesh_buffer MeshBuffers[OPENGL_MAX_MESH_BUFFER_COUNT];

	GLuint SimpleShaderProgram;
	GLuint GridShaderProgram;
	GLuint ForwardShadingShaderProgram;

	// Win32
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
};
