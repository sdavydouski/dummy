#include "dummy_defs.h"
#include "dummy_math.h"
#include "dummy_random.h"
#include "dummy_memory.h"
#include "dummy_string.h"
#include "dummy_platform.h"
#include "dummy_collision.h"
#include "dummy_physics.h"
#include "dummy_renderer.h"
#include "dummy_animation.h"
#include "dummy_assets.h"
#include "dummy.h"

#include "dummy_assets.cpp"
#include "dummy_renderer.cpp"
#include "dummy_animation.cpp"

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

inline material
CreateMaterial(material_type Type, mesh_material *MeshMaterial, b32 IsWireframe)
{
    material Result = {};

    Result.Type = Type;
    Result.MeshMaterial = MeshMaterial;
    Result.IsWireframe = IsWireframe;

    return Result;
}

inline material
CreateMaterial(material_type Type, vec3 Color, b32 IsWireframe)
{
    material Result = {};

    Result.Type = Type;
    Result.Color = Color;
    Result.IsWireframe = IsWireframe;

    return Result;
}

inline void
DrawSkinnedModel(render_commands *RenderCommands, model *Model, skeleton_pose *Pose, transform Transform, point_light PointLight1, point_light PointLight2)
{
    Assert(Model->Skeleton);
    
    for (u32 JointIndex = 0; JointIndex < Model->Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Model->Skeleton->Joints + JointIndex;
        mat4 *GlobalJointPose = Pose->GlobalJointPoses + JointIndex;
        mat4 *SkinningMatrix = Model->SkinningMatrices + JointIndex;

        *SkinningMatrix = *GlobalJointPose * Joint->InvBindTranform;
    }

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
        material Material = CreateMaterial(MaterialType_BlinnPhong, MeshMaterial, false);

        DrawSkinnedMesh(
            RenderCommands, Mesh->Id, {}, Material,
            PointLight1, PointLight2, Model->SkinningMatrixCount, Model->SkinningMatrices
        );
    }
}

inline void
DrawModel(render_commands *RenderCommands, model *Model, transform Transform, point_light PointLight1, point_light PointLight2)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
        material Material = CreateMaterial(MaterialType_BlinnPhong, MeshMaterial, false);

        DrawMesh(RenderCommands, Mesh->Id, Transform, Material, PointLight1, PointLight2);
    }
}

inline void
DrawModel(render_commands *RenderCommands, model *Model, transform Transform, material Material, point_light PointLight1, point_light PointLight2)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        DrawMesh(RenderCommands, Mesh->Id, Transform, Material, PointLight1, PointLight2);
    }
}

inline void
DrawModelInstanced(render_commands *RenderCommands, model *Model, u32 InstanceCount, render_instance *Instances, point_light PointLight1, point_light PointLight2)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
        material Material = CreateMaterial(MaterialType_BlinnPhong, MeshMaterial, false);

        DrawMeshInstanced(RenderCommands, Mesh->Id, InstanceCount, Instances, Material, PointLight1, PointLight2);
    }
}

// todo:
inline u32
GenerateMeshId()
{
    persist u32 MeshId = 0;

    return MeshId++;
}

// todo:
inline u32
GenerateTextureId()
{
    persist u32 TextureId = 10;

    return TextureId++;
}

inline void
InitModel(model_asset *Asset, model *Model, memory_arena *Arena, render_commands *RenderCommands, u32 MaxInstanceCount = 0)
{
    *Model = {};

    Model->Skeleton = &Asset->Skeleton;
    Model->BindPose = &Asset->BindPose;
    
    Model->Pose = PushType(Arena, skeleton_pose);
    Model->Pose->Skeleton = Model->Skeleton;
    Model->Pose->LocalJointPoses = PushArray(Arena, Model->Skeleton->JointCount, joint_pose);
    Model->Pose->GlobalJointPoses = PushArray(Arena, Model->Skeleton->JointCount, mat4);

    for (u32 JointIndex = 0; JointIndex < Model->BindPose->Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *SourceLocalJointPose = Model->BindPose->LocalJointPoses + JointIndex;
        joint_pose *DestLocalJointPose = Model->Pose->LocalJointPoses + JointIndex;

        *DestLocalJointPose = *SourceLocalJointPose;
    }

    Model->MeshCount = Asset->MeshCount;
    Model->Meshes = Asset->Meshes;
    Model->MaterialCount = Asset->MaterialCount;
    Model->Materials = Asset->Materials;
    Model->AnimationCount = Asset->AnimationCount;
    Model->Animations = Asset->Animations;

    if (Model->Skeleton)
    {
        Model->SkinningMatrixCount = Model->Skeleton->JointCount;
        Model->SkinningMatrices = PushArray(Arena, Model->SkinningMatrixCount, mat4);
    }

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        Mesh->Id = GenerateMeshId();
        AddMesh(RenderCommands, Mesh->Id, Mesh->VertexCount, Mesh->Vertices, Mesh->IndexCount, Mesh->Indices, MaxInstanceCount);

        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;

        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MeshMaterial->PropertyCount; ++MaterialPropertyIndex)
        {
            material_property *MaterialProperty = MeshMaterial->Properties + MaterialPropertyIndex;
            MaterialProperty->Id = -1;

            if (
                MaterialProperty->Type == MaterialProperty_Texture_Diffuse ||
                MaterialProperty->Type == MaterialProperty_Texture_Specular ||
                MaterialProperty->Type == MaterialProperty_Texture_Shininess ||
                MaterialProperty->Type == MaterialProperty_Texture_Normal
            )
            {
                MaterialProperty->Id = GenerateTextureId();
                AddTexture(RenderCommands, MaterialProperty->Id, &MaterialProperty->Bitmap);
            }
        }
    }
}

internal void
DrawSkeleton(render_commands *RenderCommands, skeleton_pose *Pose, model *JointModel)
{
    for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Pose->Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Pose->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Pose->GlobalJointPoses + JointIndex;

        transform Transform = CreateTransform(GetTranslation(*GlobalJointPose), vec3(0.05f), quat(0.f));
        material Material = CreateMaterial(MaterialType_Unlit, vec3(1.f, 1.f, 0.f), false);

        DrawModel(RenderCommands, JointModel, Transform, Material, {}, {});

        if (Joint->ParentIndex > -1)
        {
            mat4 *ParentGlobalJointPose = Pose->GlobalJointPoses + Joint->ParentIndex;

            vec3 LineStart = GetTranslation(*ParentGlobalJointPose);
            vec3 LineEnd = GetTranslation(*GlobalJointPose);

            DrawLine(RenderCommands, LineStart, LineEnd, vec4(1.f, 0.f, 1.f, 1.f), 1.f);
        }
    }
}

extern "C" DLLExport
GAME_INIT(GameInit)
{
    game_state *State = (game_state *)Memory->PermanentStorage;
    platform_api *Platform = Memory->Platform;

    InitMemoryArena(
        &State->WorldArena,
        (u8 *)Memory->PermanentStorage + sizeof(game_state),
        Memory->PermanentStorageSize - sizeof(game_state)
    );

    State->Mode = GameMode_World;

    InitCamera(&State->FreeCamera, RADIANS(0.f), RADIANS(-90.f), RADIANS(45.f), vec3(0.f, 4.f, 16.f));
    InitCamera(&State->PlayerCamera, RADIANS(20.f), RADIANS(0.f), RADIANS(45.f), vec3(0.f, 0.f, 0.f));
    State->PlayerCamera.Radius = 16.f;

    State->RigidBodiesCount = 1;
    State->RigidBodies = PushArray(&State->WorldArena, State->RigidBodiesCount, rigid_body);

    {
        rigid_body *Body = State->RigidBodies + 0;
        *Body = {};
        Body->PrevPosition = vec3(0.f, 3.f, 0.f);
        Body->Position = vec3(0.f, 3.f, 0.f);
        Body->Orientation = quat(0.f, 0.f, 0.f, 1.f);
        Body->Damping = 0.0001f;
        Body->HalfSize = vec3(1.f, 3.f, 1.f);
        SetRigidBodyMass(Body, 75.f);
    }

    {
        rigid_body *Body = State->RigidBodies + 1;
        *Body = {};
        Body->PrevPosition = vec3(5.f, 10.f, 0.f);
        Body->Position = vec3(5.f, 10.f, 0.f);
        Body->Orientation = quat(0.f, 0.f, 0.f, 1.f);
        Body->Damping = 0.0001f;
        Body->HalfSize = vec3(1.f);
        SetRigidBodyMass(Body, 10.f);
    }

    State->Ground = ComputePlane(vec3(-1.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f), vec3(1.f, 0.f, 0.f));
    State->BackgroundColor = vec3(0.f, 0.f, 0.f);
    State->DirectionalColor = vec3(1.f);

    State->RNG = RandomSequence(42);

    State->ShowModel = true;
    State->ShowSkeleton = false;

    ClearRenderCommands(Memory);
    render_commands *RenderCommands = GetRenderCommands(Memory);
    InitRenderer(RenderCommands);

    model_asset *YBotModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\pelegrini.asset", &State->WorldArena);
    InitModel(YBotModelAsset, &State->YBotModel, &State->WorldArena, RenderCommands);

    model_asset *SkullModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\skull.asset", &State->WorldArena);
    InitModel(SkullModelAsset, &State->SkullModel, &State->WorldArena, RenderCommands);

    model_asset *SphereModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\sphere.asset", &State->WorldArena);
    InitModel(SphereModelAsset, &State->SphereModel, &State->WorldArena, RenderCommands);

    model_asset *CubeModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\cube.asset", &State->WorldArena);
    InitModel(CubeModelAsset, &State->CubeModel, &State->WorldArena, RenderCommands);

    model_asset *WallModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\wall.asset", &State->WorldArena);
    InitModel(WallModelAsset, &State->WallModel, &State->WorldArena, RenderCommands);

    State->FloorDim = vec2(10.f, 10.f);
    State->InstanceCount = (u32)(State->FloorDim.x * State->FloorDim.y);
    State->Instances = PushArray(&State->WorldArena, State->InstanceCount, render_instance);

    model_asset *FloorModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\floor.asset", &State->WorldArena);
    InitModel(FloorModelAsset, &State->FloorModel, &State->WorldArena, RenderCommands, State->InstanceCount);

    State->LerperQuat = {};
    State->CurrentMove = vec2(0.f);
    State->TargetMove = vec2(0.f);

    State->Player = {};
    State->Player.Model = &State->YBotModel;
    State->Player.RigidBody = State->RigidBodies + 0;
    State->Player.Offset = vec3(0.f, -3.f, 0.f);
    State->Player.State = EntityState_Idle;
    // 

    Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);

    BuildAnimationGraph(&State->Player.AnimationGraph, State->Player.Model, &State->WorldArena, &State->RNG);
    ActivateAnimationNode(&State->Player.AnimationGraph, "Idle_Node");
}

extern "C" DLLExport
GAME_PROCESS_INPUT(GameProcessInput)
{
    game_state *State = (game_state *)Memory->PermanentStorage;
    platform_api *Platform = Memory->Platform;

    vec3 xAxis = vec3(1.f, 0.f, 0.f);
    vec3 yAxis = vec3(0.f, 1.f, 0.f);
    vec3 zAxis = vec3(0.f, 0.f, 1.f);

    f32 Lag = Parameters->UpdateLag / Parameters->UpdateRate;
    Assert(0.f <= Lag && Lag <= 1.f);

    vec3 PlayerPosition = Lerp(State->Player.RigidBody->PrevPosition, Lag, State->Player.RigidBody->Position);

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

    if (Input->EditMode.IsActivated)
    {
        if (State->Mode == GameMode_World || State->Mode == GameMode_Menu)
        {
            State->Mode = GameMode_Edit;
            Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Cursor);
        }
        else
        {
            State->Mode = GameMode_World;
            Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);
        }
    }

    State->IsBackgroundHighlighted = Input->HighlightBackground.IsActive;

    switch (State->Mode)
    {
        case GameMode_World:
        {
            f32 PlayerCameraSensitivity = 2.f;

            State->PlayerCamera.Pitch -= Input->Camera.Range.y * PlayerCameraSensitivity * Parameters->Delta;
            State->PlayerCamera.Pitch = Clamp(State->PlayerCamera.Pitch, RADIANS(0.f), RADIANS(89.f));

            State->PlayerCamera.Yaw += Input->Camera.Range.x * PlayerCameraSensitivity * Parameters->Delta;
            State->PlayerCamera.Yaw = Mod(State->PlayerCamera.Yaw, 2 * PI);

            State->PlayerCamera.Radius -= Input->ZoomDelta * 250.0f * Parameters->Delta;
            State->PlayerCamera.Radius = Clamp(State->PlayerCamera.Radius, 10.f, 70.f);

            f32 CameraHeight = State->PlayerCamera.Radius * Sin(State->PlayerCamera.Pitch);

            State->PlayerCamera.Position.x = PlayerPosition.x +
                Sqrt(Square(State->PlayerCamera.Radius) - Square(CameraHeight)) * Sin(State->PlayerCamera.Yaw);
            State->PlayerCamera.Position.y = PlayerPosition.y + CameraHeight;
            State->PlayerCamera.Position.z = PlayerPosition.z -
                Sqrt(Square(State->PlayerCamera.Radius) - Square(CameraHeight)) * Cos(State->PlayerCamera.Yaw);

            vec3 CameraLookAtPoint = PlayerPosition; //+ vec3(0.f, 4.f, 0.f);

            State->PlayerCamera.Direction = Normalize(CameraLookAtPoint - State->PlayerCamera.Position);

            vec3 yMoveAxis = Normalize(Projection(State->PlayerCamera.Direction, State->Ground));
            vec3 xMoveAxis = Normalize(Orthogonal(yMoveAxis, State->Ground));

            f32 xMoveY = Dot(yMoveAxis, xAxis) * Move.y;
            f32 zMoveY = Dot(yMoveAxis, zAxis) * Move.y;

            f32 xMoveX = Dot(xMoveAxis, xAxis) * Move.x;
            f32 zMoveX = Dot(xMoveAxis, zAxis) * Move.x;

            f32 MoveMaginute = Clamp(Magnitude(Move), 0.f, 1.f);
            
            f32 Clock = 1.f;

            /*
                Rotate the characters towards their desired lookat vector, and then take the speed of the character + 
                their relative angle between their velocity and their current forward vector to feed into a 360 motion locomotion blendspace.
            */

            vec3 PlayerDirection = vec3(Dot(vec3(Move.x, 0.f, Move.y), xMoveAxis), 0.f, Dot(vec3(Move.x, 0.f, Move.y), yMoveAxis));

            State->TargetMove = Move;

            State->Player.RigidBody->Acceleration.x = (xMoveX + xMoveY) * 110.f;
            State->Player.RigidBody->Acceleration.z = (zMoveX + zMoveY) * 110.f;

            f32 CrossFadeDuration = 0.2f;

            quat PlayerOrientation = AxisAngle2Quat(vec4(yAxis, Atan2(PlayerDirection.x, PlayerDirection.z)));

            if (MoveMaginute > 0.f)
            {
                State->LerperQuat.IsEnabled = true;
                State->LerperQuat.Time = 0.f;
                State->LerperQuat.Duration = 0.2f;
                State->LerperQuat.From = State->Player.RigidBody->Orientation;
                State->LerperQuat.To = PlayerOrientation;
                State->LerperQuat.Result = &State->Player.RigidBody->Orientation;
            }

#if 1
            switch (State->Player.State)
            {
                case EntityState_Idle:
                {
                    if (Input->Crouch.IsActivated)
                    {
                        State->Player.State = EntityState_Dance;
                        TransitionToNode(&State->Player.AnimationGraph, "Dance_Node");
                    }
                    
                    if (MoveMaginute > 0.f)
                    {
                        State->Player.State = EntityState_Moving;
                        TransitionToNode(&State->Player.AnimationGraph, "Move_Node");
                    }

                    break;
                }
                case EntityState_Moving:
                {
                    if (MoveMaginute == 0.f)
                    {
                        State->Player.State = EntityState_Idle;
                        TransitionToNode(&State->Player.AnimationGraph, "Idle_Node");
                    }

                    break;
                }
                case EntityState_Dance:
                {
                    if (Input->Crouch.IsActivated)
                    {
                        State->Player.State = EntityState_Idle;
                        TransitionToNode(&State->Player.AnimationGraph, "Idle_Node");
                    }

                    break;
                }
                default:
                {
                    Assert(!"Invalid state");
                }
            }
#endif

            break;
        }
        case GameMode_Edit:
        {
            f32 FreeCameraSpeed = 10.f;
            f32 FreeCameraSensitivity = 1.f;

            if (Input->EnableFreeCameraMovement.IsActive)
            {
                Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);

                State->FreeCamera.Pitch += Input->Camera.Range.y * FreeCameraSensitivity * Parameters->Delta;
                State->FreeCamera.Pitch = Clamp(State->FreeCamera.Pitch, RADIANS(-89.f), RADIANS(89.f));

                State->FreeCamera.Yaw += Input->Camera.Range.x * FreeCameraSensitivity * Parameters->Delta;
                State->FreeCamera.Yaw = Mod(State->FreeCamera.Yaw, 2 * PI);

                State->FreeCamera.Direction = CalculateDirectionFromEulerAngles(State->FreeCamera.Pitch, State->FreeCamera.Yaw);
            }
            else
            {
                Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Cursor);
            }

            State->FreeCamera.Position += (
                    Move.x * (Normalize(Cross(State->FreeCamera.Direction, State->FreeCamera.Up))) +
                    Move.y * State->FreeCamera.Direction
                ) * FreeCameraSpeed * Parameters->Delta;

            break;
        }
        case GameMode_Menu:
        {
            break;
        }
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
        State->BackgroundColor = vec3(0.f);

        for (u32 RigidBodyIndex = 0; RigidBodyIndex < State->RigidBodiesCount; ++RigidBodyIndex)
        {
            rigid_body *Body = State->RigidBodies + RigidBodyIndex;

            AddGravityForce(Body, vec3(0.f, -10.f, 0.f));
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

            for (u32 OtherRigidBodyIndex = RigidBodyIndex + 1; OtherRigidBodyIndex < State->RigidBodiesCount; ++OtherRigidBodyIndex)
            {
                rigid_body *OtherBody = State->RigidBodies + OtherRigidBodyIndex;

                aabb OtherBodyAABB = GetRigidBodyAABB(OtherBody);

                if (TestAABBAABB(BodyAABB, OtherBodyAABB))
                {
                    State->BackgroundColor = vec3(1.f, 0.f, 0.f);
                }
            }
        }
    }
}

extern "C" DLLExport 
GAME_RENDER(GameRender)
{
    game_state *State = (game_state *)Memory->PermanentStorage;
    render_commands *RenderCommands = GetRenderCommands(Memory);

    RenderCommands->WindowWidth = Parameters->WindowWidth;
    RenderCommands->WindowHeight = Parameters->WindowHeight;
    RenderCommands->Time = Parameters->Time * 10.f;

    f32 Lag = Parameters->UpdateLag / Parameters->UpdateRate;

    vec3 PlayerPosition = Lerp(State->Player.RigidBody->PrevPosition, Lag, State->Player.RigidBody->Position);

    SetViewport(RenderCommands, 0, 0, Parameters->WindowWidth, Parameters->WindowHeight);
    
    f32 FrustrumWidthInUnits = 20.f;
    f32 PixelsPerUnit = (f32)Parameters->WindowWidth / FrustrumWidthInUnits;
    f32 FrustrumHeightInUnits = (f32)Parameters->WindowHeight / PixelsPerUnit;

    switch (State->Mode)
    {
        case GameMode_World:
        case GameMode_Edit:
        {
            vec3 xAxis = vec3(1.f, 0.f, 0.f);
            vec3 yAxis = vec3(0.f, 1.f, 0.f);
            vec3 zAxis = vec3(0.f, 0.f, 1.f);

            game_camera *Camera = State->Mode == GameMode_World 
                ? &State->PlayerCamera
                : &State->FreeCamera;

            if (State->IsBackgroundHighlighted)
            {
                Clear(RenderCommands, vec4(1.f, 1.f, 1., 1.f));
            }
            else
            {
                Clear(RenderCommands, vec4(State->BackgroundColor, 1.f));
            }

            SetPerspectiveProjection(RenderCommands, Camera->FovY, FrustrumWidthInUnits / FrustrumHeightInUnits, 0.1f, 1000.f);
            
            SetCamera(RenderCommands, Camera->Position, Camera->Position + Camera->Direction, Camera->Up, Camera->Position);
            
            // todo: SetTransform render command?
            f32 Bounds = 500.f;

            // Axis
            //DrawLine(RenderCommands, vec3(-Bounds, 0.f, 0.f), vec3(Bounds, 0.f, 0.f), vec4(NormalizeRGB(vec3(255, 51, 82)), 1.f), 4.f);
            //DrawLine(RenderCommands, vec3(0.f, -Bounds, 0.f), vec3(0.f, Bounds, 0.f), vec4(NormalizeRGB(vec3(135, 213, 2)), 1.f), 4.f);
            //DrawLine(RenderCommands, vec3(0.f, 0.f, -Bounds), vec3(0.f, 0.f, Bounds), vec4(NormalizeRGB(vec3(40, 144, 255)), 1.f), 4.f);
            
            // todo: pretty expensive
            //DrawGround(RenderCommands, Camera->Position);

            // Skydome rendering
            //transform SkydomeTransform = CreateTransform(Camera->Position, vec3(100.f), quat(0.f));
            //material SkydomeMaterial = CreateMaterial(MaterialType_Unlit, vec3(1.f, 0.f, 1.f), true);
            //DrawModel(RenderCommands, &State->SphereModel, SkydomeTransform, SkydomeMaterial, {}, {});

            if (State->Player.State == EntityState_Dance)
            {
                f32 Time = Parameters->Time * 11.f;
                State->DirectionalColor = vec3(Sin(Time), Cos(Time), Sin(Time)) * 0.8f;
            }
            else
            {
                //State->DirectionalColor = vec3(0.f);
            }

            directional_light DirectionalLight = {};
            DirectionalLight.Color = State->DirectionalColor;
            DirectionalLight.Direction = Normalize(vec3(0.4f, -0.8f, -0.4f));

            SetDirectionalLight(RenderCommands, DirectionalLight);

            f32 PointLightRadius = 4.f;
            vec3 PointLight1Position = PlayerPosition + vec3(Cos(Parameters->Time * 2.f) * PointLightRadius, 1.f, Sin(Parameters->Time * 2.f) * PointLightRadius);
            vec3 PointLight1Color = vec3(1.f, 1.f, 0.f);

            vec3 PointLight2Position = PlayerPosition + vec3(Cos(Parameters->Time * 2.f - PI) * PointLightRadius, -1.f, Sin(Parameters->Time * 2.f - PI) * PointLightRadius);
            vec3 PointLight2Color = vec3(1.f, 0.f, 1.f);

            point_light DummyPointLight = {};
            DummyPointLight.Attenuation.Constant = 1.f;
            DummyPointLight.Attenuation.Linear = 1.f;
            DummyPointLight.Attenuation.Quadratic = 1.f;

            point_light PointLight1 = {};
            PointLight1.Position = PointLight1Position;
            PointLight1.Color = PointLight1Color;
            PointLight1.Attenuation.Constant = 1.f;
            PointLight1.Attenuation.Linear = 0.09f;
            PointLight1.Attenuation.Quadratic = 0.032f;

            point_light PointLight2 = {};
            PointLight2.Position = PointLight2Position;
            PointLight2.Color = PointLight2Color;
            PointLight2.Attenuation.Constant = 1.f;
            PointLight2.Attenuation.Linear = 0.09f;
            PointLight2.Attenuation.Quadratic = 0.032f;

            // todo: extract
            if (State->LerperQuat.IsEnabled)
            {
                State->LerperQuat.Time += Parameters->Delta;

                f32 t = State->LerperQuat.Time / State->LerperQuat.Duration;

                if (t <= 1.f)
                {
                    *State->LerperQuat.Result = Slerp(State->LerperQuat.From, t, State->LerperQuat.To);
                }
                else
                {
                    State->LerperQuat.IsEnabled = false;
                    State->LerperQuat.Time = 0.f;
                    State->LerperQuat.Duration = 0.f;
                    State->LerperQuat.From = quat(0.f);
                    State->LerperQuat.To = quat(0.f);
                    State->LerperQuat.Result = 0;
                }
            }

            /*
                Pivotal Movement
                To implement pivotal movement, we can simply play the forward locomotion
                loop while rotating the entire character about its vertical axis to make it turn.
                Pivotal movement looks more natural if the character’s body doesn’t remain
                bolt upright when it is turning—real humans tend to lean into their turns a
                little bit. We could try slightly tilting the vertical axis of the character as a
                whole, but that would cause problems with the inner foot sinking into the
                ground while the outer foot comes off the ground. A more natural-looking
                result can be achieved by animating three variations on the basic forward walk
                or run—one going perfectly straight, one making an extreme left turn and one
                making an extreme right turn. We can then LERP-blend between the straight
                clip and the extreme left turn clip to implement any desired lean angle.
            */

            // todo: naming
            f32 InterpolationTime = 0.2f;
            vec2 dMove = (State->TargetMove - State->CurrentMove) / InterpolationTime;
            State->CurrentMove += dMove * Parameters->Delta;

            AnimationGraphPerFrameUpdate(&State->Player.AnimationGraph, Parameters->Delta, Clamp(Magnitude(State->CurrentMove), 0.f, 1.f));

            skeleton_pose *Pose = State->Player.Model->Pose;

            CalculateFinalSkeletonPose(&State->Player.AnimationGraph, Pose, &State->WorldArena);

            transform PlayerTransform = CreateTransform(
                PlayerPosition + State->Player.Offset,
                vec3(3.f),
                State->Player.RigidBody->Orientation
            );
            UpdateGlobalJointPoses(Pose, PlayerTransform);

            if (State->ShowModel)
            {
                DrawSkinnedModel(RenderCommands, State->Player.Model, Pose, PlayerTransform, PointLight1, PointLight2);
            }

            if (State->ShowSkeleton)
            {
                DrawSkeleton(RenderCommands, Pose, &State->CubeModel);
            }

#if 1
            vec2 HalfDim = State->FloorDim / 2.f;
            u32 InstanceIndex = 0;

            for (f32 i = -HalfDim.x; i < HalfDim.x; ++i)
            {
                f32 x = i * 4.f + 2.f;

                for (f32 j = -HalfDim.y; j < HalfDim.y; ++j)
                {
                    render_instance *Instance = State->Instances + InstanceIndex++;

                    f32 z = j * 4.f + 1.f;

                    Instance->Model = Transform(CreateTransform(vec3(x, -0.3f, z), vec3(2.f), quat(0.f)));
                }
            }

            DrawModelInstanced(RenderCommands, &State->FloorModel, State->InstanceCount, State->Instances, PointLight1, PointLight2);
#endif

            {
                transform Transform = CreateTransform(PointLight1Position, vec3(1.f), State->Player.RigidBody->Orientation);
                material Material = CreateMaterial(MaterialType_Unlit, PointLight1Color, true);

                DrawModel(RenderCommands, &State->SkullModel, Transform, PointLight1, PointLight2);
            }
            {
                transform Transform = CreateTransform(PointLight2Position, vec3(1.f), State->Player.RigidBody->Orientation);
                material Material = CreateMaterial(MaterialType_Unlit, PointLight2Color, true);

                DrawModel(RenderCommands, &State->SkullModel, Transform, PointLight1, PointLight2);
            }

            for (u32 RigidBodyIndex = 0; RigidBodyIndex < State->RigidBodiesCount; ++RigidBodyIndex)
            {
                rigid_body *Body = State->RigidBodies + RigidBodyIndex;

                vec3 Position = Lerp(Body->PrevPosition, Lag, Body->Position);
                transform Transform = CreateTransform(Position, Body->HalfSize, Body->Orientation);
                material Material = CreateMaterial(MaterialType_Unlit, vec3(1.f, 1.f, 0.f), true);
                DrawModel(RenderCommands, &State->CubeModel, Transform, Material, {}, {});
            }

            break;
        }
        case GameMode_Menu:
        {
            Clear(RenderCommands, vec4(1.f, 1.f, 1.f, 1.f));

            SetOrthographicProjection(RenderCommands,
                -FrustrumWidthInUnits / 2.f, FrustrumWidthInUnits / 2.f,
                -FrustrumHeightInUnits / 2.f, FrustrumHeightInUnits / 2.f,
                -10.f, 10.f
            );

            DrawRectangle(
                RenderCommands, 
                CreateTransform(vec3(-2.f, 2.f, 0.f) * vec3(Cos(Parameters->Time), 1.f, 0.f), vec3(0.5f), quat(0.f)), 
                vec4(1.f, 0.f, 0.f, 1.f)
            );
            DrawRectangle(
                RenderCommands, 
                CreateTransform(vec3(2.f, 2.f, 0.f) *vec3(1.f, Cos(Parameters->Time), 0.f), vec3(0.5f), quat(0.f)), 
                vec4(0.f, 1.f, 0.f, 1.f)
            );
            DrawRectangle(
                RenderCommands, 
                CreateTransform(vec3(2.f, -2.f, 0.f) * vec3(-Cos(Parameters->Time + PI), 1.f, 0.f), vec3(0.5f), quat(0.f)),
                vec4(0.f, 0.f, 1.f, 1.f)
            );
            DrawRectangle(
                RenderCommands, 
                CreateTransform(vec3(-2.f, -2.f, 0.f) * vec3(1.f, Cos(Parameters->Time), 0.f), vec3(0.5f), quat(0.f)), 
                vec4(1.f, 1.f, 0.f, 1.f)
            );

            break;
        }
        default:
        {
            Assert(!"GameMode is not supported");
        }
    }
}
