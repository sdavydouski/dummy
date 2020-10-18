#include "dummy_defs.h"
#include "dummy_math.h"
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

inline joint_pose
Interpolate(joint_pose *A, f32 t, joint_pose *B)
{
    joint_pose Result;

    Result.Translation = Lerp(A->Translation, t, B->Translation);
    Result.Rotation = Slerp(A->Rotation, t, B->Rotation);
    // todo: logarithmic interpolation for scale? (https://www.cmu.edu/biolphys/deserno/pdf/log_interpol.pdf)
    Result.Scale = Lerp(A->Scale, t, B->Scale);

    return Result;
}

inline joint_pose
Interpolate(joint_pose *A, joint_pose *B, joint_pose *C, f32 Alpha, f32 Beta, f32 Gamma)
{
    joint_pose Result;

    // todo: recheck!
    Result.Translation = A->Translation * Alpha + B->Translation * Beta + C->Translation * Gamma;
    Result.Rotation = A->Rotation * Alpha + B->Rotation * Beta + C->Rotation * Gamma;
    Result.Scale = A->Scale * Alpha + B->Scale * Beta + C->Scale * Gamma;

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

inline joint_pose *
GetRootLocalJointPose(skeleton *Skeleton)
{
    joint_pose *Result = Skeleton->LocalJointPoses + 0;
    return Result;
}

inline joint_pose *
GetRootTranslationLocalJointPose(skeleton_pose *SkeletonPose)
{
    joint_pose *Result = SkeletonPose->LocalJointPoses + 1;
    return Result;
}

inline joint_pose *
GetRootTranslationLocalJointPose(skeleton *Skeleton)
{
    joint_pose *Result = Skeleton->LocalJointPoses + 1;
    return Result;
}

// todo: looping animations has discontinuity between last and first key frames (assimp issue?)
// todo!!!: something is wrong with animation for some models
internal void
AnimateSkeletonPose(skeleton_pose *SkeletonPose, animation_clip *Animation, f32 CurrentTime)
{
    for (u32 JointIndex = 0; JointIndex < SkeletonPose->Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;

        animation_sample *PoseSample = GetAnimationSampleByJointIndex(Animation, JointIndex);

        if (PoseSample && PoseSample->KeyFrameCount > 1)
        {
            closest_key_frames_result ClosestKeyFrames = FindClosestKeyFrames(PoseSample, CurrentTime);
            key_frame *CurrentKeyFrame = ClosestKeyFrames.Current;
            key_frame *NextKeyFrame = ClosestKeyFrames.Next;

            f32 t = (CurrentTime - CurrentKeyFrame->Time) / (Abs(NextKeyFrame->Time - CurrentKeyFrame->Time));

            Assert(t >= 0.f && t <= 1.f);

            *LocalJointPose = Interpolate(&CurrentKeyFrame->Pose, t, &NextKeyFrame->Pose);
        }
    }

    if (Animation->InPlace)
    {
        joint_pose *RootTranslationLocalJointPose = GetRootTranslationLocalJointPose(SkeletonPose);
        RootTranslationLocalJointPose->Translation = vec3(0.f, RootTranslationLocalJointPose->Translation.y, 0.f);
    }
}

// todo: merge these two functions
internal void
AnimateSkeleton(skeleton *Skeleton, animation_clip *Animation, f32 CurrentTime)
{
    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;

        animation_sample *PoseSample = GetAnimationSampleByJointIndex(Animation, JointIndex);

        if (PoseSample && PoseSample->KeyFrameCount > 1)
        {
            closest_key_frames_result ClosestKeyFrames = FindClosestKeyFrames(PoseSample, CurrentTime);
            key_frame *CurrentKeyFrame = ClosestKeyFrames.Current;
            key_frame *NextKeyFrame = ClosestKeyFrames.Next;

            f32 t = (CurrentTime - CurrentKeyFrame->Time) / (Abs(NextKeyFrame->Time - CurrentKeyFrame->Time));

            Assert(t >= 0.f && t <= 1.f);

            *LocalJointPose = Interpolate(&CurrentKeyFrame->Pose, t, &NextKeyFrame->Pose);
        }
    }

    if (Animation->InPlace)
    {
        joint_pose *RootTranslationLocalJointPose = GetRootTranslationLocalJointPose(Skeleton);
        RootTranslationLocalJointPose->Translation = vec3(0.f, RootTranslationLocalJointPose->Translation.y, 0.f);
    }
}

inline b32
InRange(f32 Min, f32 Max, f32 Value)
{
    b32 Result = Value >= Min && Value <= Max;
    return Result;
}

internal void
TriangularLerpBlend(
    animation_blend_space_2d_triangle *AnimationBlendSpaceTriangle,
    vec2 Blend, 
    skeleton *Skeleton, 
    memory_arena *Arena
)
{
    scoped_memory ScopedMemory(Arena);
    skeleton_pose *SkeletonPoses = PushArray(ScopedMemory.Arena, 3, skeleton_pose);
    
    for (u32 SkeletonPoseIndex = 0; SkeletonPoseIndex < 3; ++SkeletonPoseIndex)
    {
        skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex;
        SkeletonPose->Skeleton = Skeleton;
        SkeletonPose->LocalJointPoses = PushArray(ScopedMemory.Arena, Skeleton->JointCount, joint_pose);

        for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
        {
            joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;
            joint_pose *SkeletonLocalJointPose = Skeleton->LocalJointPoses + JointIndex;

            *LocalJointPose = *SkeletonLocalJointPose;
        }
    }

    skeleton_pose *SkeletonPoseA = SkeletonPoses + 0;
    skeleton_pose *SkeletonPoseB = SkeletonPoses + 1;
    skeleton_pose *SkeletonPoseC = SkeletonPoses + 2;

    animation_blend_space_value_2d *A = &AnimationBlendSpaceTriangle->Points[0];
    animation_blend_space_value_2d *B = &AnimationBlendSpaceTriangle->Points[1];
    animation_blend_space_value_2d *C = &AnimationBlendSpaceTriangle->Points[2];

    AnimateSkeletonPose(SkeletonPoseA, A->AnimationState->Animation, A->AnimationState->Time);
    AnimateSkeletonPose(SkeletonPoseB, B->AnimationState->Animation, B->AnimationState->Time);
    AnimateSkeletonPose(SkeletonPoseC, C->AnimationState->Animation, C->AnimationState->Time);

    f32 Alpha, Beta, Gamma;
    Barycentric(Blend, A->Value, B->Value, C->Value, Alpha, Beta, Gamma);

    Assert(Abs(1.f - (Alpha + Beta + Gamma)) < 0.0001f);

    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;

        joint_pose *LocalJointPoseA = SkeletonPoseA->LocalJointPoses + JointIndex;
        joint_pose *LocalJointPoseB = SkeletonPoseB->LocalJointPoses + JointIndex;
        joint_pose *LocalJointPoseC = SkeletonPoseC->LocalJointPoses + JointIndex;

        *LocalJointPose = Interpolate(LocalJointPoseA, LocalJointPoseB, LocalJointPoseC, Alpha, Beta, Gamma);
    }
}

internal void
LinearLerpBlend(animation_blend_space_1d *AnimationBlendSpace, f32 BlendParameter, skeleton *Skeleton, memory_arena *Arena)
{
    animation_blend_space_value_1d *AnimationBlendValue1 = 0;
    animation_blend_space_value_1d *AnimationBlendValue2 = 0;

    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < AnimationBlendSpace->AnimationBlendSpaceValueCount - 1; ++AnimationBlendValueIndex)
    {
        animation_blend_space_value_1d *CurrentAnimationBlendValue = AnimationBlendSpace->AnimationBlendSpaceValues + AnimationBlendValueIndex;
        animation_blend_space_value_1d *NextAnimationBlendValue = AnimationBlendSpace->AnimationBlendSpaceValues + AnimationBlendValueIndex + 1;

        if (InRange(CurrentAnimationBlendValue->Value, NextAnimationBlendValue->Value, BlendParameter))
        {
            AnimationBlendValue1 = CurrentAnimationBlendValue;
            AnimationBlendValue2 = NextAnimationBlendValue;
            break;
        }
    }

    Assert(AnimationBlendValue1 && AnimationBlendValue2);

    scoped_memory ScopedMemory(Arena);

    skeleton_pose *SkeletonPoses = PushArray(ScopedMemory.Arena, 2, skeleton_pose);

    skeleton_pose *SkeletonPoseA = SkeletonPoses + 0;
    SkeletonPoseA->Skeleton = Skeleton;
    SkeletonPoseA->LocalJointPoses = PushArray(ScopedMemory.Arena, Skeleton->JointCount, joint_pose);

    skeleton_pose *SkeletonPoseB = SkeletonPoses + 1;
    SkeletonPoseB->Skeleton = Skeleton;
    SkeletonPoseB->LocalJointPoses = PushArray(ScopedMemory.Arena, Skeleton->JointCount, joint_pose);

    AnimateSkeletonPose(SkeletonPoseA, AnimationBlendValue1->AnimationState->Animation, AnimationBlendValue1->AnimationState->Time);
    AnimateSkeletonPose(SkeletonPoseB, AnimationBlendValue2->AnimationState->Animation, AnimationBlendValue2->AnimationState->Time);

    f32 BlendFactor = (BlendParameter - AnimationBlendValue1->Value) / (AnimationBlendValue2->Value - AnimationBlendValue1->Value);

    // todo:
    if (AnimationBlendValue1->AnimationState->IsEnabled && !AnimationBlendValue2->AnimationState->IsEnabled)
    {
        BlendFactor = 0.f;
    }
    else if (!AnimationBlendValue1->AnimationState->IsEnabled && AnimationBlendValue2->AnimationState->IsEnabled)
    {
        BlendFactor = 1.f;
    }

    Assert(BlendFactor >= 0.f && BlendFactor <= 1.f);

    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;

        joint_pose *LocalJointPoseA = SkeletonPoseA->LocalJointPoses + JointIndex;
        joint_pose *LocalJointPoseB = SkeletonPoseB->LocalJointPoses + JointIndex;

        *LocalJointPose = Interpolate(LocalJointPoseA, BlendFactor, LocalJointPoseB);
    }
}

// todo: do not operate on skeleton directly. Return skeleton_pose?
internal void
LerpBlend(animation_clip_state *A, animation_clip_state *B, f32 BlendFactor, skeleton *Skeleton, memory_arena *Arena)
{
    scoped_memory ScopedMemory(Arena);
    skeleton_pose *SkeletonPoses = PushArray(ScopedMemory.Arena, 2, skeleton_pose);

    for (u32 SkeletonPoseIndex = 0; SkeletonPoseIndex < 2; ++SkeletonPoseIndex)
    {
        skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex;
        SkeletonPose->Skeleton = Skeleton;
        SkeletonPose->LocalJointPoses = PushArray(ScopedMemory.Arena, Skeleton->JointCount, joint_pose);

        for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
        {
            joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;
            joint_pose *SkeletonLocalJointPose = Skeleton->LocalJointPoses + JointIndex;

            *LocalJointPose = *SkeletonLocalJointPose;
        }
    }

    skeleton_pose *SkeletonPoseA = SkeletonPoses + 0;
    skeleton_pose *SkeletonPoseB = SkeletonPoses + 1;

    AnimateSkeletonPose(SkeletonPoseA, A->Animation, A->Time);
    AnimateSkeletonPose(SkeletonPoseB, B->Animation, B->Time);

    Assert(BlendFactor >= 0.f && BlendFactor <= 1.f);

    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;

        joint_pose *LocalJointPoseA = SkeletonPoseA->LocalJointPoses + JointIndex;
        joint_pose *LocalJointPoseB = SkeletonPoseB->LocalJointPoses + JointIndex;

        *LocalJointPose = Interpolate(LocalJointPoseA, BlendFactor, LocalJointPoseB);
    }
}

#if 0
internal void
BlendAnimationClips(skeleton *Skeleton, animation_state_set *AnimationStateSet, memory_arena *Arena)
{
    scoped_memory ScopedMemory(Arena);

    skeleton_pose *SkeletonPoses = PushArray(ScopedMemory.Arena, AnimationStateSet->AnimationStateCount, skeleton_pose);
    for (u32 SkeletonPoseIndex = 0; SkeletonPoseIndex < AnimationStateSet->AnimationStateCount; ++SkeletonPoseIndex)
    {
        skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex;
        SkeletonPose->Skeleton = Skeleton;
        SkeletonPose->LocalJointPoses = PushArray(ScopedMemory.Arena, Skeleton->JointCount, joint_pose);
    }

    u32 SkeletonPoseIndex = 0;
    for (u32 AnimationStateIndex = 0; AnimationStateIndex < AnimationStateSet->AnimationStateCount; ++AnimationStateIndex)
    {
        animation_state *AnimationState = AnimationStateSet->AnimationStates + AnimationStateIndex;
       
        if (AnimationState->IsEnabled)
        {
            skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex++;
            // todo: parallel tasks
            AnimateSkeletonPose(SkeletonPose, AnimationState->Animation, AnimationState->Time);
        }
    }

    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;

        vec3 Translation = vec3(0.f);
        quat Rotation = quat(0.f);
        vec3 Scale = vec3(0.f);
        
        f32 TotalWeight = 0.f;
        u32 SkeletonPoseIndex = 0;
        for (u32 AnimationStateIndex = 0; AnimationStateIndex < AnimationStateSet->AnimationStateCount; ++AnimationStateIndex)
        {
            animation_state *AnimationState = AnimationStateSet->AnimationStates + AnimationStateIndex;

            if (AnimationState->IsEnabled)
            {
                skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex++;
                joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;

                Translation += AnimationState->Weight * LocalJointPose->Translation;
                Rotation += AnimationState->Weight * LocalJointPose->Rotation;
                Scale += AnimationState->Weight * LocalJointPose->Scale;

                TotalWeight += AnimationState->Weight;
            }
        }

        if (TotalWeight > 0.f)
        {
            Assert(TotalWeight == 1.f);

            LocalJointPose->Translation = Translation;
            LocalJointPose->Rotation = Rotation;
            LocalJointPose->Scale = Scale;
        }
    }
}
#endif

inline void
DrawSkinnedModel(render_commands *RenderCommands, model *Model, transform Transform, point_light PointLight1, point_light PointLight2)
{
    Assert(Model->Skeleton);

    joint_pose *RootLocalJointPose = GetRootLocalJointPose(Model->Skeleton);
    RootLocalJointPose->Translation = Transform.Translation;
    RootLocalJointPose->Rotation = Transform.Rotation;
    RootLocalJointPose->Scale = Transform.Scale;

    for (u32 JointIndex = 0; JointIndex < Model->Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Model->Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Model->Skeleton->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Model->Skeleton->GlobalJointPoses + JointIndex;
        mat4 *SkinningMatrix = Model->SkinningMatrices + JointIndex;

        *GlobalJointPose = CalculateGlobalJointPose(Joint, LocalJointPose, Model->Skeleton);
        *SkinningMatrix = *GlobalJointPose * Joint->InvBindTranform;
    }

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
        material Material = CreateMaterial(MaterialType_Phong, MeshMaterial, false);

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
        material Material = CreateMaterial(MaterialType_Phong, MeshMaterial, false);

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
    persist u32 MeshId = 0;

    return MeshId++;
}

// todo:
inline u32
GenerateTextureId()
{
    persist u32 TextureId = 0;

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
    // todo: do not draw root joint?
    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Skeleton->GlobalJointPoses + JointIndex;

        transform Transform = CreateTransform(GetTranslation(*GlobalJointPose), vec3(0.05f), quat(0.f));
        material Material = CreateMaterial(MaterialType_Unlit, vec3(1.f, 1.f, 0.f), false);

        DrawModel(RenderCommands, JointModel, Transform, Material, {}, {});

        if (Joint->ParentIndex > -1)
        {
            mat4 *ParentGlobalJointPose = Skeleton->GlobalJointPoses + Joint->ParentIndex;

            vec3 LineStart = GetTranslation(*ParentGlobalJointPose);
            vec3 LineEnd = GetTranslation(*GlobalJointPose);

            DrawLine(RenderCommands, LineStart, LineEnd, vec4(1.f, 0.f, 1.f, 1.f), 1.f);
        }
    }
}

// todo: hashtable
internal animation_clip_state *
GetAnimation(animation_state_set *AnimationStateSet, const char *AnimationClipName)
{
    animation_clip_state *Result = 0;

    for (u32 AnimationStateIndex = 0; AnimationStateIndex < AnimationStateSet->AnimationStateCount; ++AnimationStateIndex)
    {
        animation_clip_state *AnimationState = AnimationStateSet->AnimationStates + AnimationStateIndex;

        if (StringEquals(AnimationState->Animation->Name, AnimationClipName))
        {
            Result = AnimationState;
            break;
        }
    }

    return Result;
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
    InitCamera(&State->PlayerCamera, RADIANS(45.f), RADIANS(0.f), RADIANS(45.f), vec3(0.f, 0.f, 0.f));
    State->PlayerCamera.Radius = 16.f;

    State->GridCount = 100;

    State->RigidBodiesCount = 1;
    State->RigidBodies = PushArray(&State->WorldArena, State->RigidBodiesCount, rigid_body);

    {
        rigid_body *Body = State->RigidBodies + 0;
        *Body = {};
        Body->PrevPosition = vec3(0.f, 3.f, 0.f);
        Body->Position = vec3(0.f, 3.f, 0.f);
        Body->Damping = 0.000001f;
        Body->HalfSize = vec3(1.f, 3.f, 1.f);
        SetRigidBodyMass(Body, 100.f);
    }

    State->Ground = ComputePlane(vec3(-1.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f), vec3(1.f, 0.f, 0.f));
    State->BackgroundColor = NormalizeRGB(vec3(6.f));

    ClearRenderCommands(Memory);
    render_commands *RenderCommands = GetRenderCommands(Memory);
    InitRenderer(RenderCommands, State->GridCount);

    model_asset *YBotModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\ybot.asset", &State->WorldArena);
    InitModel(YBotModelAsset, &State->YBotModel, &State->WorldArena, RenderCommands);

    model_asset *MutantModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\mutant.asset", &State->WorldArena);
    InitModel(MutantModelAsset, &State->MutantModel, &State->WorldArena, RenderCommands);

    model_asset *ArissaModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\arissa.asset", &State->WorldArena);
    InitModel(ArissaModelAsset, &State->ArissaModel, &State->WorldArena, RenderCommands);

    model_asset *LightModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\sphere.asset", &State->WorldArena);
    InitModel(LightModelAsset, &State->LightModel, &State->WorldArena, RenderCommands);

    model_asset *CubeModelAsset = LoadModelAsset(Memory->Platform, (char *)"assets\\cube.asset", &State->WorldArena);
    InitModel(CubeModelAsset, &State->CubeModel, &State->WorldArena, RenderCommands);

    State->Player = {};
    State->Player.Direction = vec3(0.f, 0.f, 1.f);
    State->Player.State = PlayerState_Idle;
    State->Player.Model = &State->YBotModel;
    State->Player.RigidBody = State->RigidBodies + 0;
    State->Player.Offset = vec3(0.f, -3.f, 0.f);

    Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);

    State->PlayerAnimationStateSet = {};
    // todo: ?
    State->PlayerAnimationStateSet.AnimationStateCount = State->Player.Model->AnimationCount;
    // todo: push&clear?
    State->PlayerAnimationStateSet.AnimationStates = PushArray(&State->WorldArena, State->PlayerAnimationStateSet.AnimationStateCount, animation_clip_state);

    for (u32 AnimationStateIndex = 0; AnimationStateIndex < State->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
    {
        animation_clip_state *AnimationState = State->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;
        AnimationState->Animation = State->Player.Model->Animations + AnimationStateIndex;
        AnimationState->Time = 0.f;
        AnimationState->Weight = 0.f;
        AnimationState->PlaybackRate = 1.f;
        AnimationState->IsEnabled = false;
    }

#if 0
    State->LocomotionBlendSpace = {};
    State->LocomotionBlendSpace.AnimationBlendSpaceValueCount = 3;
    State->LocomotionBlendSpace.AnimationBlendSpaceValues = PushArray(&State->WorldArena, State->LocomotionBlendSpace.AnimationBlendSpaceValueCount, animation_blend_space_value_1d);

    {
        animation_blend_space_value_1d *AnimationBlendValue = State->LocomotionBlendSpace.AnimationBlendSpaceValues + 0;
        AnimationBlendValue->AnimationState = GetAnimation(&State->PlayerAnimationStateSet, "Left_Turn_90");
        AnimationBlendValue->Value = -PI / 2.f;
    }

    {
        animation_blend_space_value_1d *AnimationBlendValue = State->LocomotionBlendSpace.AnimationBlendSpaceValues + 1;
        AnimationBlendValue->AnimationState = GetAnimation(&State->PlayerAnimationStateSet, "Walking");
        AnimationBlendValue->Value = 0.f;
    }

    {
        animation_blend_space_value_1d *AnimationBlendValue = State->LocomotionBlendSpace.AnimationBlendSpaceValues + 2;
        AnimationBlendValue->AnimationState = GetAnimation(&State->PlayerAnimationStateSet, "Right_Turn_90");
        AnimationBlendValue->Value = PI / 2.f;
    }
#endif

#if 0
    animation_clip_state *IdleAnimation = GetAnimation(&State->PlayerAnimationStateSet, "Idle");
    animation_clip_state *WalkingAnimation = GetAnimation(&State->PlayerAnimationStateSet, "Walking");
    animation_clip_state *RightStrafeWalkingAnimation = GetAnimation(&State->PlayerAnimationStateSet, "Right_Strafe_Walking");
    animation_clip_state *LeftStrafeWalkingAnimation = GetAnimation(&State->PlayerAnimationStateSet, "Left_Strafe_Walking");

    State->LocomotionBlendSpace2D = {};
    State->LocomotionBlendSpace2D.AnimationBlendSpaceTriangleCount = 2;
    State->LocomotionBlendSpace2D.AnimationBlendSpaceTriangles = 
        PushArray(&State->WorldArena, State->LocomotionBlendSpace2D.AnimationBlendSpaceTriangleCount, animation_blend_space_2d_triangle);

    {
        animation_blend_space_2d_triangle *AnimationBlendTriangle = State->LocomotionBlendSpace2D.AnimationBlendSpaceTriangles + 0;

        AnimationBlendTriangle->Points[0].AnimationState = IdleAnimation;
        AnimationBlendTriangle->Points[0].Value = vec2(0.f);

        AnimationBlendTriangle->Points[1].AnimationState = WalkingAnimation;
        AnimationBlendTriangle->Points[1].Value = vec2(0.f, 1.f);

        AnimationBlendTriangle->Points[2].AnimationState = RightStrafeWalkingAnimation;
        AnimationBlendTriangle->Points[2].Value = vec2(1.f, 0.f);
    }

    {
        animation_blend_space_2d_triangle *AnimationBlendTriangle = State->LocomotionBlendSpace2D.AnimationBlendSpaceTriangles + 1;

        AnimationBlendTriangle->Points[0].AnimationState = IdleAnimation;
        AnimationBlendTriangle->Points[0].Value = vec2(0.f);

        AnimationBlendTriangle->Points[1].AnimationState = WalkingAnimation;
        AnimationBlendTriangle->Points[1].Value = vec2(0.f, 1.f);

        AnimationBlendTriangle->Points[2].AnimationState = LeftStrafeWalkingAnimation;
        AnimationBlendTriangle->Points[2].Value = vec2(-1.f, 0.f);
    }

    State->Blend = vec2(0.f);
#endif
}

// todo:
// 1. Merge animations into one asset file.
// 2. Figure out locomotion-based animation movement.

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
            State->PlayerCamera.Pitch = Clamp(State->PlayerCamera.Pitch, RADIANS(-89.f), RADIANS(89.f));

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

            State->Player.RigidBody->Acceleration.x = (xMoveX + xMoveY) * 75.f;
            State->Player.RigidBody->Acceleration.z = (zMoveX + zMoveY) * 75.f;
            //joint_pose *RootTranslationLocalJointPose = GetRootTranslationLocalJointPose(State->Player.Model->Skeleton);
            //State->Player.RigidBody->Position.x += RootTranslationLocalJointPose->Translation.x;
            //State->Player.RigidBody->Position.z += RootTranslationLocalJointPose->Translation.z;

            f32 MoveMaginute = Magnitude(Move);
            
            f32 Clock = 1.f;

            /*
                Rotate the characters towards their desired lookat vector, and then take the speed of the character + 
                their relative angle between their velocity and their current forward vector to feed into a 360 motion locomotion blendspace.
            */

            //vec3 PlayerDirection = xMoveAxis * Move.x + yMoveAxis * Move.y;
            vec3 PlayerDirection = yMoveAxis;

            State->Blend = Move;

#if 1
            // todo: continue
            switch (State->Player.State)
            {
                case PlayerState_Idle:
                {
                    if (MoveMaginute > 0.f)
                    {
                        // todo: IdleToWalking state?
                        // todo: https://docs.unity3d.com/ScriptReference/Animation.CrossFade.html
                        State->Player.State = PlayerState_Walking;

                        State->Player.Direction = PlayerDirection;

                        // todo: Cross-fade (fade-out IdleAnimation, fade-in WalkingAnimation)
                        animation_clip_state *IdleAnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Idle");
                        animation_clip_state *WalkingAnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Walking");
                        IdleAnimationClipState->Time = 0.f;
                        IdleAnimationClipState->IsEnabled = false;
                        WalkingAnimationClipState->Time = 0.f;
                        WalkingAnimationClipState->IsEnabled = true;

#if 0
                        {
                            animation_clip_state *AnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Walking");
                            AnimationClipState->IsEnabled = true;
                        }

                        {
                            animation_clip_state *AnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Left_Turn_90");
                            AnimationClipState->IsEnabled = true;
                        }

                        {
                            animation_clip_state *AnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Right_Turn_90");
                            AnimationClipState->IsEnabled = true;
                        }
#endif
                    }

                    break;
                }
                case PlayerState_Walking:
                {
#if 1
                    if (MoveMaginute < 0.0001f)
                    {
                        State->Player.State = PlayerState_Idle;

                        // todo: Cross-fade (fade-in IdleAnimation, fade-out WalkingAnimation)
                        animation_clip_state *IdleAnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Idle");
                        animation_clip_state *WalkingAnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Walking");
                        IdleAnimationClipState->Time = 0.f;
                        IdleAnimationClipState->IsEnabled = true;
                        WalkingAnimationClipState->Time = 0.f;
                        WalkingAnimationClipState->IsEnabled = false;
                    }
                    else
                    {
                        State->Player.Direction = PlayerDirection;
                    }
#endif

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
    
    Input->ZoomDelta = 0.f;
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
        }
    }
}

extern "C" DLLExport 
GAME_RENDER(GameRender)
{
    game_state *State = (game_state *)Memory->PermanentStorage;
    render_commands *RenderCommands = GetRenderCommands(Memory);

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
            f32 Bounds = 100.f;

            DrawLine(RenderCommands, vec3(-Bounds, 0.f, 0.f), vec3(Bounds, 0.f, 0.f), vec4(NormalizeRGB(vec3(255, 51, 82)), 1.f), 1.f);
            DrawLine(RenderCommands, vec3(0.f, -Bounds, 0.f), vec3(0.f, Bounds, 0.f), vec4(NormalizeRGB(vec3(135, 213, 2)), 1.f), 1.f);
            DrawLine(RenderCommands, vec3(0.f, 0.f, -Bounds), vec3(0.f, 0.f, Bounds), vec4(NormalizeRGB(vec3(40, 144, 255)), 1.f), 1.f);

            DrawGrid(RenderCommands, Bounds, State->GridCount, Camera->Position, vec3(0.f, 0.f, 1.f));

            directional_light DirectionalLight = {};
            DirectionalLight.Color = vec3(0.75f);
            DirectionalLight.Direction = vec3(0.f, -0.5f, -0.3f);

            SetDirectionalLight(RenderCommands, DirectionalLight);

            f32 PointLightRadius = 6.f;
            vec3 PointLight1Position = vec3(Cos(Parameters->Time) * PointLightRadius, 1.f, Sin(Parameters->Time) * PointLightRadius);
            vec3 PointLight1Color = vec3(1.f, 1.f, 0.f);

            vec3 PointLight2Position = vec3(-Cos(Parameters->Time) * PointLightRadius, 4.f, Sin(Parameters->Time) * PointLightRadius);
            vec3 PointLight2Color = vec3(1.f, 0.f, 1.f);

            point_light DummyPointLight = {};
            DummyPointLight.Attenuation.Constant = 1.f;
            DummyPointLight.Attenuation.Linear = 1.f;
            DummyPointLight.Attenuation.Quadratic = 1.f;

            {
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

#if 0
                switch (State->Player.State)
                {
                    case PlayerState_Idle:
                    {
                        animation_clip_state *AnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Idle");
                        AnimateSkeleton(State->Player.Model->Skeleton, AnimationClipState->Animation, AnimationClipState->Time);

                        break;
                    }
                    case PlayerState_Walking:
                    {
                        //LerpBlend(&State->LocomotionBlendSpace, State->Angle, State->Player.Model->Skeleton, &State->WorldArena);
                        animation_clip_state *AnimationClipState = GetAnimation(&State->PlayerAnimationStateSet, "Walking");
                        AnimateSkeleton(State->Player.Model->Skeleton, AnimationClipState->Animation, AnimationClipState->Time);

                        break;
                    }
                    default:
                    {
                        Assert(!"Invalid state");
                    }
                }
#endif

                animation_clip_state *EnabledAnimationClipState = 0;
                for (u32 AnimationClipStateIndex = 0; AnimationClipStateIndex < State->PlayerAnimationStateSet.AnimationStateCount; ++AnimationClipStateIndex)
                {
                    animation_clip_state *AnimationClipState = State->PlayerAnimationStateSet.AnimationStates + AnimationClipStateIndex;

                    if (AnimationClipState->IsEnabled)
                    {
                        EnabledAnimationClipState = AnimationClipState;
                        break;
                    }
                }
                if (EnabledAnimationClipState)
                {
                    AnimateSkeleton(State->Player.Model->Skeleton, EnabledAnimationClipState->Animation, EnabledAnimationClipState->Time);
                }

                // todo: animation fade in/fade out

#if 0
                // todo: check if a point is in triangle
                animation_blend_space_2d_triangle *AnimationBlendSpaceTriangle = 0;

                if (State->Blend.x > 0.f)
                {
                    AnimationBlendSpaceTriangle = State->LocomotionBlendSpace2D.AnimationBlendSpaceTriangles + 0;
                }
                else
                {
                    AnimationBlendSpaceTriangle = State->LocomotionBlendSpace2D.AnimationBlendSpaceTriangles + 1;
                }

                animation_clip_state *AnimationState = 0;
                AnimationState->Weight += Parameters->Delta;
                if (AnimationState->Weight > 1.f)
                {
                    AnimationState->Weight = 1.f;
                }

                TriangularLerpBlend(
                    AnimationBlendSpaceTriangle,
                    State->Blend, 
                    State->Player.Model->Skeleton, 
                    &State->WorldArena
                );
#endif

                for (u32 AnimationStateIndex = 0; AnimationStateIndex < State->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
                {
                    animation_clip_state *AnimationState = State->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;

                    if (AnimationState->IsEnabled)
                    {
                        AnimationState->Time += Parameters->Delta * AnimationState->PlaybackRate;

                        if (AnimationState->Time > AnimationState->Animation->Duration)
                        {
                            if (AnimationState->Animation->IsLooping)
                            {
                                AnimationState->Time = 0.f;
                            }
                            else
                            {
                                AnimationState->Time = 0.f;
                                AnimationState->IsEnabled = false;
                            }
                        }
                    }
                }

                transform Transform = CreateTransform(
                    PlayerPosition + State->Player.Offset,
                    vec3(3.f),
                    //quat(0.f)
                    AxisAngle2Quat(vec4(yAxis, Atan2(State->Player.Direction.x, State->Player.Direction.z)))
                );
                DrawSkinnedModel(RenderCommands, State->Player.Model, Transform, PointLight1, PointLight2);

                //DrawSkeleton(RenderCommands, State->DummyModel.Skeleton, &State->CubeModel);
            }
#if 1
            {
                transform Transform = CreateTransform(PointLight1Position, vec3(20.f), quat(0.f));
                material Material = CreateMaterial(MaterialType_Unlit, PointLight1Color, true);

                DrawModel(RenderCommands, &State->LightModel, Transform, Material, {}, {});
            }
            {
                transform Transform = CreateTransform(PointLight2Position, vec3(20.f), quat(0.f));
                material Material = CreateMaterial(MaterialType_Unlit, PointLight2Color, true);

                DrawModel(RenderCommands, &State->LightModel, Transform, Material, {}, {});
            }
#endif

            for (u32 RigidBodyIndex = 0; RigidBodyIndex < State->RigidBodiesCount; ++RigidBodyIndex)
            {
                rigid_body *Body = State->RigidBodies + RigidBodyIndex;

                vec3 Position = Lerp(Body->PrevPosition, Lag, Body->Position);
                transform Transform = CreateTransform(Position, Body->HalfSize, quat(0.f));
                material Material = CreateMaterial(MaterialType_Unlit, vec3(1.f, 1.f, 0.f), true);
                //DrawModel(RenderCommands, &State->CubeModel, Transform, Material, {}, {});
            }

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
