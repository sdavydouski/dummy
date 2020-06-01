#include "handmade_defs.h"
#include "handmade_math.h"
#include "handmade_memory.h"
#include "handmade_platform.h"
#include "handmade_physics.h"
#include "handmade.h"

#include "handmade_assets.cpp"
#include "handmade_renderer.cpp"

inline vec3
NormalizeRGB(vec3 RGB)
{
	vec3 Result = RGB / 255.f;
	return Result;
}

inline void
InitCamera(game_camera *Camera, f32 Pitch, f32 Yaw, f32 FovY, vec3 Position, vec3 Up = vec3(0.f, 1.f, 0.f))
{
	Camera->Pitch = Pitch;
	Camera->Yaw = Yaw;
	Camera->FovY = FovY;
	Camera->Position = Position;
	Camera->Up = Up;
	Camera->Direction = CalculateDirectionFromEulerAngles(Camera->Pitch, Camera->Yaw);
}

extern "C" DLLExport
GAME_INIT(GameInit)
{
	game_state *State = (game_state *)Memory->PermanentStorage;

	InitMemoryArena(
		&State->WorldArena,
		(u8 *)Memory->PermanentStorage + sizeof(game_state),
		Memory->PermanentStorageSize - sizeof(game_state)
	);

	State->Mode = GameMode_World;
	
	InitCamera(&State->DebugCamera, RADIANS(0.f), RADIANS(-90.f), RADIANS(45.f), vec3(0.f, 4.f, 16.f));

	State->GridCount = 100;

	State->RigidBodiesCount = 1;
	State->RigidBodies = PushArray(&State->WorldArena, State->RigidBodiesCount, rigid_body);

	{
		rigid_body *Body = State->RigidBodies + 0;
		*Body = {};
		Body->Position = vec3(0.f, 10.f, 0.f);
		Body->Damping = 0.5f;
		Body->HalfSize = vec3(1.f);
		SetRigidBodyMass(Body, 10.f);
	}

	State->Ground = ComputePlane(vec3(-1.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f), vec3(1.f, 0.f, 0.f));

	render_commands *RenderCommands = GetRenderCommandsFromMemory(Memory);
	InitRenderer(RenderCommands, State->GridCount);

	{
		model_asset ModelAsset = ReadModelAsset(Memory->Platform, (char *)"assets\\monkey.asset", &State->WorldArena);
		// todo: process all meshes in a model
		mesh *Mesh = ModelAsset.Meshes + 0;
		AddMesh(RenderCommands, 1, PrimitiveType_Triangle, Mesh->VertexCount, Mesh->Vertices, Mesh->IndexCount, Mesh->Indices);
	}

	{
		model_asset ModelAsset = ReadModelAsset(Memory->Platform, (char *)"assets\\light.asset", &State->WorldArena);
		// todo: process all meshes in a model
		mesh *Mesh = ModelAsset.Meshes + 0;
		AddMesh(RenderCommands, 2, PrimitiveType_Line, Mesh->VertexCount, Mesh->Vertices, Mesh->IndexCount, Mesh->Indices);
	}
}

extern "C" DLLExport
GAME_PROCESS_INPUT(GameProcessInput)
{
	game_state *State = (game_state *)Memory->PermanentStorage;
	platform_api *Platform = Memory->Platform;

	// todo?
	vec2 Move = Input->Move.Range;
	if ((Move.x == -1.f || Move.x == 1.f) && (Move.y == -1.f || Move.y == 1.f))
	{
		Move = Normalize(Move);
	}

	if (Input->Menu.IsActivated)
	{
		if (State->Mode == GameMode_Menu)
		{
			State->Mode = GameMode_World;
		}
		else if (State->Mode == GameMode_World)
		{
			State->Mode = GameMode_Menu;
		}
	}

	if (Input->Advance.IsActivated)
	{
		State->Advance = true;
	}

	if (Input->EnableFreeCameraMovement.IsActive)
	{
		Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);
	}
	else
	{
		Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Cursor);
	}

	State->IsBackgroundHighlighted = Input->HighlightBackground.IsActive;

	switch (State->Mode)
	{
		case GameMode_World:
		{
			f32 DebugCameraSpeed = 5.f;
			f32 DebugCameraSensitivity = 100.f;

			if (Input->EnableFreeCameraMovement.IsActive)
			{
				State->DebugCamera.Pitch += RADIANS(Input->Camera.Range.y) * DebugCameraSensitivity * Parameters->Delta;
				State->DebugCamera.Yaw += RADIANS(Input->Camera.Range.x) * DebugCameraSensitivity * Parameters->Delta;

				State->DebugCamera.Pitch = Clamp(State->DebugCamera.Pitch, RADIANS(-89.f), RADIANS(89.f));
				State->DebugCamera.Yaw = Mod(State->DebugCamera.Yaw, 2 * PI);

				State->DebugCamera.Direction = CalculateDirectionFromEulerAngles(State->DebugCamera.Pitch, State->DebugCamera.Yaw);
			}

			State->DebugCamera.Position += 
				(
					Move.x * (Normalize(Cross(State->DebugCamera.Direction, State->DebugCamera.Up))) + 
					Move.y * State->DebugCamera.Direction
				) * DebugCameraSpeed * Parameters->Delta;
		}
		break;
		case GameMode_Menu:
		{
			
		}
		break;
		default:
		{
			Assert(!"GameMode is not supported");
		}
	}
	
}

extern "C" DLLExport
GAME_UPDATE(GameUpdate)
{
	game_state *State = (game_state *)Memory->PermanentStorage;

	//if (State->Advance)
	{
		State->Advance = false;

		for (u32 RigidBodyIndex = 0; RigidBodyIndex < State->RigidBodiesCount; ++RigidBodyIndex)
		{
			rigid_body *Body = State->RigidBodies + RigidBodyIndex;

			AddGravityForce(Body, vec3(0.f, -1.f, 0.f));
			Integrate(Body, Parameters->UpdateRate);

			aabb BodyAABB = GetRigidBodyAABB(Body);

			// todo: dymamic intersection test
			if (TestAABBPlane(BodyAABB, State->Ground))
			{
				ResolveVelocity(Body, &State->Ground, Parameters->UpdateRate, 0.f);

				f32 Overlap = GetAABBPlaneMinDistance(BodyAABB, State->Ground);
				ResolveIntepenetration(Body, &State->Ground, Overlap);
				Body->Acceleration = vec3(0.f);
			}
		}
	}
}

extern "C" DLLExport 
GAME_RENDER(GameRender)
{
	game_state *State = (game_state *)Memory->PermanentStorage;
	render_commands *RenderCommands = GetRenderCommandsFromMemory(Memory);

	SetViewport(RenderCommands, 0, 0, Parameters->WindowWidth, Parameters->WindowHeight);
	
	f32 FrustrumWidthInUnits = 20.f;
	f32 PixelsPerUnit = (f32)Parameters->WindowWidth / FrustrumWidthInUnits;
	f32 FrustrumHeightInUnits = (f32)Parameters->WindowHeight / PixelsPerUnit;

	switch (State->Mode)
	{
		case GameMode_World:
		{
			vec3 xAxis = vec3(1.f, 0.f, 0.f);
			vec3 yAxis = vec3(0.f, 1.f, 0.f);
			vec3 zAxis = vec3(0.f, 0.f, 1.f);

			if (State->IsBackgroundHighlighted)
			{
				Clear(RenderCommands, vec4(0.f, 1.f, 1.f, 1.f));
			}
			else
			{
				Clear(RenderCommands, vec4(NormalizeRGB(vec3(0.f, 0.f, 0.f)), 1.f));
			}

			SetPerspectiveProjection(RenderCommands, State->DebugCamera.FovY, FrustrumWidthInUnits / FrustrumHeightInUnits, 0.1f, 1000.f);
			
			SetCamera(RenderCommands, State->DebugCamera.Position, State->DebugCamera.Position + State->DebugCamera.Direction, State->DebugCamera.Up, State->DebugCamera.Position);
			
			f32 Bounds = 100.f;

			DrawLine(RenderCommands, vec3(-Bounds, 0.f, 0.f), vec3(Bounds, 0.f, 0.f), vec4(NormalizeRGB(vec3(255, 51, 82)), 1.f), 1.f);
			DrawLine(RenderCommands, vec3(0.f, -Bounds, 0.f), vec3(0.f, Bounds, 0.f), vec4(NormalizeRGB(vec3(135, 213, 2)), 1.f), 1.f);
			DrawLine(RenderCommands, vec3(0.f, 0.f, -Bounds), vec3(0.f, 0.f, Bounds), vec4(NormalizeRGB(vec3(40, 144, 255)), 1.f), 1.f);

			DrawGrid(RenderCommands, Bounds, State->GridCount, State->DebugCamera.Position, vec3(0.f, 0.f, 1.f));

			directional_light DirectionalLight = {};
			DirectionalLight.Color = vec3(0.25f);
			DirectionalLight.Direction = vec3(0.f, -0.5f, -0.3f);

			SetDirectionalLight(RenderCommands, DirectionalLight);

			SetWireframe(RenderCommands, true);

			material BoxMaterial = {};
			BoxMaterial.DiffuseColor = vec3(1.f, 1.f, 1.f);
			BoxMaterial.AmbientStrength = 0.75f;
			BoxMaterial.SpecularStrength = 1.f;
			BoxMaterial.SpecularShininess = 32;

			for (u32 RigidBodyCount = 0; RigidBodyCount < State->RigidBodiesCount; ++RigidBodyCount)
			{
				rigid_body *Body = State->RigidBodies + RigidBodyCount;

				//DrawBox(RenderCommands, Body->Position, Body->HalfSize, BoxMaterial, vec4(Parameters->Time, vec3(0.f, 1.f, 0.f)));
			}

			DrawMesh(RenderCommands, 1, vec3(0.f, 2.f, 0.f), vec3(2.f));//, vec4(Parameters->Time, vec3(0.f, 1.f, 0.f)));
			DrawMesh(RenderCommands, 2, vec3(Cos(Parameters->Time) * 5.f, 2.f, Sin(Parameters->Time) * 5.f), vec3(0.5f));

			//DrawBox(RenderCommands, State->Player.Position, State->Player.HalfSize, vec4(0.f, 1.f, 1.f, 1.f));
#if 0
			DrawBox(RenderCommands, vec3(4.f, 2.f, -13.8f), vec3(0.2f, 2.f, 10.f), BoxMaterial);
			DrawBox(RenderCommands, vec3(-4.f, 2.f, -13.8f), vec3(0.2f, 2.f, 10.f), BoxMaterial);
			DrawBox(RenderCommands, vec3(0.f, 2.f, -24.f), vec3(4.2f, 2.f, 0.2f), BoxMaterial);

			DrawBox(RenderCommands, vec3(4.f, 2.f, 13.8f), vec3(0.2f, 2.f, 10.f), BoxMaterial);
			DrawBox(RenderCommands, vec3(-4.f, 2.f, 13.8f), vec3(0.2f, 2.f, 10.f), BoxMaterial);
			DrawBox(RenderCommands, vec3(0.f, 2.f, 24.f), vec3(4.2f, 2.f, 0.2f), BoxMaterial);

			DrawBox(RenderCommands, vec3(-14.2f, 2.f, 4.f), vec3(10.f, 2.f, 0.2f), BoxMaterial);
			DrawBox(RenderCommands, vec3(-14.2f, 2.f, -4.f), vec3(10.f, 2.f, 0.2f), BoxMaterial);
			DrawBox(RenderCommands, vec3(-24.f, 2.f, 0.f), vec3(0.2f, 2.f, 3.8f), BoxMaterial);

			DrawBox(RenderCommands, vec3(14.2f, 2.f, 4.f), vec3(10.f, 2.f, 0.2f), BoxMaterial);
			DrawBox(RenderCommands, vec3(14.2f, 2.f, -4.f), vec3(10.f, 2.f, 0.2f), BoxMaterial);
			DrawBox(RenderCommands, vec3(24.f, 2.f, 0.f), vec3(0.2f, 2.f, 3.8f), BoxMaterial);
#endif
		}
		break;
		case GameMode_Menu:
		{
			Clear(RenderCommands, vec4(1.f, 1.f, 1.f, 1.f));

			SetOrthographicProjection(RenderCommands,
				-FrustrumWidthInUnits / 2.f, FrustrumWidthInUnits / 2.f,
				-FrustrumHeightInUnits / 2.f, FrustrumHeightInUnits / 2.f,
				-10.f, 10.f
			);

			SetWireframe(RenderCommands, false);

			DrawRectangle(RenderCommands, vec2(-2.f, 2.f) * vec2(Cos(Parameters->Time), 1.f), vec2(0.5f), vec4(1.f, 0.f, 0.f, 1.f));
			DrawRectangle(RenderCommands, vec2(2.f, 2.f) * vec2(1.f, Cos(Parameters->Time)), vec2(0.5f), vec4(0.f, 1.f, 0.f, 1.f));
			DrawRectangle(RenderCommands, vec2(2.f, -2.f) * vec2(-Cos(Parameters->Time + PI), 1.f), vec2(0.5f), vec4(0.f, 0.f, 1.f, 1.f));
			DrawRectangle(RenderCommands, vec2(-2.f, -2.f) * vec2(1.f, Cos(Parameters->Time)), vec2(0.5f), vec4(1.f, 1.f, 0.f, 1.f));
		}
		break;
		default:
		{
			Assert(!"GameMode is not supported");
		}
	}
}
