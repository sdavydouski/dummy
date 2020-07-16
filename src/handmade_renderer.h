#pragma once

#include "handmade_defs.h"
#include "handmade_math.h"

#define MAX_NUMBER_OF_POINT_LIGHTS_PER_OBJECT 2

enum material_type
{
	MaterialType_Standard,
	MaterialType_Unlit
};

// todo: textures
struct material
{
	material_type Type;

	union
	{
		struct
		{
			vec3 DiffuseColor;
			f32 AmbientStrength;
			f32 SpecularStrength;
			f32 SpecularShininess;
		};

		struct
		{
			vec3 Color;
		};
	};
};

struct light_attenuation
{
	f32 Constant;
	f32 Linear;
	f32 Quadratic;
};

struct directional_light
{
	vec3 Direction;
	vec3 Color;
};

struct point_light
{
	vec3 Position;
	vec3 Color;

	light_attenuation Attenuation;
};

struct spot_light
{
	vec3 Position;
	vec3 Color;
	vec3 Direction;

	f32 InnerCutOffAngle;
	f32 OuterCutOffAngle;

	light_attenuation Attenuation;
};

struct skinned_vertex
{
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoords;
	i32 JointIndices[4];
	vec4 Weights;
};

enum primitive_type
{
	PrimitiveType_Line,
	PrimitiveType_Triangle
};

enum render_command_type
{
	RenderCommand_InitRenderer,
	RenderCommand_AddMesh,

	RenderCommand_SetViewport,
	RenderCommand_SetOrthographicProjection,
	RenderCommand_SetPerspectiveProjection,
	RenderCommand_SetCamera,
	RenderCommand_SetWireframe,

	RenderCommand_Clear,
	
	RenderCommand_DrawLine,
	RenderCommand_DrawRectangle,
	// todo: get rid of DrawGrid (he-he) and replace if with DrawMesh. (remove usage of a memory_arena inside the renderer)
	RenderCommand_DrawGrid,
	RenderCommand_DrawMesh,
	RenderCommand_DrawSkinnedMesh,

	RenderCommand_SetDirectionalLight
};

struct render_command_header
{
	render_command_type Type;
	u32 Size;
};

struct render_command_init_renderer
{
	render_command_header Header;
	u32 GridCount;
};

struct render_command_add_mesh
{
	render_command_header Header;

	u32 Id;
	primitive_type PrimitiveType;

	u32 VertexCount;
	skinned_vertex *Vertices;
	u32 IndexCount;
	u32 *Indices;
};

struct render_command_set_viewport
{
	render_command_header Header;
	u32 x;
	u32 y;
	u32 Width;
	u32 Height;
};

struct render_command_set_orthographic_projection
{
	render_command_header Header;
	f32 Left;
	f32 Right;
	f32 Bottom;
	f32 Top;
	f32 Near;
	f32 Far;
};

struct render_command_set_perspective_projection
{
	render_command_header Header;
	f32 FovY;
	f32 Aspect;
	f32 Near;
	f32 Far;
};

struct render_command_set_camera
{
	render_command_header Header;
	vec3 Eye;
	vec3 Target;
	vec3 Up;
	vec3 Position;
};

struct render_command_set_wireframe
{
	render_command_header Header;
	b32 IsWireframe;
};

struct render_command_clear
{
	render_command_header Header;
	vec4 Color;
};

struct render_command_draw_line
{
	render_command_header Header;
	vec3 Start;
	vec3 End;
	vec4 Color;
	f32 Thickness;
};

struct render_command_draw_rectangle
{
	render_command_header Header;
	transform Transform;
	vec4 Color;
};

struct render_command_draw_grid
{
	render_command_header Header;
	f32 Size;
	u32 Count;
	vec3 CameraPosition;
	vec3 Color;
};

struct render_command_draw_mesh
{
	render_command_header Header;

	u32 Id;

	transform Transform;

	material Material;

	point_light PointLight1;
	point_light PointLight2;
};

struct render_command_draw_skinned_mesh
{
	render_command_header Header;

	u32 Id;

	transform Transform;

	material Material;

	point_light PointLight1;
	point_light PointLight2;

	u32 SkinningMatrixCount;
	mat4 *SkinningMatrices;
};

struct render_command_set_directional_light
{
	render_command_header Header;
	directional_light Light;
};

struct render_commands
{
	u32 MaxRenderCommandsBufferSize;
	u32 RenderCommandsBufferSize;
	void *RenderCommandsBuffer;
};
