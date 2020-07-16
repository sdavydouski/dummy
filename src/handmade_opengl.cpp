#include <glad/glad.h>

#include "handmade_opengl.h"
#include "handmade_opengl_shaders.h"

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

		int t = 0;

		//PrintError("Error: ", "Shader program linkage failed", ErrorLog);
	}
	Assert(IsProgramLinked);

	return Program;
}

internal void
InitLine(opengl_state *State)
{
	vec3 LineVertices[] = {
		vec3(0.f, 0.f, 0.f),
		vec3(1.f, 1.f, 1.f),
	};

	glGenVertexArrays(1, &State->LineVAO);
	glBindVertexArray(State->LineVAO);

	GLuint LineVBO;
	glGenBuffers(1, &LineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, LineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertices), LineVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);

	glBindVertexArray(0);
}

internal void
InitRectangle(opengl_state *State)
{
	vec3 RectangleVertices[] = {
		vec3(-1.f, -1.f, 0.f),
		vec3(1.f, -1.f, 0.f),
		vec3(-1.f, 1.f, 0.f),
		vec3(1.f, 1.f, 0.f)
	};

	glGenVertexArrays(1, &State->RectangleVAO);
	glBindVertexArray(State->RectangleVAO);

	GLuint RectangleVBO;
	glGenBuffers(1, &RectangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, RectangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(RectangleVertices), RectangleVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);

	glBindVertexArray(0);
}

internal void
InitGrid(opengl_state *State, u32 GridCount)
{
	scoped_memory ScopedMemory(&State->Arena);

	vec3 *GridVertices = PushArray(ScopedMemory.Arena, GridCount * 8, vec3);

	for (u32 Index = 0; Index < GridCount; ++Index)
	{
		f32 Coord = (f32)(Index + 1) / (f32)GridCount;
		u32 GridVertexIndex = Index * 8;

		vec3 *GridVertex0 = GridVertices + GridVertexIndex + 0;
		*GridVertex0 = vec3(-Coord, 0.f, -1.f);

		vec3 *GridVertex1 = GridVertices + GridVertexIndex + 1;
		*GridVertex1 = vec3(-Coord, 0.f, 1.f);

		vec3 *GridVertex2 = GridVertices + GridVertexIndex + 2;
		*GridVertex2 = vec3(Coord, 0.f, -1.f);

		vec3 *GridVertex3 = GridVertices + GridVertexIndex + 3;
		*GridVertex3 = vec3(Coord, 0.f, 1.f);

		vec3 *GridVertex4 = GridVertices + GridVertexIndex + 4;
		*GridVertex4 = vec3(-1.f, 0.f, -Coord);

		vec3 *GridVertex5 = GridVertices + GridVertexIndex + 5;
		*GridVertex5 = vec3(1.f, 0.f, -Coord);

		vec3 *GridVertex6 = GridVertices + GridVertexIndex + 6;
		*GridVertex6 = vec3(-1.f, 0.f, Coord);

		vec3 *GridVertex7 = GridVertices + GridVertexIndex + 7;
		*GridVertex7 = vec3(1.f, 0.f, Coord);
	}

	glGenVertexArrays(1, &State->GridVAO);
	glBindVertexArray(State->GridVAO);

	GLuint GridVBO;
	glGenBuffers(1, &GridVBO);
	glBindBuffer(GL_ARRAY_BUFFER, GridVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * GridCount * 8, GridVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);

	glBindVertexArray(0);
}

// todo: use hashtable
inline opengl_mesh_buffer *
OpenGLGetMeshBuffer(opengl_state *State, u32 Id)
{
	opengl_mesh_buffer *Result = 0;

	for (u32 MeshBufferIndex = 0; MeshBufferIndex < State->CurrentMeshBufferCount; ++MeshBufferIndex)
	{
		opengl_mesh_buffer *MeshBuffer = State->MeshBuffers + MeshBufferIndex;

		if (MeshBuffer->Id == Id)
		{
			Result = MeshBuffer;
		}
	}

	return Result;
}

internal void
OpenGLAddMeshBuffer(opengl_state *State, u32 Id, primitive_type PrimitiveType, u32 VertexCount, skinned_vertex *Vertices, u32 IndexCount, u32 *Indices)
{
	Assert(State->CurrentMeshBufferCount < OPENGL_MAX_MESH_BUFFER_COUNT);

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, VertexCount * sizeof(skinned_vertex), Vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, TextureCoords));

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, JointIndices));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(skinned_vertex), (void *)StructOffset(skinned_vertex, Weights));

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexCount * sizeof(u32), Indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	opengl_mesh_buffer *MeshBuffer = State->MeshBuffers + State->CurrentMeshBufferCount++;
	MeshBuffer->Id = Id;
	MeshBuffer->PrimitiveType = PrimitiveType;
	MeshBuffer->IndexCount = IndexCount;
	MeshBuffer->VAO = VAO;
	MeshBuffer->VBO = VBO;
	MeshBuffer->EBO = EBO;
}

internal void
OpenGLProcessRenderCommands(opengl_state *State, render_commands *Commands)
{
	for (u32 BaseAddress = 0; BaseAddress < Commands->RenderCommandsBufferSize;)
	{
		render_command_header *Entry = (render_command_header *)((u8 *)Commands->RenderCommandsBuffer + BaseAddress);

		switch (Entry->Type)
		{
		case RenderCommand_InitRenderer:
		{
			render_command_init_renderer *Command = (render_command_init_renderer *)Entry;

			InitLine(State);
			InitRectangle(State);
			InitGrid(State, Command->GridCount);

			{
				GLuint VertexShader = CreateShader(GL_VERTEX_SHADER, SimpleVertexShader);
				GLuint FragmentShader = CreateShader(GL_FRAGMENT_SHADER, SimpleFragmentShader);
				State->SimpleShaderProgram = CreateProgram(VertexShader, FragmentShader);
			}

			{
				GLuint VertexShader = CreateShader(GL_VERTEX_SHADER, GridVertexShader);
				GLuint FragmentShader = CreateShader(GL_FRAGMENT_SHADER, GridFragmentShader);

				State->GridShaderProgram = CreateProgram(VertexShader, FragmentShader);
			}

			{
				GLuint VertexShader = CreateShader(GL_VERTEX_SHADER, ForwardShadingVertexShader);
				GLuint FragmentShader = CreateShader(GL_FRAGMENT_SHADER, ForwardShadingFragmentShader);
				State->ForwardShadingShaderProgram = CreateProgram(VertexShader, FragmentShader);
			}

			{
				GLuint VertexShader = CreateShader(GL_VERTEX_SHADER, SkinnedMeshVertexShader);
				GLuint FragmentShader = CreateShader(GL_FRAGMENT_SHADER, ForwardShadingFragmentShader);
				State->SkinnedMeshForwardShadingShaderProgram = CreateProgram(VertexShader, FragmentShader);
			}

			glGenBuffers(1, &State->SkinningTBO);
			glBindBuffer(GL_TEXTURE_BUFFER, State->SkinningTBO);
			// todo: size?
			glBufferData(GL_TEXTURE_BUFFER, Megabytes(4), 0, GL_STREAM_DRAW);

			glGenTextures(1, &State->SkinningTBOTexture);

			glBindBuffer(GL_TEXTURE_BUFFER, 0);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_MULTISAMPLE);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_AddMesh:
		{
			render_command_add_mesh *Command = (render_command_add_mesh *)Entry;

			OpenGLAddMeshBuffer(State, Command->Id, Command->PrimitiveType, Command->VertexCount, Command->Vertices, Command->IndexCount, Command->Indices);

			BaseAddress += sizeof(*Command);
			break;
		}
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
				glUniformMatrix4fv(ProjectionUniformLocation, 1, GL_TRUE, &Projection.Elements[0][0]);
			}
			glUseProgram(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_SetPerspectiveProjection:
		{
			render_command_set_perspective_projection *Command = (render_command_set_perspective_projection *)Entry;

			// todo: use uniform buffer
			glUseProgram(State->SimpleShaderProgram);
			{
				mat4 Projection = Perspective(Command->FovY, Command->Aspect, Command->Near, Command->Far);

				i32 ProjectionUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Projection");
				glUniformMatrix4fv(ProjectionUniformLocation, 1, GL_TRUE, &Projection.Elements[0][0]);
			}
			glUseProgram(0);

			glUseProgram(State->GridShaderProgram);
			{
				mat4 Projection = Perspective(Command->FovY, Command->Aspect, Command->Near, Command->Far);

				i32 ProjectionUniformLocation = glGetUniformLocation(State->GridShaderProgram, "u_Projection");
				glUniformMatrix4fv(ProjectionUniformLocation, 1, GL_TRUE, &Projection.Elements[0][0]);
			}
			glUseProgram(0);

			glUseProgram(State->ForwardShadingShaderProgram);
			{
				mat4 Projection = Perspective(Command->FovY, Command->Aspect, Command->Near, Command->Far);

				i32 ProjectionUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_Projection");
				glUniformMatrix4fv(ProjectionUniformLocation, 1, GL_TRUE, &Projection.Elements[0][0]);
			}
			glUseProgram(0);

			glUseProgram(State->SkinnedMeshForwardShadingShaderProgram);
			{
				mat4 Projection = Perspective(Command->FovY, Command->Aspect, Command->Near, Command->Far);

				i32 ProjectionUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_Projection");
				glUniformMatrix4fv(ProjectionUniformLocation, 1, GL_TRUE, &Projection.Elements[0][0]);
			}
			glUseProgram(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_SetCamera:
		{
			render_command_set_camera *Command = (render_command_set_camera *)Entry;

			// todo: use uniform buffer
			glUseProgram(State->SimpleShaderProgram);
			{
				mat4 View = LookAtLH(Command->Eye, Command->Target, Command->Up);

				i32 ViewUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_View");
				glUniformMatrix4fv(ViewUniformLocation, 1, GL_TRUE, &View.Elements[0][0]);
			}
			glUseProgram(0);

			glUseProgram(State->GridShaderProgram);
			{
				mat4 View = LookAtLH(Command->Eye, Command->Target, Command->Up);

				i32 ViewUniformLocation = glGetUniformLocation(State->GridShaderProgram, "u_View");
				glUniformMatrix4fv(ViewUniformLocation, 1, GL_TRUE, &View.Elements[0][0]);
			}
			glUseProgram(0);

			glUseProgram(State->ForwardShadingShaderProgram);
			{
				mat4 View = LookAtLH(Command->Eye, Command->Target, Command->Up);

				i32 ViewUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_View");
				glUniformMatrix4fv(ViewUniformLocation, 1, GL_TRUE, &View.Elements[0][0]);

				i32 CameraPositionUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_CameraPosition");
				glUniform3f(CameraPositionUniformLocation, Command->Position.x, Command->Position.y, Command->Position.z);
			}
			glUseProgram(0);

			glUseProgram(State->SkinnedMeshForwardShadingShaderProgram);
			{
				mat4 View = LookAtLH(Command->Eye, Command->Target, Command->Up);

				i32 ViewUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_View");
				glUniformMatrix4fv(ViewUniformLocation, 1, GL_TRUE, &View.Elements[0][0]);

				i32 CameraPositionUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_CameraPosition");
				glUniform3f(CameraPositionUniformLocation, Command->Position.x, Command->Position.y, Command->Position.z);
			}
			glUseProgram(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_SetWireframe:
		{
			render_command_set_wireframe *Command = (render_command_set_wireframe *)Entry;

			if (Command->IsWireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_Clear:
		{
			render_command_clear *Command = (render_command_clear *)Entry;

			glClearColor(Command->Color.x, Command->Color.y, Command->Color.z, Command->Color.w);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_DrawLine:
		{
			render_command_draw_line *Command = (render_command_draw_line *)Entry;

			// todo: restore line width
			glLineWidth(Command->Thickness);

			glBindVertexArray(State->LineVAO);
			glUseProgram(State->SimpleShaderProgram);
			{
				mat4 T = Translate(Command->Start);
				mat4 S = Scale(Command->End - Command->Start);
				mat4 Model = T * S;

				i32 ModelUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Model");
				glUniformMatrix4fv(ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

				i32 ColorUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Color");
				glUniform4f(ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
			}
			glDrawArrays(GL_LINES, 0, 2);

			glUseProgram(0);
			glBindVertexArray(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_DrawRectangle:
		{
			render_command_draw_rectangle *Command = (render_command_draw_rectangle *)Entry;

			glBindVertexArray(State->RectangleVAO);
			glUseProgram(State->SimpleShaderProgram);

			{
				mat4 Model = Transform(Command->Transform);

				i32 ModelUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Model");
				glUniformMatrix4fv(ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

				mat4 View = mat4(1.f);
				i32 ViewUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_View");
				glUniformMatrix4fv(ViewUniformLocation, 1, GL_TRUE, &View.Elements[0][0]);

				i32 ColorUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Color");
				glUniform4f(ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b, Command->Color.a);
			}
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glUseProgram(0);
			glBindVertexArray(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_DrawGrid:
		{
			render_command_draw_grid *Command = (render_command_draw_grid *)Entry;

			glBindVertexArray(State->GridVAO);
			glUseProgram(State->GridShaderProgram);
			{
				mat4 Model = Scale(Command->Size);

				i32 ModelUniformLocation = glGetUniformLocation(State->GridShaderProgram, "u_Model");
				glUniformMatrix4fv(ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

				i32 ColorUniformLocation = glGetUniformLocation(State->GridShaderProgram, "u_Color");
				glUniform3f(ColorUniformLocation, Command->Color.r, Command->Color.g, Command->Color.b);

				i32 CameraPositionUniformLocation = glGetUniformLocation(State->GridShaderProgram, "u_CameraPosition");
				glUniform3f(CameraPositionUniformLocation, Command->CameraPosition.x, Command->CameraPosition.y, Command->CameraPosition.z);
			}
			glDrawArrays(GL_LINES, 0, Command->Count * 8);

			glUseProgram(0);
			glBindVertexArray(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_DrawMesh:
		{
			render_command_draw_mesh *Command = (render_command_draw_mesh *)Entry;

			opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->Id);

			Assert(MeshBuffer);

			glBindVertexArray(MeshBuffer->VAO);

			switch (Command->Material.Type)
			{
				case MaterialType_Standard:
				{
					glUseProgram(State->ForwardShadingShaderProgram);

					mat4 Model = Transform(Command->Transform);

					i32 ModelUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_Model");
					glUniformMatrix4fv(ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

					// todo: store uniform locations
					i32 MaterialDiffuseColorUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_Material.DiffuseColor");
					i32 MaterialAmbientStrengthUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_Material.AmbientStrength");
					i32 MaterialSpecularStrengthUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_Material.SpecularStrength");
					i32 MaterialSpecularShininessUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_Material.SpecularShininess");
					glUniform3f(
						MaterialDiffuseColorUniformLocation,
						Command->Material.DiffuseColor.r,
						Command->Material.DiffuseColor.g,
						Command->Material.DiffuseColor.b
					);
					glUniform1f(MaterialAmbientStrengthUniformLocation, Command->Material.AmbientStrength);
					glUniform1f(MaterialSpecularStrengthUniformLocation, Command->Material.SpecularStrength);
					glUniform1f(MaterialSpecularShininessUniformLocation, Command->Material.SpecularShininess);

					// Point Lights
					{
						i32 PointLight1PositionUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[0].Position");
						i32 PointLight1ColorUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[0].Color");
						i32 PointLight1AttenuationConstantUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[0].Attenuation.Constant");
						i32 PointLight1AttenuationLinearUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[0].Attenuation.Linear");
						i32 PointLight1AttenuationQuadraticUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[0].Attenuation.Quadratic");

						glUniform3f(
							PointLight1PositionUniformLocation,
							Command->PointLight1.Position.x,
							Command->PointLight1.Position.y,
							Command->PointLight1.Position.z
						);
						glUniform3f(
							PointLight1ColorUniformLocation,
							Command->PointLight1.Color.r,
							Command->PointLight1.Color.g,
							Command->PointLight1.Color.b
						);
						glUniform1f(PointLight1AttenuationConstantUniformLocation, Command->PointLight1.Attenuation.Constant);
						glUniform1f(PointLight1AttenuationLinearUniformLocation, Command->PointLight1.Attenuation.Linear);
						glUniform1f(PointLight1AttenuationQuadraticUniformLocation, Command->PointLight1.Attenuation.Quadratic);
					}

					{
						i32 PointLight2PositionUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[1].Position");
						i32 PointLight2ColorUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[1].Color");
						i32 PointLight2AttenuationConstantUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[1].Attenuation.Constant");
						i32 PointLight2AttenuationLinearUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[1].Attenuation.Linear");
						i32 PointLight2AttenuationQuadraticUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_PointLights[1].Attenuation.Quadratic");

						glUniform3f(
							PointLight2PositionUniformLocation,
							Command->PointLight2.Position.x,
							Command->PointLight2.Position.y,
							Command->PointLight2.Position.z
						);
						glUniform3f(
							PointLight2ColorUniformLocation,
							Command->PointLight2.Color.r,
							Command->PointLight2.Color.g,
							Command->PointLight2.Color.b
						);
						glUniform1f(PointLight2AttenuationConstantUniformLocation, Command->PointLight2.Attenuation.Constant);
						glUniform1f(PointLight2AttenuationLinearUniformLocation, Command->PointLight2.Attenuation.Linear);
						glUniform1f(PointLight2AttenuationQuadraticUniformLocation, Command->PointLight2.Attenuation.Quadratic);
					}

					break;
				}
				case MaterialType_Unlit:
				{
					glUseProgram(State->SimpleShaderProgram);

					mat4 Model = Transform(Command->Transform);

					i32 ModelUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Model");
					glUniformMatrix4fv(ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

					i32 ColorUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Color");
					glUniform4f(ColorUniformLocation, Command->Material.Color.r, Command->Material.Color.g, Command->Material.Color.b, 1.f);

					break;
				}
				default:
				{
					Assert(!"Invalid material type");
					break;
				}
			}
			
			switch (MeshBuffer->PrimitiveType)
			{
				case PrimitiveType_Line:
				{
					glDrawElements(GL_LINES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
					break;
				}
				case PrimitiveType_Triangle:
				{
					glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
					break;
				}
				default:
				{
					Assert(!"Invalid primitive type");
				}
			}

			glUseProgram(0);
			glBindVertexArray(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_SetDirectionalLight:
		{
			render_command_set_directional_light *Command = (render_command_set_directional_light *)Entry;

			// todo: !!!
			glUseProgram(State->ForwardShadingShaderProgram);
			{
				i32 DirectionalLightDirectionUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_DirectionalLight.Direction");
				i32 DirectionalLightColorUniformLocation = glGetUniformLocation(State->ForwardShadingShaderProgram, "u_DirectionalLight.Color");
				glUniform3f(DirectionalLightDirectionUniformLocation, Command->Light.Direction.x, Command->Light.Direction.y, Command->Light.Direction.z);
				glUniform3f(DirectionalLightColorUniformLocation, Command->Light.Color.r, Command->Light.Color.g, Command->Light.Color.b);
			}
			glUseProgram(0);
			glUseProgram(State->SkinnedMeshForwardShadingShaderProgram);
			{
				i32 DirectionalLightDirectionUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_DirectionalLight.Direction");
				i32 DirectionalLightColorUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_DirectionalLight.Color");
				glUniform3f(DirectionalLightDirectionUniformLocation, Command->Light.Direction.x, Command->Light.Direction.y, Command->Light.Direction.z);
				glUniform3f(DirectionalLightColorUniformLocation, Command->Light.Color.r, Command->Light.Color.g, Command->Light.Color.b);
			}
			glUseProgram(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		case RenderCommand_DrawSkinnedMesh:
		{
			render_command_draw_skinned_mesh *Command = (render_command_draw_skinned_mesh *)Entry;

			opengl_mesh_buffer *MeshBuffer = OpenGLGetMeshBuffer(State, Command->Id);

			Assert(MeshBuffer);

			glBindVertexArray(MeshBuffer->VAO);

			switch (Command->Material.Type)
			{
				case MaterialType_Standard:
				{
					glUseProgram(State->SkinnedMeshForwardShadingShaderProgram);

					glBindBuffer(GL_TEXTURE_BUFFER, State->SkinningTBO);
					glBufferSubData(GL_TEXTURE_BUFFER, 0, Command->SkinningMatrixCount * sizeof(mat4), Command->SkinningMatrices);
					glBindBuffer(GL_TEXTURE_BUFFER, 0);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_BUFFER, State->SkinningTBOTexture);
					glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, State->SkinningTBO);

					i32 SkinningMatricesSamplerUniformLocation = 
						glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_SkinningMatricesSampler");
					glUniform1i(SkinningMatricesSamplerUniformLocation, 0);

					// todo: store uniform locations
					i32 MaterialDiffuseColorUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_Material.DiffuseColor");
					i32 MaterialAmbientStrengthUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_Material.AmbientStrength");
					i32 MaterialSpecularStrengthUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_Material.SpecularStrength");
					i32 MaterialSpecularShininessUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_Material.SpecularShininess");
					glUniform3f(
						MaterialDiffuseColorUniformLocation,
						Command->Material.DiffuseColor.r,
						Command->Material.DiffuseColor.g,
						Command->Material.DiffuseColor.b
					);
					glUniform1f(MaterialAmbientStrengthUniformLocation, Command->Material.AmbientStrength);
					glUniform1f(MaterialSpecularStrengthUniformLocation, Command->Material.SpecularStrength);
					glUniform1f(MaterialSpecularShininessUniformLocation, Command->Material.SpecularShininess);

					// Point Lights
					{
						i32 PointLight1PositionUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[0].Position");
						i32 PointLight1ColorUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[0].Color");
						i32 PointLight1AttenuationConstantUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[0].Attenuation.Constant");
						i32 PointLight1AttenuationLinearUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[0].Attenuation.Linear");
						i32 PointLight1AttenuationQuadraticUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[0].Attenuation.Quadratic");

						glUniform3f(
							PointLight1PositionUniformLocation,
							Command->PointLight1.Position.x,
							Command->PointLight1.Position.y,
							Command->PointLight1.Position.z
						);
						glUniform3f(
							PointLight1ColorUniformLocation,
							Command->PointLight1.Color.r,
							Command->PointLight1.Color.g,
							Command->PointLight1.Color.b
						);
						glUniform1f(PointLight1AttenuationConstantUniformLocation, Command->PointLight1.Attenuation.Constant);
						glUniform1f(PointLight1AttenuationLinearUniformLocation, Command->PointLight1.Attenuation.Linear);
						glUniform1f(PointLight1AttenuationQuadraticUniformLocation, Command->PointLight1.Attenuation.Quadratic);
					}

					{
						i32 PointLight2PositionUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[1].Position");
						i32 PointLight2ColorUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[1].Color");
						i32 PointLight2AttenuationConstantUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[1].Attenuation.Constant");
						i32 PointLight2AttenuationLinearUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[1].Attenuation.Linear");
						i32 PointLight2AttenuationQuadraticUniformLocation = glGetUniformLocation(State->SkinnedMeshForwardShadingShaderProgram, "u_PointLights[1].Attenuation.Quadratic");

						glUniform3f(
							PointLight2PositionUniformLocation,
							Command->PointLight2.Position.x,
							Command->PointLight2.Position.y,
							Command->PointLight2.Position.z
						);
						glUniform3f(
							PointLight2ColorUniformLocation,
							Command->PointLight2.Color.r,
							Command->PointLight2.Color.g,
							Command->PointLight2.Color.b
						);
						glUniform1f(PointLight2AttenuationConstantUniformLocation, Command->PointLight2.Attenuation.Constant);
						glUniform1f(PointLight2AttenuationLinearUniformLocation, Command->PointLight2.Attenuation.Linear);
						glUniform1f(PointLight2AttenuationQuadraticUniformLocation, Command->PointLight2.Attenuation.Quadratic);
					}

					break;
				}
				case MaterialType_Unlit:
				{
					Assert(!"Not Implemented");

	#if 0
					glUseProgram(State->SimpleShaderProgram);

					mat4 Model = Transform(Command->Position, Command->Scale, Command->Rotation);

					i32 ModelUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Model");
					glUniformMatrix4fv(ModelUniformLocation, 1, GL_TRUE, (f32 *)Model.Elements);

					i32 ColorUniformLocation = glGetUniformLocation(State->SimpleShaderProgram, "u_Color");
					glUniform4f(ColorUniformLocation, Command->Material.Color.r, Command->Material.Color.g, Command->Material.Color.b, 1.f);
	#endif

					break;
				}
				default:
				{
					Assert(!"Invalid material type");
					break;
				}
			}

			switch (MeshBuffer->PrimitiveType)
			{
				case PrimitiveType_Line:
				{
					glDrawElements(GL_LINES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
					break;
				}
				case PrimitiveType_Triangle:
				{
					glDrawElements(GL_TRIANGLES, MeshBuffer->IndexCount, GL_UNSIGNED_INT, 0);
					break;
				}
				default:
				{
					Assert(!"Invalid primitive type");
				}
			}

			glUseProgram(0);
			glBindVertexArray(0);

			BaseAddress += sizeof(*Command);
			break;
		}
		default:
			Assert(!"Render command is not supported");
		}
	}
}
