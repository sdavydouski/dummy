#include "dummy_defs.h"
#include "dummy_math.h"
#include "dummy_memory.h"
#include "dummy_string.h"
#include "dummy_platform.h"
#include "dummy_collision.h"
#include "dummy_physics.h"
#include "dummy_renderer.h"
#include "dummy_assets.h"
#include "dummy.h"

#include "dummy_assets.cpp"
#include "dummy_renderer.cpp"

inline vec3
NormalizeRGB(vec3 RGB)
{
    vec3 Result = RGB / 255.f;
    return Result;
}

#if 0
inline mesh_material
CreateMeshMaterial(memory_arena *Arena, material_property_type Type, f32 Value)
{
    mesh_material Result = {};
    Result.PropertyCount = 1;
    Result.Properties = PushArray(Arena, 1, material_property);

    material_property *MaterialProperty = Result.Properties + 0;
    MaterialProperty->Type = Type;
    MaterialProperty->Value = Value;

    return Result;
}

inline mesh_material
CreateMeshMaterial(memory_arena *Arena, material_property_type Type, vec4 Color)
{
    mesh_material Result = {};
    Result.PropertyCount = 1;
    Result.Properties = PushArray(Arena, 1, material_property);

    material_property *MaterialProperty = Result.Properties + 0;
    MaterialProperty->Type = Type;
    MaterialProperty->Color = Color;

    return Result;
}

inline mesh_material
CreateMeshMaterial(memory_arena *Arena, material_property_type Type, bitmap Bitmap)
{
    mesh_material Result = {};
    Result.PropertyCount = 1;
    Result.Properties = PushArray(Arena, 1, material_property);

    material_property *MaterialProperty = Result.Properties + 0;
    MaterialProperty->Type = Type;
    MaterialProperty->Bitmap = Bitmap;

    return Result;
}
#endif

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

inline joint_pose
Interpolate(joint_pose A, f32 t, joint_pose B)
{
    joint_pose Result;

    Result.Translation = Lerp(A.Translation, t, B.Translation);
    Result.Rotation = Slerp(A.Rotation, t, B.Rotation);
    // todo: logarithmic interpolation for scale? (https://www.cmu.edu/biolphys/deserno/pdf/log_interpol.pdf)
    Result.Scale = Lerp(A.Scale, t, B.Scale);

    return Result;
}

internal mat4
CalculateGlobalJointPose(joint *CurrentJoint, joint_pose *CurrentJointPose, skeleton *Skeleton)
{
    mat4 Result = mat4(1.f);

    while (true)
    {
        mat4 Pose = Transform(*CurrentJointPose);
        Result = Pose * Result;

        if (CurrentJoint->ParentIndex == -1)
        {
            break;
        }

        CurrentJointPose = Skeleton->LocalJointPoses + CurrentJoint->ParentIndex;
        CurrentJoint = Skeleton->Joints + CurrentJoint->ParentIndex;
    }

    return Result;
}

// todo: optimize (caching?)
inline animation_sample *
GetAnimationSampleByJointIndex(animation_clip *Animation, u32 JointIndex)
{
    animation_sample *Result = 0;
    
    for (u32 AnimationSampleIndex = 0; AnimationSampleIndex < Animation->PoseSampleCount; ++AnimationSampleIndex)
    {
        animation_sample *AnimationSample = Animation->PoseSamples + AnimationSampleIndex;

        if (AnimationSample->JointIndex == JointIndex)
        {
            Result = AnimationSample;
            break;
        }
    }

    return Result;
}

struct closest_key_frames_result
{
    key_frame *Current;
    key_frame *Next;
};

internal closest_key_frames_result
FindClosestKeyFrames(animation_sample *PoseSample, f32 CurrentTime)
{
    Assert(PoseSample->KeyFrameCount > 1);

    closest_key_frames_result Result = {};

    for (u32 KeyFrameIndex = 0; KeyFrameIndex < PoseSample->KeyFrameCount - 1; ++KeyFrameIndex)
    {
        key_frame *CurrentKeyFrame = PoseSample->KeyFrames + KeyFrameIndex;
        key_frame *NextKeyFrame = PoseSample->KeyFrames + KeyFrameIndex + 1;

        if (CurrentKeyFrame->Time <= CurrentTime && CurrentTime < NextKeyFrame->Time)
        {
            Result.Current = CurrentKeyFrame;
            Result.Next = NextKeyFrame;
            break;
        }
    }

    if (!Result.Current || !Result.Next)
    {
        Result.Current = PoseSample->KeyFrames + (PoseSample->KeyFrameCount - 1);
        Result.Next = PoseSample->KeyFrames + 0;
    }

    return Result;
}

// todo: looping animations has discontinuity between last and first key frames (assimp issue?)
// todo!!!: something is wrong with animation for some models
internal void
AnimateSkeleton(animation_clip *Animation, f32 CurrentTime, skeleton *Skeleton)
{
    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Skeleton->GlobalJointPoses + JointIndex;

        animation_sample *PoseSample = GetAnimationSampleByJointIndex(Animation, JointIndex);

        if (PoseSample && PoseSample->KeyFrameCount > 1)
        {
            closest_key_frames_result ClosestKeyFrames = FindClosestKeyFrames(PoseSample, CurrentTime);
            key_frame *CurrentKeyFrame = ClosestKeyFrames.Current;
            key_frame *NextKeyFrame = ClosestKeyFrames.Next;

            f32 t = (CurrentTime - CurrentKeyFrame->Time) / (Abs(NextKeyFrame->Time - CurrentKeyFrame->Time));

            Assert(t >= 0.f && t <= 1.f);

            *LocalJointPose = Interpolate(CurrentKeyFrame->Pose, t, NextKeyFrame->Pose);
            *GlobalJointPose = CalculateGlobalJointPose(Joint, LocalJointPose, Skeleton);
        }
    }
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

inline void
DrawSkinnedModel(render_commands *RenderCommands, model *Model, transform Transform, point_light PointLight1, point_light PointLight2)
{
    Assert(Model->Skeleton);

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
        material Material = CreateMaterial(MaterialType_Standard, MeshMaterial, true);

        DrawSkinnedMesh(
            RenderCommands, Mesh->Id, Transform, Material, 
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
        material Material = CreateMaterial(MaterialType_Standard, MeshMaterial, true);

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

// todo:
inline u32
GenerateMeshId()
{
    static u32 MeshId = 0;

    return MeshId++;
}

// todo:
inline u32
GenerateTextureId()
{
    static u32 TextureId = 0;

    return TextureId++;
}

inline void
InitModel(model_asset *Asset, model *Model, memory_arena *Arena, render_commands *RenderCommands)
{
    *Model = {};

    Model->Skeleton = &Asset->Skeleton;
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

    if (Model->AnimationCount > 0)
    {
        Model->CurrentAnimation = Model->Animations + 0;
        Model->CurrentTime = 0.f;
        Model->PlaybackRate = 1.f;
    }

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        Mesh->Id = GenerateMeshId();
        AddMesh(RenderCommands, Mesh->Id, Mesh->PrimitiveType, Mesh->VertexCount, Mesh->Vertices, Mesh->IndexCount, Mesh->Indices);

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
DrawSkeleton(render_commands *RenderCommands, skeleton *Skeleton, model *JointModel)
{
    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Skeleton->GlobalJointPoses + JointIndex;

        mat4 S = Scale(3.f);
        mat4 Model = S * (*GlobalJointPose);

        transform Transform = CreateTransform(GetTranslation(Model), vec3(0.05f), quat(0.f));
        material Material = CreateMaterial(MaterialType_Unlit, 0, true);

        DrawModel(RenderCommands, JointModel, Transform, Material, {}, {});

        if (Joint->ParentIndex > -1)
        {
            mat4 *ParentGlobalJointPose = Skeleton->GlobalJointPoses + Joint->ParentIndex;
            mat4 ParentModel = S * (*ParentGlobalJointPose);

            vec3 LineStart = GetTranslation(ParentModel);
            vec3 LineEnd = GetTranslation(Model);

            DrawLine(RenderCommands, LineStart, LineEnd, vec4(1.f, 0.f, 1.f, 1.f), 1.f);
        }
    }
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

    //model_asset *DummyModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\arissa.asset", &State->WorldArena);
    //model_asset *DummyModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\mutant_samba_dancing.asset", &State->WorldArena);
    model_asset *DummyModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\ybot_gangnam_style.asset", &State->WorldArena);
    //model_asset *DummyModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\zlorp_gangnam_style.asset", &State->WorldArena);
    InitModel(DummyModelAsset, &State->DummyModel, &State->WorldArena, RenderCommands);

    model_asset *LightModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\light.asset", &State->WorldArena);
    InitModel(LightModelAsset, &State->LightModel, &State->WorldArena, RenderCommands);

    model_asset *CubeModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\cube.asset", &State->WorldArena);
    InitModel(CubeModelAsset, &State->CubeModel, &State->WorldArena, RenderCommands);
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
            f32 DebugCameraSpeed = 10.f;
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
            
            // todo: SetTransform render command?
            f32 Bounds = 100.f;

            DrawLine(RenderCommands, vec3(-Bounds, 0.f, 0.f), vec3(Bounds, 0.f, 0.f), vec4(NormalizeRGB(vec3(255, 51, 82)), 1.f), 1.f);
            DrawLine(RenderCommands, vec3(0.f, -Bounds, 0.f), vec3(0.f, Bounds, 0.f), vec4(NormalizeRGB(vec3(135, 213, 2)), 1.f), 1.f);
            DrawLine(RenderCommands, vec3(0.f, 0.f, -Bounds), vec3(0.f, 0.f, Bounds), vec4(NormalizeRGB(vec3(40, 144, 255)), 1.f), 1.f);

            DrawGrid(RenderCommands, Bounds, State->GridCount, State->DebugCamera.Position, vec3(0.f, 0.f, 1.f));

            directional_light DirectionalLight = {};
            DirectionalLight.Color = vec3(0.9f);
            DirectionalLight.Direction = vec3(0.f, -0.5f, -0.3f);

            SetDirectionalLight(RenderCommands, DirectionalLight);

#if 0
            for (u32 RigidBodyCount = 0; RigidBodyCount < State->RigidBodiesCount; ++RigidBodyCount)
            {
                rigid_body *Body = State->RigidBodies + RigidBodyCount;

                DrawBox(RenderCommands, Body->Position, Body->HalfSize, BoxMaterial, vec4(Parameters->Time, vec3(0.f, 1.f, 0.f)));
            }
#endif

            f32 CurrentTime = State->DummyModel.CurrentTime * State->DummyModel.PlaybackRate;

            AnimateSkeleton(State->DummyModel.CurrentAnimation, CurrentTime, State->DummyModel.Skeleton);

            State->DummyModel.CurrentTime += Parameters->Delta;
            if (State->DummyModel.CurrentTime > (State->DummyModel.CurrentAnimation->Duration / State->DummyModel.PlaybackRate))
            {
                State->DummyModel.CurrentTime = 0.f;
            }

            f32 Scale = 3.f;

            mat4 Model = Transform(CreateTransform(vec3(0.f, 0.f, 0.f), vec3(Scale), quat(0.f)));

#if 1
            skeleton *Skeleton = State->DummyModel.Skeleton;
            for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
            {
                joint *Joint = Skeleton->Joints + JointIndex;
                joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;
                mat4 *SkinningMatrix = State->DummyModel.SkinningMatrices + JointIndex;

                mat4 GlobalJointPose = CalculateGlobalJointPose(Joint, LocalJointPose, Skeleton);
                *SkinningMatrix = Model * GlobalJointPose * Joint->InvBindTranform;
        }
#endif

            f32 PointLightRadius = 6.f;
            vec3 PointLight1Position = vec3(Cos(Parameters->Time) * PointLightRadius, 1.f, Sin(Parameters->Time) * PointLightRadius);
            vec3 PointLight1Color = vec3(1.f, 1.f, 1.f);

            vec3 PointLight2Position = vec3(-Cos(Parameters->Time) * PointLightRadius, 4.f, Sin(Parameters->Time) * PointLightRadius);
            vec3 PointLight2Color = vec3(1.f, 1.f, 1.f);

            point_light DummyPointLight = {};
            DummyPointLight.Attenuation.Constant = 1.f;
            DummyPointLight.Attenuation.Linear = 1.f;
            DummyPointLight.Attenuation.Quadratic = 1.f;

            {
#if 0
                material Material = {};
                Material.Type = MaterialType_Standard;
                Material.DiffuseColor = vec3(1.f, 1.f, 1.f);
                Material.AmbientStrength = 0.1f;
                Material.SpecularStrength = 1.f;
                Material.SpecularShininess = 32;
#endif

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

#if 1
                DrawSkinnedModel(RenderCommands, &State->DummyModel, CreateTransform(vec3(0.f, 0.f, 0.f), vec3(Scale), quat(0.f)), PointLight1, PointLight2);
#endif

#if 0
                DrawModel(
                    RenderCommands, &State->CubeModel, 
                    CreateTransform(vec3(6.f, 1.f, 0.f), vec3(1.f), AxisAngle2Quat(vec4(vec3(0.f, 1.f, 0.f), Cos(Parameters->Time)))), 
                    Material, PointLight1, PointLight2
                );
#endif

                DrawSkeleton(RenderCommands, State->DummyModel.Skeleton, &State->CubeModel);
            }
#if 0
            {
                material Material = {};
                Material.Type = MaterialType_Unlit;
                Material.Color = PointLight1Color;

                //DrawModel(RenderCommands, &State->LightModel, CreateTransform(PointLight1Position, vec3(0.2f), quat(0.f)), Material, {}, {});
            }
            {
                material Material = {};
                Material.Type = MaterialType_Unlit;
                Material.Color = PointLight2Color;

                //DrawModel(RenderCommands, &State->LightModel, CreateTransform(PointLight2Position, vec3(0.2f), quat(0.f)), Material, {}, {});
            }

            {
                material Material = {};
                Material.Type = MaterialType_Unlit;
                Material.Color = vec3(1.f, 1.f, 0.f);

                //DrawMesh(RenderCommands, 3, CreateTransform(vec3(-4.f, 1.f, 0.f), vec3(1.f), quat(0.f)), Material, {}, {});
            }
#endif

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
        }
        break;
        default:
        {
            Assert(!"GameMode is not supported");
        }
    }
}
