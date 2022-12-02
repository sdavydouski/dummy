#include "dummy_defs.h"
#include "dummy_math.h"
#include "dummy_random.h"
#include "dummy_memory.h"
#include "dummy_string.h"
#include "dummy_container.h"
#include "dummy_input.h"
#include "dummy_events.h"
#include "dummy_collision.h"
#include "dummy_physics.h"
#include "dummy_visibility.h"
#include "dummy_spatial.h"
#include "dummy_process.h"
#include "dummy_animation.h"
#include "dummy_assets.h"
#include "dummy_audio.h"
#include "dummy_renderer.h"
#include "dummy_text.h"
#include "dummy_job.h"
#include "dummy_platform.h"
#include "dummy.h"

//#define sid u32
//#define SID(String) Hash(String)

#include "dummy_assets.cpp"
#include "dummy_audio.cpp"
#include "dummy_renderer.cpp"
#include "dummy_events.cpp"
#include "dummy_collision.cpp"
#include "dummy_physics.cpp"
#include "dummy_spatial.cpp"
#include "dummy_animation.cpp"
#include "dummy_animator.cpp"
#include "dummy_process.cpp"
#include "dummy_visibility.cpp"

inline vec3
NormalizeRGB(vec3 RGB)
{
    vec3 Result = RGB / 255.f;
    return Result;
}

inline void
InitCamera(game_camera *Camera, f32 FieldOfView, f32 AspectRatio, f32 NearClipPlane, f32 FarClipPlane, vec3 Position, f32 Pitch, f32 Yaw, vec3 Up = vec3(0.f, 1.f, 0.f))
{
    Camera->Transform = CreateTransform(Position, vec3(1.f), Euler2Quat(Yaw, Pitch, 0.f));
    Camera->Direction = Euler2Direction(Yaw, Pitch);
    Camera->Up = Up;

    Camera->Pitch = Pitch;
    Camera->Yaw = Yaw;

    Camera->FieldOfView = FieldOfView;
    Camera->FocalLength = 1.f / Tan(FieldOfView * 0.5f);
    Camera->AspectRatio = AspectRatio;
    Camera->NearClipPlane = NearClipPlane;
    Camera->FarClipPlane = FarClipPlane;
}

inline material
CreateBasicMaterial(vec4 Color, b32 Wireframe = false, b32 CastShadow = false)
{
    material Result = {};

    Result.Type = MaterialType_Basic;
    Result.Color = Color;
    Result.Wireframe = Wireframe;
    Result.CastShadow = CastShadow;

    return Result;
}

inline material
CreatePhongMaterial(mesh_material *MeshMaterial, b32 Wireframe = false, b32 CastShadow = true)
{
    material Result = {};

    Result.Type = MaterialType_Phong;
    Result.MeshMaterial = MeshMaterial;
    Result.Wireframe = Wireframe;
    Result.CastShadow = CastShadow;

    return Result;
}

inline void
DrawSkinnedModel(render_commands *RenderCommands, model *Model, skeleton_pose *Pose, skinning_data *Skinning)
{
    Assert(Model->Skeleton);

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        if (Mesh->Visible)
        {
            mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
            material Material = CreatePhongMaterial(MeshMaterial);

            DrawSkinnedMesh(
                RenderCommands, Mesh->Id, {}, Material,
                Skinning->SkinningBufferId, Skinning->SkinningMatrixCount, Skinning->SkinningMatrices
            );
        }
    }
}

inline void
DrawModel(render_commands *RenderCommands, model *Model, transform Transform)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        if (Mesh->Visible)
        {
            mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
            material Material = CreatePhongMaterial(MeshMaterial);

            DrawMesh(RenderCommands, Mesh->Id, Transform, Material);
        }
    }
}

inline void
DrawModelInstanced(render_commands *RenderCommands, model *Model, u32 InstanceCount, render_instance *Instances)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        if (Mesh->Visible)
        {
            mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
            material Material = CreatePhongMaterial(MeshMaterial);

            DrawMeshInstanced(RenderCommands, Mesh->Id, InstanceCount, Instances, Material);
        }
    }
}

inline u32
GenerateEntityId(game_state *State)
{
    u32 Result = State->NextFreeEntityId++;
    return Result;
}

inline u32
GenerateMeshId(game_state *State)
{
    u32 Result = State->NextFreeMeshId++;
    return Result;
}

inline u32
GenerateTextureId(game_state *State)
{
    u32 Result = State->NextFreeTextureId++;
    return Result;
}

inline u32
GenerateSkinningBufferId(game_state *State)
{
    u32 Result = State->NextFreeSkinningId++;
    return Result;
}

inline void
InitModel(game_state *State, model_asset *Asset, model *Model, const char *Name, memory_arena *Arena, render_commands *RenderCommands)
{
    *Model = {};

    CopyString(Name, Model->Key);
    Model->Bounds = Asset->Bounds;
    Model->Skeleton = &Asset->Skeleton;
    Model->BindPose = &Asset->BindPose;
    Model->AnimationGraph = &Asset->AnimationGraph;
    Model->MeshCount = Asset->MeshCount;
    Model->Meshes = Asset->Meshes;
    Model->MaterialCount = Asset->MaterialCount;
    Model->Materials = Asset->Materials;
    Model->AnimationCount = Asset->AnimationCount;
    Model->Animations = Asset->Animations;

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        Mesh->Id = GenerateMeshId(State);
        Mesh->Visible = true;

        AddMesh(
            RenderCommands, Mesh->Id, Mesh->VertexCount,
            Mesh->Positions, Mesh->Normals, Mesh->Tangents, Mesh->Bitangents, Mesh->TextureCoords, Mesh->Weights, Mesh->JointIndices,
            Mesh->IndexCount, Mesh->Indices
        );

        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;

        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MeshMaterial->PropertyCount; ++MaterialPropertyIndex)
        {
            material_property *MaterialProperty = MeshMaterial->Properties + MaterialPropertyIndex;

            if (
                MaterialProperty->Type == MaterialProperty_Texture_Diffuse ||
                MaterialProperty->Type == MaterialProperty_Texture_Specular ||
                MaterialProperty->Type == MaterialProperty_Texture_Shininess ||
                MaterialProperty->Type == MaterialProperty_Texture_Normal
                )
            {
                MaterialProperty->TextureId = GenerateTextureId(State);
                AddTexture(RenderCommands, MaterialProperty->TextureId, &MaterialProperty->Bitmap);
            }
        }
    }
}

internal void
InitFont(game_state *State, font_asset *Asset, font *Font, const char *Name, render_commands *RenderCommands)
{
    CopyString(Name, Font->Key);
    Font->TextureAtlas = Asset->TextureAtlas;
    Font->VerticalAdvance = Asset->VerticalAdvance;
    Font->Ascent = Asset->Ascent;
    Font->Descent = Asset->Descent;
    Font->CodepointsRangeCount = Asset->CodepointsRangeCount;
    Font->CodepointsRanges = Asset->CodepointsRanges;
    Font->HorizontalAdvanceTableCount = Asset->HorizontalAdvanceTableCount;
    Font->HorizontalAdvanceTable = Asset->HorizontalAdvanceTable;
    Font->GlyphCount = Asset->GlyphCount;
    Font->Glyphs = Asset->Glyphs;
    Font->TextureId = GenerateTextureId(State);

    AddTexture(RenderCommands, Font->TextureId, &Font->TextureAtlas);
}

internal void
InitAudioClip(audio_clip_asset *Asset, audio_clip *AudioClip, const char *Name)
{
    CopyString(Name, AudioClip->Key);
    AudioClip->Format = Asset->Format;
    AudioClip->Channels = Asset->Channels;
    AudioClip->SamplesPerSecond = Asset->SamplesPerSecond;
    AudioClip->BitsPerSample = Asset->BitsPerSample;
    AudioClip->AudioBytes = Asset->AudioBytes;
    AudioClip->AudioData = Asset->AudioData;
}

inline void
InitSkinningBuffer(game_state *State, skinning_data *Skinning, model *Model, memory_arena *Arena, render_commands *RenderCommands)
{
    *Skinning = {};

    Skinning->Pose = PushType(Arena, skeleton_pose);
    Skinning->Pose->Skeleton = Model->Skeleton;
    Skinning->Pose->LocalJointPoses = PushArray(Arena, Model->Skeleton->JointCount, joint_pose);
    Skinning->Pose->GlobalJointPoses = PushArray(Arena, Model->Skeleton->JointCount, mat4, Align(16));

    for (u32 JointIndex = 0; JointIndex < Model->BindPose->Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *SourceLocalJointPose = Model->BindPose->LocalJointPoses + JointIndex;
        joint_pose *DestLocalJointPose = Skinning->Pose->LocalJointPoses + JointIndex;

        *DestLocalJointPose = *SourceLocalJointPose;
    }

    u32 SkinningMatrixCount = Model->Skeleton->JointCount;

    Skinning->SkinningMatrixCount = Model->Skeleton->JointCount;
    Skinning->SkinningMatrices = PushArray(Arena, SkinningMatrixCount, mat4, Align(16));
    Skinning->SkinningBufferId = GenerateSkinningBufferId(State);

    AddSkinningBuffer(RenderCommands, Skinning->SkinningBufferId, Skinning->SkinningMatrixCount);
}

inline ray
ScreenPointToWorldRay(vec2 ScreenPoint, vec2 ScreenSize, game_camera *Camera)
{
    vec4 ClipRay = vec4(
        2.f * ScreenPoint.x / ScreenSize.x - 1.f,
        1.f - 2.f * ScreenPoint.y / ScreenSize.y,
        -1.f, 1.f
    );

    mat4 View = GetCameraTransform(Camera);

    f32 Aspect = (f32) ScreenSize.x / (f32) ScreenSize.y;
    mat4 Projection = FrustrumProjection(Camera->FieldOfView, Aspect, Camera->NearClipPlane, Camera->FarClipPlane);

    vec4 CameraRay = Inverse(Projection) * ClipRay;
    CameraRay = vec4(CameraRay.xy, -1.f, 0.f);

    vec3 WorldRay = (Inverse(View) * CameraRay).xyz;

    ray Result = {};
    Result.Origin = Camera->Transform.Translation;
    Result.Direction = Normalize(WorldRay);

    return Result;
}

internal entity_render_batch *
GetRenderBatch(game_state *State, char *Name)
{
    entity_render_batch *Result = HashTableLookup(&State->EntityBatches, Name);
    return Result;
}

inline b32
IsRenderBatchEmpty(entity_render_batch *Batch)
{
    b32 Result = StringEquals(Batch->Key, "");
    return Result;
}

internal void
LoadModelAssets(game_assets *Assets, platform_api *Platform)
{
    game_asset ModelAssets[] = {
        {
            "pelegrini",
            "assets\\pelegrini.asset"
        },
        {
            "xbot",
            "assets\\xbot.asset"
        },
        {
            "ybot",
            "assets\\ybot.asset"
        },
        {
            "paladin",
            "assets\\paladin.asset"
        },
        {
            "warrok",
            "assets\\warrok.asset"
        },
        {
            "maw",
            "assets\\maw.asset"
        },
        {
            "cube",
            "assets\\cube.asset"
        },
        {
            "sphere",
            "assets\\sphere.asset"
        },

        // Dungeon Assets
        {
            "cleric",
            "assets\\cleric.asset"
        },

        {
            "floor_standard",
            "assets\\Floor_Standard.asset"
        },
        {
            "wall",
            "assets\\Wall.asset"
        },

        {
            "barrel",
            "assets\\Barrel.asset"
        },
        {
            "crate",
            "assets\\Crate.asset"
        },
        {
            "torch",
            "assets\\torch.asset"
        },
    };

    Assets->ModelAssetCount = ArrayCount(ModelAssets);
    Assets->ModelAssets = PushArray(&Assets->Arena, Assets->ModelAssetCount, game_asset_model);

    for (u32 ModelAssetIndex = 0; ModelAssetIndex < ArrayCount(ModelAssets); ++ModelAssetIndex)
    {
        game_asset GameAsset = ModelAssets[ModelAssetIndex];
        game_asset_model *GameAssetModel = Assets->ModelAssets + ModelAssetIndex;

        model_asset *LoadedAsset = LoadModelAsset(Platform, GameAsset.Path, &Assets->Arena);

        GameAssetModel->GameAsset = GameAsset;
        GameAssetModel->ModelAsset = LoadedAsset;
    }
}

internal void
LoadFontAssets(game_assets *Assets, platform_api *Platform)
{
    game_asset FontAssets[] = {
        {
            "Consolas",
            "assets\\Consolas.asset"
        },
        {
            "Where My Keys",
            "assets\\Where My Keys.asset"
        }
    };

    Assets->FontAssetCount = ArrayCount(FontAssets);
    Assets->FontAssets = PushArray(&Assets->Arena, Assets->FontAssetCount, game_asset_font);

    for (u32 FontAssetIndex = 0; FontAssetIndex < ArrayCount(FontAssets); ++FontAssetIndex)
    {
        game_asset GameAsset = FontAssets[FontAssetIndex];

        game_asset_font *GameAssetFont = Assets->FontAssets + FontAssetIndex;

        font_asset *LoadedAsset = LoadFontAsset(Platform, GameAsset.Path, &Assets->Arena);

        GameAssetFont->GameAsset = GameAsset;
        GameAssetFont->FontAsset = LoadedAsset;
    }
}

internal void
LoadAudioClipAssets(game_assets *Assets, platform_api *Platform)
{
    game_asset AudioClipAssets[] = {
        {
            "Ambient 5",
            "assets\\Ambient 5.asset"
        },
        {
            "samba",
            "assets\\samba.asset"
        },
        {
            "step_metal",
            "assets\\step_metal.asset"
        },
        {
            "step_cloth1",
            "assets\\step_cloth1.asset"
        },
        {
            "step_lth1",
            "assets\\step_lth1.asset"
        },
        {
            "step_lth4",
            "assets\\step_lth4.asset"
        }
    };

    Assets->AudioClipAssetCount = ArrayCount(AudioClipAssets);
    Assets->AudioClipAssets = PushArray(&Assets->Arena, Assets->AudioClipAssetCount, game_asset_audio_clip);

    for (u32 AudioClipAssetIndex = 0; AudioClipAssetIndex < ArrayCount(AudioClipAssets); ++AudioClipAssetIndex)
    {
        game_asset GameAsset = AudioClipAssets[AudioClipAssetIndex];

        game_asset_audio_clip *GameAssetAudioClip = Assets->AudioClipAssets + AudioClipAssetIndex;

        audio_clip_asset *LoadedAsset = LoadAudioClipAsset(Platform, GameAsset.Path, &Assets->Arena);

        GameAssetAudioClip->GameAsset = GameAsset;
        GameAssetAudioClip->AudioAsset = LoadedAsset;
    }
}

internal void
LoadGameAssets(game_assets *Assets, platform_api *Platform)
{
    // todo:
    Assets->State = GameAssetsState_Unloaded;

    LoadModelAssets(Assets, Platform);
    LoadAudioClipAssets(Assets, Platform);

    Assets->State = GameAssetsState_Loaded;
}

struct load_game_assets_job
{
    game_assets *Assets;
    platform_api *Platform;
};

JOB_ENTRY_POINT(LoadGameAssetsJob)
{
    load_game_assets_job *Data = (load_game_assets_job *) Parameters;
    LoadGameAssets(Data->Assets, Data->Platform);
}

internal void
InitGameModelAssets(game_state *State, game_assets *Assets, render_commands *RenderCommands)
{
    // Using a prime table size in conjunction with quadratic probing tends to yield 
    // the best coverage of the available table slots with minimal clustering
    Assets->Models.Count = 251;
    Assets->Models.Values = PushArray(&Assets->Arena, Assets->Models.Count, model);

    Assert(Assets->Models.Count > Assets->ModelAssetCount);

    for (u32 GameAssetModelIndex = 0; GameAssetModelIndex < Assets->ModelAssetCount; ++GameAssetModelIndex)
    {
        game_asset_model *GameAssetModel = Assets->ModelAssets + GameAssetModelIndex;

        model *Model = GetModelAsset(Assets, GameAssetModel->GameAsset.Name);
        InitModel(State, GameAssetModel->ModelAsset, Model, GameAssetModel->GameAsset.Name, &Assets->Arena, RenderCommands);
    }
}

internal void
InitGameFontAssets(game_state *State, game_assets *Assets, render_commands *RenderCommands)
{
    // todo:
    Assets->Fonts.Count = 11;
    Assets->Fonts.Values = PushArray(&Assets->Arena, Assets->Fonts.Count, font);

    Assert(Assets->Fonts.Count > Assets->FontAssetCount);

    for (u32 GameAssetFontIndex = 0; GameAssetFontIndex < Assets->FontAssetCount; ++GameAssetFontIndex)
    {
        game_asset_font *GameAssetFont = Assets->FontAssets + GameAssetFontIndex;

        font *Font = GetFontAsset(Assets, GameAssetFont->GameAsset.Name);
        InitFont(State, GameAssetFont->FontAsset, Font, GameAssetFont->GameAsset.Name, RenderCommands);
    }
}

internal void
InitGameAudioClipAssets(game_assets *Assets)
{
    // todo:
    Assets->AudioClips.Count = 31;
    Assets->AudioClips.Values = PushArray(&Assets->Arena, Assets->AudioClips.Count, audio_clip);

    Assert(Assets->AudioClips.Count > Assets->AudioClipAssetCount);

    for (u32 GameAssetAudioClipIndex = 0; GameAssetAudioClipIndex < Assets->AudioClipAssetCount; ++GameAssetAudioClipIndex)
    {
        game_asset_audio_clip *GameAssetAudioClip = Assets->AudioClipAssets + GameAssetAudioClipIndex;

        audio_clip *AudioClip = GetAudioClipAsset(Assets, GameAssetAudioClip->GameAsset.Name);
        InitAudioClip(GameAssetAudioClip->AudioAsset, AudioClip, GameAssetAudioClip->GameAsset.Name);
    }
}

internal void
DrawSkeleton(render_commands *RenderCommands, game_state *State, skeleton_pose *Pose)
{
    font *Font = GetFontAsset(&State->Assets, "Consolas");

    for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Pose->Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Pose->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Pose->GlobalJointPoses + JointIndex;

        transform Transform = CreateTransform(GetTranslation(*GlobalJointPose), vec3(0.01f), LocalJointPose->Rotation);
        vec4 Color = vec4(1.f, 1.f, 0.f, 1.f);

        DrawBox(RenderCommands, Transform, Color);
        DrawText(RenderCommands, Joint->Name, Font, Transform.Translation, 0.1f, vec4(0.f, 0.f, 1.f, 1.f), DrawText_AlignCenter, DrawText_WorldSpace, true);

        if (Joint->ParentIndex > -1)
        {
            mat4 *ParentGlobalJointPose = Pose->GlobalJointPoses + Joint->ParentIndex;

            vec3 LineStart = GetTranslation(*ParentGlobalJointPose);
            vec3 LineEnd = GetTranslation(*GlobalJointPose);

            DrawLine(RenderCommands, LineStart, LineEnd, vec4(1.f, 0.f, 1.f, 1.f), 2.f);
        }
    }
}

internal void
RenderFrustrum(render_commands *RenderCommands, polyhedron *Frustrum)
{
    // Draw edges
    for (u32 EdgeIndex = 0; EdgeIndex < Frustrum->EdgeCount; ++EdgeIndex)
    {
        edge Edge = Frustrum->Edges[EdgeIndex];

        vec3 LineStart = Frustrum->Vertices[Edge.VertexIndex[0]];
        vec3 LineEnd = Frustrum->Vertices[Edge.VertexIndex[1]];
        DrawLine(RenderCommands, LineStart, LineEnd, vec4(1.f, 0.f, 1.f, 1.f), 4.f);
    }

#if 0
    // Draw inward normals
    for (u32 PlaneIndex = 0; PlaneIndex < Frustrum->FaceCount; ++PlaneIndex)
    {
        plane Plane = Frustrum->Planes[PlaneIndex];
        face Face = Frustrum->Faces[PlaneIndex];

        vec3 MiddleEdgePoints[4];

        for (u32 FaceEdgeIndex = 0; FaceEdgeIndex < Face.EdgeCount; ++FaceEdgeIndex)
        {
            u32 EdgeIndex = Face.EdgeIndex[FaceEdgeIndex];
            edge Edge = Frustrum->Edges[EdgeIndex];

            vec3 Vertex0 = Frustrum->Vertices[Edge.VertexIndex[0]];
            vec3 Vertex1 = Frustrum->Vertices[Edge.VertexIndex[1]];

            vec3 MiddleEdgePoint = (Vertex0 + Vertex1) / 2.f;

            Assert(FaceEdgeIndex < ArrayCount(MiddleEdgePoints));

            MiddleEdgePoints[FaceEdgeIndex] = MiddleEdgePoint;
        }

        vec3 LineStart = (MiddleEdgePoints[0] + MiddleEdgePoints[1] + MiddleEdgePoints[2] + MiddleEdgePoints[3]) / 4.f;
        vec3 LineEnd = LineStart + Plane.Normal * 5.f;
        DrawLine(RenderCommands, LineStart, LineEnd, vec4(0.f, 1.f, 0.f, 1.f), 4.f);
    }
#endif
}

internal void
RenderBoundingBox(render_commands *RenderCommands, game_state *State, game_entity *Entity)
{
    // Pivot point
    DrawPoint(RenderCommands, Entity->Transform.Translation, vec4(1.f, 0.f, 0.f, 1.f), 10.f);

    bounds Bounds = GetEntityBounds(Entity);
    vec3 HalfSize = vec3(0.5f);
    vec3 BottomCenterPosition = Entity->Transform.Translation;

    if (Bounds.Min != Bounds.Max)
    {
        HalfSize = (Bounds.Max - Bounds.Min) / 2.f;
        BottomCenterPosition = Bounds.Min + vec3(HalfSize.x, 0.f, HalfSize.z);
    }

#if 0
    quat Rotation = Entity->Transform.Rotation;
#else
    quat Rotation = quat(0.f, 0.f, 0.f, 1.f);
#endif

    transform Transform = CreateTransform(BottomCenterPosition, HalfSize, Rotation);
    vec4 Color = vec4(0.f, 1.f, 0.f, 1.f);

    if (Entity->Model)
    {
        Color = vec4(0.f, 1.f, 1.f, 1.f);
    }

    if (Entity->Collider)
    {
        Color = vec4(0.f, 0.f, 1.f, 1.f);
    }

    if (Entity->Body)
    {
        Color = vec4(1.f, 0.f, 0.f, 1.f);
    }

    DrawBox(RenderCommands, Transform, Color);
}

internal void
RenderEntity(render_commands *RenderCommands, game_state *State, game_entity *Entity)
{
    if (Entity->Model->Skeleton->JointCount > 1)
    {
        if (State->Options.ShowSkeletons)
        //if (true)
        {
            DrawSkeleton(RenderCommands, State, Entity->Skinning->Pose);
        }
        else
        {
            DrawSkinnedModel(RenderCommands, Entity->Model, Entity->Skinning->Pose, Entity->Skinning);
        }
    }
    else
    {
        DrawModel(RenderCommands, Entity->Model, Entity->Transform);
    }
}

internal void
RenderEntityBatch(render_commands *RenderCommands, game_state *State, entity_render_batch *Batch)
{
    DrawModelInstanced(RenderCommands, Batch->Model, Batch->EntityCount, Batch->Instances);

    // debug drawing
    // todo: instancing
    for (u32 EntityIndex = 0; EntityIndex < Batch->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = Batch->Entities[EntityIndex];
        //render_instance *Instance = Batch->Instances + EntityIndex;

        if ((State->SelectedEntity && State->SelectedEntity->Id == Entity->Id) || State->Options.ShowBoundingVolumes)
        {
            RenderBoundingBox(RenderCommands, State, Entity);
        }
    }
}

inline void
InitRenderBatch(entity_render_batch *Batch, model *Model, u32 MaxEntityCount, memory_arena *Arena)
{
    CopyString(Model->Key, Batch->Key);
    Batch->Model = Model;
    Batch->EntityCount = 0;
    Batch->MaxEntityCount = MaxEntityCount;
    Batch->Entities = PushArray(Arena, Batch->MaxEntityCount, game_entity *);
    Batch->Instances = PushArray(Arena, Batch->MaxEntityCount, render_instance);
}

inline void
AddEntityToRenderBatch(entity_render_batch *Batch, game_entity *Entity)
{
    Assert(Batch->EntityCount < Batch->MaxEntityCount);

    game_entity **NextFreeEntity = Batch->Entities + Batch->EntityCount;
    render_instance *NextFreeInstance = Batch->Instances + Batch->EntityCount;

    *NextFreeEntity = Entity;
    NextFreeInstance->Model = Transform(Entity->Transform);
    NextFreeInstance->Color = Entity->DebugColor;

    Batch->EntityCount++;
}

inline game_entity *
CreateGameEntity(game_state *State)
{
    world_area *Area = &State->WorldArea;

    game_entity *Entity = Area->Entities + Area->EntityCount++;

    Assert(Area->EntityCount <= Area->MaxEntityCount);

    Entity->Id = GenerateEntityId(State);
    // todo:
    Entity->TestColor = vec3(1.f);
    Entity->DebugColor = vec3(1.f);

    return Entity;
}

inline void
RemoveGameEntity(game_entity *Entity)
{
    Assert(!Entity->Destroyed);
    Entity->Destroyed = true;
}

inline game_entity *
GetGameEntity(game_state *State, u32 EntityId)
{
    // todo:
    world_area *Area = &State->WorldArea;
    game_entity *Entity = Area->Entities + EntityId;
    return Entity;
}

inline void
AddModel(game_state *State, game_entity *Entity, game_assets *Assets, const char *ModelName, render_commands *RenderCommands, memory_arena *Arena)
{
    Entity->Model = GetModelAsset(Assets, ModelName);

    if (Entity->Model->Skeleton->JointCount > 1)
    {
        Entity->Skinning = PushType(Arena, skinning_data);
        InitSkinningBuffer(State, Entity->Skinning, Entity->Model, Arena, RenderCommands);

        if (Entity->Model->AnimationCount > 0)
        {
            Entity->Animation = PushType(Arena, animation_graph);
            BuildAnimationGraph(Entity->Animation, Entity->Model->AnimationGraph, Entity->Model, Entity->Id, &State->EventList, Arena);
        }
    }
}

inline void
AddBoxCollider(game_entity *Entity, vec3 Size, memory_arena *Arena)
{
    Entity->Collider = PushType(Arena, collider);
    Entity->Collider->Type = Collider_Box;
    Entity->Collider->BoxCollider.Size = Size;
    UpdateColliderPosition(Entity->Collider, Entity->Transform.Translation);
}

inline void
AddRigidBody(game_entity *Entity, b32 RootMotionEnabled, memory_arena *Arena)
{
    Entity->Body = PushType(Arena, rigid_body);
    BuildRigidBody(Entity->Body, Entity->Transform.Translation, Entity->Transform.Rotation, RootMotionEnabled);
}

inline void
AddPointLight(game_entity *Entity, vec3 Color, light_attenuation Attenuation, memory_arena *Arena)
{
    Entity->PointLight = PushType(Arena, point_light);
    Entity->PointLight->Position = Entity->Transform.Translation;
    Entity->PointLight->Color = Color;
    Entity->PointLight->Attenuation = Attenuation;
}

inline void
AddParticleEmitter(game_entity *Entity, u32 ParticleCount, vec4 Color, memory_arena *Arena)
{
    Entity->ParticleEmitter = PushType(Arena, particle_emitter);
    Entity->ParticleEmitter->ParticleCount = ParticleCount;
    Entity->ParticleEmitter->Particles = PushArray(Arena, ParticleCount, particle);
    Entity->ParticleEmitter->Color = Color;
    Entity->ParticleEmitter->NextParticleIndex = 0;
}

inline void
GameInput2BotAnimatorParams(game_state *State, game_entity *Entity, bot_animator_params *Params)
{
    game_input *Input = &State->Input;
    f32 Random = Random01(&State->GeneralEntropy);

    // todo:
    Params->MaxTime = 5.f;
    Params->Move = Clamp(Magnitude(State->CurrentMove), 0.f, 2.f);
    Params->MoveMagnitude = State->Mode == GameMode_World ? Clamp(Magnitude(Input->Move.Range), 0.f, 1.f) : 0.f;

    Params->ToStateActionIdle = Entity->Controllable;
    Params->ToStateStandingIdle = !Entity->Controllable;

    Params->ToStateActionIdle1 = Random < 0.5f;
    Params->ToStateActionIdle2 = Random >= 0.5f;
    Params->ToStateIdle1 = Random < 0.5f;
    Params->ToStateIdle2 = Random >= 0.5f;

    Params->ToStateDancing = Input->Dance.IsActivated || (State->DanceMode && !State->PrevDanceMode);
    Params->ToStateActionIdleFromDancing = Input->Dance.IsActivated || (!State->DanceMode && State->PrevDanceMode);
}

inline void
GameLogic2BotAnimatorParams(game_state *State, game_entity *Entity, bot_animator_params *Params)
{
    f32 Random = Random01(&State->GeneralEntropy);

    // todo:
    Params->MaxTime = 8.f;
    Params->ToStateActionIdle = Entity->Controllable;
    Params->ToStateStandingIdle = !Entity->Controllable;
    Params->ToStateActionIdle1 = 0.f <= Random && Random < 0.33f;
    Params->ToStateActionIdle2 = 0.33f <= Random && Random < 0.66f;

    Params->ToStateDancing = (State->DanceMode && !State->PrevDanceMode);
    Params->ToStateActionIdleFromDancing = (!State->DanceMode && State->PrevDanceMode);
}

inline void
GameInput2PaladinAnimatorParams(game_state *State, game_entity *Entity, paladin_animator_params *Params)
{
    game_input *Input = &State->Input;
    f32 Random = Random01(&State->GeneralEntropy);

    // todo:
    Params->MaxTime = 5.f;
    Params->Move = Clamp(Magnitude(State->CurrentMove), 0.f, 1.f);
    Params->MoveMagnitude = State->Mode == GameMode_World ? Clamp(Magnitude(Input->Move.Range), 0.f, 1.f) : 0.f;

    Params->ToStateActionIdle1 = 0.f <= Random && Random < 0.33f;
    Params->ToStateActionIdle2 = 0.33f <= Random && Random <= 0.66f;
    Params->ToStateActionIdle3 = 0.66f <= Random && Random <= 1.f;

    Params->ToStateDancing = Input->Dance.IsActivated || (State->DanceMode && !State->PrevDanceMode);
    Params->ToStateActionIdleFromDancing = Input->Dance.IsActivated || (!State->DanceMode && State->PrevDanceMode);

    Params->LightAttack = State->Mode == GameMode_World && Input->LightAttack.IsActivated;
    Params->StrongAttack = State->Mode == GameMode_World && Input->StrongAttack.IsActivated;
}

inline void
GameLogic2PaladinAnimatorParams(game_state *State, game_entity *Entity, paladin_animator_params *Params)
{
    f32 Random = Random01(&State->GeneralEntropy);

    // todo:
    Params->MaxTime = 8.f;
    Params->ToStateActionIdle1 = 0.f <= Random && Random < 0.33f;
    Params->ToStateActionIdle2 = 0.33f <= Random && Random < 0.66f;

    Params->ToStateDancing = (State->DanceMode && !State->PrevDanceMode);
    Params->ToStateActionIdleFromDancing = (!State->DanceMode && State->PrevDanceMode);
}

inline void
GameInput2MonstarAnimatorParams(game_state *State, game_entity *Entity, monstar_animator_params *Params)
{
    game_input *Input = &State->Input;

    Params->Move = Clamp(Magnitude(State->CurrentMove), 0.f, 1.f);
    Params->MoveMagnitude = State->Mode == GameMode_World ? Clamp(Magnitude(Input->Move.Range), 0.f, 1.f) : 0.f;
    Params->Attack = State->Mode == GameMode_World && Input->LightAttack.IsActivated;
    Params->ToStateDancing = Input->Dance.IsActivated || (State->DanceMode && !State->PrevDanceMode);
    Params->ToStateIdleFromDancing = Input->Dance.IsActivated || (!State->DanceMode && State->PrevDanceMode);
}

inline void
GameLogic2MonstarAnimatorParams(game_state *State, game_entity *Entity, monstar_animator_params *Params)
{
    Params->ToStateDancing = (State->DanceMode && !State->PrevDanceMode);
    Params->ToStateIdleFromDancing = (!State->DanceMode && State->PrevDanceMode);
}

inline void
GameInput2ClericAnimatorParams(game_state *State, game_entity *Entity, cleric_animator_params *Params)
{
    game_input *Input = &State->Input;

    Params->Move = Clamp(Magnitude(State->CurrentMove), 0.f, 1.f);
    Params->MoveMagnitude = State->Mode == GameMode_World ? Clamp(Magnitude(Input->Move.Range), 0.f, 1.f) : 0.f;
    Params->ToStateDancing = Input->Dance.IsActivated || (State->DanceMode && !State->PrevDanceMode);
    Params->ToStateIdleFromDancing = Input->Dance.IsActivated || (!State->DanceMode && State->PrevDanceMode);
}

inline void
GameLogic2ClericAnimatorParams(game_state *State, game_entity *Entity, cleric_animator_params *Params)
{
    Params->ToStateDancing = (State->DanceMode && !State->PrevDanceMode);
    Params->ToStateIdleFromDancing = (!State->DanceMode && State->PrevDanceMode);
}

internal void *
GetAnimatorParams(game_state *State, game_entity *Entity, memory_arena *Arena)
{
    void *Params = 0;

    if (StringEquals(Entity->Animation->Animator, "Bot"))
    {
        Params = PushType(Arena, bot_animator_params);

        if (Entity->Controllable)
        {
            GameInput2BotAnimatorParams(State, Entity, (bot_animator_params *) Params);
        }
        else
        {
            GameLogic2BotAnimatorParams(State, Entity, (bot_animator_params *) Params);
        }
    }
    else if (StringEquals(Entity->Animation->Animator, "Paladin"))
    {
        Params = PushType(Arena, paladin_animator_params);

        if (Entity->Controllable)
        {
            GameInput2PaladinAnimatorParams(State, Entity, (paladin_animator_params *) Params);
        }
        else
        {
            GameLogic2PaladinAnimatorParams(State, Entity, (paladin_animator_params *) Params);
        }
    }
    else if (StringEquals(Entity->Animation->Animator, "Monstar"))
    {
        Params = PushType(Arena, monstar_animator_params);

        if (Entity->Controllable)
        {
            GameInput2MonstarAnimatorParams(State, Entity, (monstar_animator_params *) Params);
        }
        else
        {
            GameLogic2MonstarAnimatorParams(State, Entity, (monstar_animator_params *) Params);
        }
    }
    else if (StringEquals(Entity->Animation->Animator, "Cleric"))
    {
        Params = PushType(Arena, cleric_animator_params);

        if (Entity->Controllable)
        {
            GameInput2ClericAnimatorParams(State, Entity, (cleric_animator_params *) Params);
        }
        else
        {
            GameLogic2ClericAnimatorParams(State, Entity, (cleric_animator_params *) Params);
        }
    }

    return Params;
}

internal void
AnimateEntity(game_state *State, game_entity *Entity, memory_arena *Arena, f32 Delta)
{
    Assert(Entity->Skinning);

    skeleton_pose *Pose = Entity->Skinning->Pose;
    joint_pose *Root = GetRootLocalJointPose(Pose);

    if (Entity->Animation)
    {
        void *Params = GetAnimatorParams(State, Entity, Arena);

        AnimatorPerFrameUpdate(&State->Animator, Entity->Animation, Params, Delta);
        AnimationGraphPerFrameUpdate(Entity->Animation, Delta);
        CalculateSkeletonPose(Entity->Animation, Pose, Arena);

        // Root Motion
        Entity->Animation->AccRootMotion.x += Pose->RootMotion.x;
        Entity->Animation->AccRootMotion.z += Pose->RootMotion.z;
    }

    Root->Translation = Entity->Transform.Translation;
    Root->Rotation = Entity->Transform.Rotation;
    Root->Scale = Entity->Transform.Scale;

    UpdateGlobalJointPoses(Pose);
    UpdateSkinningMatrices(Entity->Skinning);
}

struct update_entity_batch_job
{
    u32 StartIndex;
    u32 EndIndex;
    f32 UpdateRate;
    u32 PlayerId;
    game_entity *Entities;
    spatial_hash_grid *SpatialGrid;
    random_sequence *Entropy;
    memory_arena Arena;
};

JOB_ENTRY_POINT(UpdateEntityBatchJob)
{
    update_entity_batch_job *Data = (update_entity_batch_job *) Parameters;

    for (u32 EntityIndex = Data->StartIndex; EntityIndex < Data->EndIndex; ++EntityIndex)
    {
        game_entity *Entity = Data->Entities + EntityIndex;

        if (!Entity->Destroyed)
        {
            rigid_body *Body = Entity->Body;
            collider *Collider = Entity->Collider;

            if (Body)
            {
                if (Body->RootMotionEnabled)
                {
                    Assert(Entity->Animation);

                    vec3 ScaledRootMotion = Entity->Animation->AccRootMotion * Entity->Transform.Scale;
                    vec3 RotatedScaledRootMotion = Rotate(ScaledRootMotion, Entity->Transform.Rotation);

                    Entity->Body->PrevPosition = Entity->Body->Position;
                    Entity->Body->Position += RotatedScaledRootMotion;

                    Entity->Animation->AccRootMotion = vec3(0.f);
                }
                else
                {
                    Integrate(Body, Data->UpdateRate);
                }

                if (Collider)
                {
                    UpdateColliderPosition(Collider, Entity->Body->Position);

                    u32 MaxNearbyEntityCount = 100;
                    game_entity **NearbyEntities = PushArray(&Data->Arena, MaxNearbyEntityCount, game_entity *);
                    bounds Bounds = { vec3(-1.f), vec3(1.f) };

                    u32 NearbyEntityCount = FindNearbyEntities(Data->SpatialGrid, Entity, Bounds, NearbyEntities, MaxNearbyEntityCount);

                    for (u32 NearbyEntityIndex = 0; NearbyEntityIndex < NearbyEntityCount; ++NearbyEntityIndex)
                    {
                        game_entity *NearbyEntity = NearbyEntities[NearbyEntityIndex];
                        rigid_body *NearbyBody = NearbyEntity->Body;
                        collider *NearbyBodyCollider = NearbyEntity->Collider;

                        if (Entity->Id == Data->PlayerId)
                        {
                            //NearbyEntity->DebugColor = vec3(0.f, 1.f, 0.f);
                        }

                        if (NearbyBodyCollider)
                        {
                            // Collision detection and resolution
                            vec3 mtv;
                            if (TestColliders(Entity->Collider, NearbyEntity->Collider, &mtv))
                            {
                                Entity->Body->Position += mtv;
                                UpdateColliderPosition(Collider, Entity->Body->Position);
                            }
                        }
                    }
                }
            }

            if (Entity->ParticleEmitter)
            {
                particle_emitter *ParticleEmitter = Entity->ParticleEmitter;

                u32 ParticlesSpawn = 42;
                for (u32 ParticleSpawnIndex = 0; ParticleSpawnIndex < ParticlesSpawn; ++ParticleSpawnIndex)
                {
                    particle *Particle = ParticleEmitter->Particles + ParticleEmitter->NextParticleIndex++;

                    if (ParticleEmitter->NextParticleIndex >= ParticleEmitter->ParticleCount)
                    {
                        ParticleEmitter->NextParticleIndex = 0;
                    }

                    random_sequence *Entropy = Data->Entropy;

                    // todo: fix offset position
                    Particle->Position = Entity->Transform.Translation + vec3(0.f, 0.6f, 0.f) + vec3(
                        RandomBetween(Entropy, -0.05f, 0.05f),
                        RandomBetween(Entropy, 0.f, 0.1f),
                        RandomBetween(Entropy, -0.05f, 0.05f)
                    );
                    Particle->Velocity = vec3(
                        RandomBetween(Entropy, -0.5f, 0.5f),
                        RandomBetween(Entropy, 3.f, 4.f),
                        RandomBetween(Entropy, -0.5f, 0.5f)
                    );
                    Particle->Acceleration = vec3(0.f, -10.f, 0.f);
                    Particle->Color = ParticleEmitter->Color;
                    Particle->dColor = vec4(0.f, 0.f, 0.f, -4.0f);
                    Particle->Size = vec2(0.025f);
                    Particle->dSize = vec2(-0.05f);
                }
            }
        }
    }
}

struct animate_entity_job
{
    game_state *State;
    game_entity *Entity;
    memory_arena Arena;
    f32 Delta;
};

JOB_ENTRY_POINT(AnimateEntityJob)
{
    animate_entity_job *Data = (animate_entity_job *) Parameters;
    AnimateEntity(Data->State, Data->Entity, &Data->Arena, Data->Delta);
}

struct process_entity_batch_job
{
    u32 StartIndex;
    u32 EndIndex;
    f32 Lag;
    game_entity *Entities;
    spatial_hash_grid *SpatialGrid;

    u32 ShadowPlaneCount;
    plane *ShadowPlanes;
};

JOB_ENTRY_POINT(ProcessEntityBatchJob)
{
    process_entity_batch_job *Data = (process_entity_batch_job *) Parameters;

    for (u32 EntityIndex = Data->StartIndex; EntityIndex < Data->EndIndex; ++EntityIndex)
    {
        game_entity *Entity = Data->Entities + EntityIndex;

        if (!Entity->Destroyed)
        {
            UpdateInSpacialGrid(Data->SpatialGrid, Entity);

            if (Entity->Body)
            {
                Entity->Transform.Translation = Lerp(Entity->Body->PrevPosition, Data->Lag, Entity->Body->Position);
                Entity->Transform.Rotation = Entity->Body->Orientation;
            }

            if (Entity->Collider)
            {
                UpdateColliderPosition(Entity->Collider, Entity->Transform.Translation);
            }

            if (Entity->Model)
            {
                bounds BoundingBox = GetEntityBounds(Entity);

                // Frustrum culling
                Entity->Visible = AxisAlignedBoxVisible(Data->ShadowPlaneCount, Data->ShadowPlanes, BoundingBox);
            }
        }
    }
}

internal void
InitGameMenu(game_state *State)
{
    State->MenuQuads[0].Corner = 0;
    State->MenuQuads[0].Move = 0.f;
    State->MenuQuads[0].Color = vec4(1.f, 0.f, 0.f, 1.f);

    State->MenuQuads[1].Corner = 1;
    State->MenuQuads[1].Move = 0.f;
    State->MenuQuads[1].Color = vec4(0.f, 1.f, 0.f, 1.f);

    State->MenuQuads[2].Corner = 2;
    State->MenuQuads[2].Move = 0.f;
    State->MenuQuads[2].Color = vec4(0.f, 0.f, 1.f, 1.f);

    State->MenuQuads[3].Corner = 3;
    State->MenuQuads[3].Move = 0.f;
    State->MenuQuads[3].Color = vec4(1.f, 1.f, 0.f, 1.f);
}

inline void
LoadAnimators(animator *Animator)
{
    animator_controller *BotController = HashTableLookup(&Animator->Controllers, (char *) "Bot");
    BotController->Func = BotAnimatorController;

    animator_controller *PaladinController = HashTableLookup(&Animator->Controllers, (char *) "Paladin");
    PaladinController->Func = PaladinAnimatorController;

    animator_controller *MonstarController = HashTableLookup(&Animator->Controllers, (char *) "Monstar");
    MonstarController->Func = MonstarAnimatorController;

    animator_controller *ClericController = HashTableLookup(&Animator->Controllers, (char *) "Cleric");
    ClericController->Func = ClericAnimatorController;

    animator_controller *SimpleController = HashTableLookup(&Animator->Controllers, (char *) "Simple");
    SimpleController->Func = SimpleAnimatorController;
}

inline void
Entity2Spec(game_entity *Entity, game_entity_spec *Spec)
{
    Spec->Transform = Entity->Transform;

    if (Entity->Model)
    {
        Model2Spec(Entity->Model, &Spec->ModelSpec);
    }

    if (Entity->Collider)
    {
        Collider2Spec(Entity->Collider, &Spec->ColliderSpec);
    }

    if (Entity->Body)
    {
        RigidBody2Spec(Entity->Body, &Spec->RigidBodySpec);
    }

    if (Entity->PointLight)
    {
        PointLight2Spec(Entity->PointLight, &Spec->PointLightSpec);
    }

    if (Entity->ParticleEmitter)
    {
        ParticleEmitter2Spec(Entity->ParticleEmitter, &Spec->ParticleEmitterSpec);
    }
}

internal void
Spec2Entity(game_entity_spec *Spec, game_entity *Entity, game_state *State, render_commands *RenderCommands, memory_arena *Arena)
{
    Entity->Transform = Spec->Transform;

    if (Spec->ModelSpec.Has)
    {
        model *Model = GetModelAsset(&State->Assets, Spec->ModelSpec.ModelRef);
        AddModel(State, Entity, &State->Assets, Model->Key, RenderCommands, Arena);
    }

    if (Spec->ColliderSpec.Has)
    {
        collider_spec ColliderSpec = Spec->ColliderSpec;

        switch (ColliderSpec.Type)
        {
            case Collider_Box:
            {
                AddBoxCollider(Entity, ColliderSpec.Size, Arena);
                break;
            }
            default:
            {
                Assert(!"Not implemented");
            }
        }
    }

    if (Spec->RigidBodySpec.Has)
    {
        rigid_body_spec RigidBodySpec = Spec->RigidBodySpec;
        AddRigidBody(Entity, RigidBodySpec.RootMotionEnabled, Arena);
    }

    if (Spec->PointLightSpec.Has)
    {
        point_light_spec PointLightSpec = Spec->PointLightSpec;
        AddPointLight(Entity, PointLightSpec.Color, PointLightSpec.Attenuation, Arena);
    }

    if (Spec->ParticleEmitterSpec.Has)
    {
        particle_emitter_spec ParticleEmitterSpec = Spec->ParticleEmitterSpec;
        AddParticleEmitter(Entity, ParticleEmitterSpec.ParticleCount, ParticleEmitterSpec.Color, Arena);
    }
}

internal void
SaveArea(game_state *State, char *FileName,  platform_api *Platform, memory_arena *Arena)
{
    u32 HeaderSize = sizeof(u32);
    u32 BufferSize = HeaderSize + State->ActiveEntitiesCount * sizeof(game_entity_spec);
    void *Buffer = PushSize(Arena, BufferSize);

    u32 *EntityCount = (u32 *) ((u8 *) Buffer + 0);
    *EntityCount = State->ActiveEntitiesCount;

    game_entity_spec *Specs = (game_entity_spec *) ((u8 *) Buffer + HeaderSize);

    world_area *Area = &State->WorldArea;

    u32 SpecIndex = 0;
    for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = Area->Entities + EntityIndex;

        if (!Entity->Destroyed)
        {
            game_entity_spec *Spec = Specs + SpecIndex++;
            Entity2Spec(Entity, Spec);
        }

        Assert(SpecIndex <= State->ActiveEntitiesCount);
    }

    Platform->WriteFile(FileName, Buffer, BufferSize);
}

internal void
LoadWorldArea(game_state *State, char *FileName, platform_api *Platform, render_commands *RenderCommands, memory_arena *TempArena)
{
    read_file_result Result = Platform->ReadFile(FileName, TempArena, ReadBinary());

    world_area *Area = &State->WorldArea;

    u32 *EntityCount = (u32 *) Result.Contents;
    game_entity_spec *Specs = (game_entity_spec *) ((u8 *) Result.Contents + sizeof(u32));

    bounds WorldBounds = { vec3(-300.f, 0.f, -300.f), vec3(300.f, 20.f, 300.f) };
    vec3 CellSize = vec3(5.f);
    InitSpatialHashGrid(&Area->SpatialGrid, WorldBounds, CellSize, &Area->Arena);

    Area->MaxEntityCount = 10000;
    Area->EntityCount = 0;
    Area->Entities = PushArray(&Area->Arena, Area->MaxEntityCount, game_entity);

    for (u32 EntityIndex = 0; EntityIndex < *EntityCount; ++EntityIndex)
    {
        game_entity_spec *Spec = Specs + EntityIndex;

        game_entity *Entity = CreateGameEntity(State);

        Spec2Entity(Spec, Entity, State, RenderCommands, &Area->Arena);

        AddToSpacialGrid(&Area->SpatialGrid, Entity);

        // todo(continue): properly add player
        if (Entity->Body)
        {
            State->Player = Entity;
            State->Player->Controllable = true;
        }
    }
}

internal void
ClearWorldArea(game_state *State)
{
    State->NextFreeEntityId = 0;

    State->WorldArea.EntityCount = 0;
    ClearMemoryArena(&State->WorldArea.Arena);

    State->SelectedEntity = 0;
}

DLLExport GAME_INIT(GameInit)
{
    PROFILE(Memory->Profiler, "GameInit");

    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;

    umm PermanentArenaSize = Memory->PermanentStorageSize - sizeof(game_state);
    u8 *PermanentArenaBase = (u8 *) Memory->PermanentStorage + sizeof(game_state);
    InitMemoryArena(&State->PermanentArena, PermanentArenaBase, PermanentArenaSize);

    umm TransientArenaSize = Memory->TransientStorageSize;
    u8 *TransientArenaBase = (u8 *) Memory->TransientStorage;
    InitMemoryArena(&State->TransientArena, TransientArenaBase, TransientArenaSize);

    State->WorldArea = {};
    State->WorldArea.Arena = SubMemoryArena(&State->PermanentArena, Megabytes(32));
    State->WorldArea.EntityCount = 0;
    State->WorldArea.MaxEntityCount = 10000;
    State->WorldArea.Entities = PushArray(&State->WorldArea.Arena, State->WorldArea.MaxEntityCount, game_entity);

    bounds WorldBounds = { vec3(-300.f, 0.f, -300.f), vec3(300.f, 20.f, 300.f) };
    vec3 CellSize = vec3(5.f);
    InitSpatialHashGrid(&State->WorldArea.SpatialGrid, WorldBounds, CellSize, &State->WorldArea.Arena);

    State->JobQueue = Memory->JobQueue;

    State->NextFreeEntityId = 1;
    State->NextFreeMeshId = 1;
    State->NextFreeTextureId = 1;
    State->NextFreeSkinningId = 1;

    game_process *Sentinel = &State->ProcessSentinel;
    CopyString("Sentinel", Sentinel->Key);
    Sentinel->Next = Sentinel->Prev = Sentinel;

    State->Processes.Count = 31;
    State->Processes.Values = PushArray(&State->PermanentArena, State->Processes.Count, game_process);

    // Event System
    game_event_list *EventList = &State->EventList;
    EventList->MaxEventCount = 4096;
    EventList->EventCount = 0;
    EventList->Events = PushArray(&State->PermanentArena, EventList->MaxEventCount, game_event);
    EventList->Platform = Platform;
    EventList->Arena = SubMemoryArena(&State->PermanentArena, Megabytes(4));

    // Animator Setup
    State->Animator = {};
    State->Animator.Controllers.Count = 31;
    State->Animator.Controllers.Values = PushArray(&State->PermanentArena, State->Animator.Controllers.Count, animator_controller);

    LoadAnimators(&State->Animator);
    //

    State->DelayTime = 0.f;
    State->DelayDuration = 0.5f;

    State->Mode = GameMode_Editor;

    Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);

    f32 AspectRatio = (f32) Parameters->WindowWidth / (f32) Parameters->WindowHeight;
    f32 FieldOfView = RADIANS(45.f);
    InitCamera(&State->EditorCamera, FieldOfView, AspectRatio, 0.1f, 1000.f, vec3(0.f, 4.f, 8.f), RADIANS(-20.f), RADIANS(-90.f));
    InitCamera(&State->GameCamera, FieldOfView, AspectRatio, 0.1f, 320.f, vec3(0.f, 0.f, 0.f), RADIANS(20.f), RADIANS(0.f));
    // todo:
    State->GameCamera.Radius = 4.f;

    State->Ground = ComputePlane(vec3(-1.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f), vec3(1.f, 0.f, 0.f));
    State->BackgroundColor = vec3(0.f, 0.f, 0.f);
    State->DirectionalLight = {};
    State->DirectionalLight.Color = vec3(1.f);
    State->DirectionalLight.Direction = Normalize(vec3(0.4f, -0.8f, -0.4f));

    State->GeneralEntropy = RandomSequence(451);
    State->ParticleEntropy = RandomSequence(217);

    ClearRenderCommands(Memory);
    render_commands *RenderCommands = GetRenderCommands(Memory);

    ClearAudioCommands(Memory);

    State->CurrentMove = vec2(0.f);
    State->TargetMove = vec2(0.f);

    State->Options = {};
    State->Options.ShowBoundingVolumes = false;
    State->Options.ShowGrid = true;

    InitGameMenu(State);

    State->Assets = {};
    State->Assets.Arena = SubMemoryArena(&State->PermanentArena, Megabytes(256));

    LoadFontAssets(&State->Assets, Platform);
    InitGameFontAssets(State, &State->Assets, RenderCommands);

#if 0
    LoadGameAssets(&State->Assets, Platform);
#else
    // todo: should not be a PermanentArena
    load_game_assets_job *JobParams = PushType(&State->PermanentArena, load_game_assets_job);
    JobParams->Assets = &State->Assets;
    JobParams->Platform = Platform;

    job Job = {};
    Job.EntryPoint = LoadGameAssetsJob;
    Job.Parameters = JobParams;

    Platform->KickJob(State->JobQueue, Job);
#endif

    /*State->Player = State->Dummy;
    State->Player->Controllable = true;*/

    State->MasterVolume = 0.f;
}

DLLExport GAME_RELOAD(GameReload)
{
    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;

    // Restarting game processes
    game_process *GameProcess = State->ProcessSentinel.Next;

    while (GameProcess->OnUpdatePerFrame)
    {
        char ProcessName[256];
        CopyString(GameProcess->Key, ProcessName);

        game_process_on_update *OnUpdatePerFrame = (game_process_on_update *) Platform->LoadFunction(Platform->PlatformHandle, ProcessName);

        EndGameProcess(State, ProcessName);
        StartGameProcess_(State, ProcessName, OnUpdatePerFrame);

        GameProcess = GameProcess->Next;
    }

    // Reloading animators
    LoadAnimators(&State->Animator);
}

DLLExport GAME_PROCESS_INPUT(GameProcessInput)
{
    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;
    audio_commands *AudioCommands = GetAudioCommands(Memory);

    State->Input = *Input;

    vec3 xAxis = vec3(1.f, 0.f, 0.f);
    vec3 yAxis = vec3(0.f, 1.f, 0.f);
    vec3 zAxis = vec3(0.f, 0.f, 1.f);

    f32 Lag = Parameters->UpdateLag / Parameters->UpdateRate;
    Assert(0.f <= Lag && Lag <= 1.f);

    vec2 Move = Input->Move.Range;

    if (Abs(Move.x) == 1.f && Abs(Move.y) == 1.f)
    {
        Move = Normalize(Move);
    }
    else if (Abs(Move.x) == 2.f && Abs(Move.y) == 2.f)
    {
        Move = Normalize(Move) * 2.f;
    }

    if (Input->Menu.IsActivated)
    {
        if (State->Mode == GameMode_Menu)
        {
            State->Mode = State->PrevMode;
        }
        else if (State->Mode == GameMode_World || State->Mode == GameMode_Editor)
        {
            State->PrevMode = State->Mode;
            State->Mode = GameMode_Menu;
            InitGameMenu(State);
        }
    }

    /*if (Input->ChoosePrevHero.IsActivated)
    {
        State->Player->Controllable = false;
        State->Player->Body->Acceleration = vec3(0.f);

        State->Player = GetPrevHero(State);

        State->Player->Controllable = true;
    }

    if (Input->ChooseNextHero.IsActivated)
    {
        State->Player->Controllable = false;
        State->Player->Body->Acceleration = vec3(0.f);

        State->Player = GetNextHero(State);

        State->Player->Controllable = true;
    }*/

    if (Input->Reset.IsActivated)
    {
        State->Player->Controllable = false;

        // ...

        State->Player->Controllable = true;
    }

    if (State->Mode == GameMode_Editor && Input->LeftClick.IsActivated)
    {
        ray Ray = ScreenPointToWorldRay(Input->MouseCoords, vec2((f32) Parameters->WindowWidth, (f32) Parameters->WindowHeight), &State->EditorCamera);

        f32 MinDistance = F32_MAX;

        game_entity *SelectedEntity = 0;

        world_area *Area = &State->WorldArea;

        for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
        {
            game_entity *Entity = Area->Entities + EntityIndex;

            if (!Entity->Destroyed && Entity->Model)
            {
                State->SelectedEntity = 0;
                bounds Box = GetEntityBounds(Entity);

                vec3 IntersectionPoint;
                if (IntersectRayAABB(Ray, Box, IntersectionPoint))
                {
                    f32 Distance = Magnitude(IntersectionPoint - State->EditorCamera.Transform.Translation);

                    if (Distance < MinDistance)
                    {
                        MinDistance = Distance;
                        SelectedEntity = Entity;
                    }
                }
            }
        }

        if (SelectedEntity)
        {
            State->SelectedEntity = SelectedEntity;
        }
    }

    if (Input->EditMode.IsActivated)
    {
        if (State->Mode == GameMode_World || State->Mode == GameMode_Menu)
        {
            State->Mode = GameMode_Editor;
            Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Cursor);
        }
        else
        {
            State->Mode = GameMode_World;
            Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);
        }
    }

#if 0
    State->IsBackgroundHighlighted = Input->HighlightBackground.IsActive;
#endif

    switch (State->Mode)
    {
        case GameMode_World:
        {
            // Camera
            // https://www.gamasutra.com/blogs/YoannPignole/20150928/249412/Third_person_camera_design_with_free_move_zone.php
            game_camera *PlayerCamera = &State->GameCamera;

            PlayerCamera->Pitch -= Input->Camera.Range.y;
            PlayerCamera->Pitch = Clamp(PlayerCamera->Pitch, RADIANS(0.f), RADIANS(89.f));

            PlayerCamera->Yaw += Input->Camera.Range.x;
            PlayerCamera->Yaw = Mod(PlayerCamera->Yaw, 2 * PI);

            PlayerCamera->Radius -= Input->ZoomDelta;
            PlayerCamera->Radius = Clamp(PlayerCamera->Radius, 4.f, 16.f);

            f32 CameraHeight = Max(0.1f, PlayerCamera->Radius * Sin(PlayerCamera->Pitch));

            vec3 PlayerPosition = State->Player->Transform.Translation;

            if (PlayerPosition != PlayerCamera->PivotPosition)
            {
                f32 Distance = Magnitude(PlayerPosition - PlayerCamera->PivotPosition);
                f32 Duration = LogisticFunction(0.8f, 0.5f, 5.f, Distance);

                SetVec3Lerp(&PlayerCamera->PivotPositionLerp, 0.f, Duration, PlayerCamera->PivotPosition, PlayerPosition);
                StartGameProcess(State, CameraPivotPositionLerpProcess);
            }

            PlayerCamera->Transform.Translation.x = PlayerCamera->PivotPosition.x +
                Sqrt(Square(PlayerCamera->Radius) - Square(CameraHeight)) * Sin(PlayerCamera->Yaw);
            PlayerCamera->Transform.Translation.y = PlayerCamera->PivotPosition.y + CameraHeight;
            PlayerCamera->Transform.Translation.z = PlayerCamera->PivotPosition.z -
                Sqrt(Square(PlayerCamera->Radius) - Square(CameraHeight)) * Cos(PlayerCamera->Yaw);

            PlayerCamera->Transform.Rotation = Euler2Quat(PlayerCamera->Yaw, PlayerCamera->Pitch, 0.f);

            // todo:
            vec3 CameraLookAtPoint = PlayerCamera->PivotPosition + vec3(0.f, 1.2f, 0.f);
            PlayerCamera->Direction = Normalize(CameraLookAtPoint - State->GameCamera.Transform.Translation);

            game_entity *Player = State->Player;

            // todo:
            if (Player->Controllable && Player->Body)
            {
                vec3 yMoveAxis = Normalize(Projection(State->GameCamera.Direction, State->Ground));
                vec3 xMoveAxis = Normalize(Orthogonal(yMoveAxis, State->Ground));

                f32 xMoveY = Dot(yMoveAxis, xAxis) * Move.y;
                f32 zMoveY = Dot(yMoveAxis, zAxis) * Move.y;

                f32 xMoveX = Dot(xMoveAxis, xAxis) * Move.x;
                f32 zMoveX = Dot(xMoveAxis, zAxis) * Move.x;

                vec3 PlayerDirection = vec3(Dot(vec3(Move.x, 0.f, Move.y), xMoveAxis), 0.f, Dot(vec3(Move.x, 0.f, Move.y), yMoveAxis));

                State->TargetMove = Move;

                if (State->TargetMove != State->CurrentMove)
                {
                    StartGameProcess(State, PlayerMoveLerpProcess);
                }

                Player->Body->Acceleration.x = (xMoveX + xMoveY) * 20.f;
                Player->Body->Acceleration.z = (zMoveX + zMoveY) * 20.f;

                quat PlayerOrientation = AxisAngle2Quat(vec4(yAxis, Atan2(PlayerDirection.x, PlayerDirection.z)));

                f32 MoveMaginute = Clamp(Magnitude(Move), 0.f, 1.f);

                if (MoveMaginute > 0.f)
                {
                    SetQuatLerp(&Player->Body->OrientationLerp, 0.f, 0.2f, Player->Body->Orientation, PlayerOrientation);

                    // todo: make helper function to generate names for game processes
                    //AttachChildGameProcess(State, Stringify(PlayerOrientationLerpProcess), "PlayerOrientationLerpProcess_DelayProcess", DelayProcess);
                    //AttachChildGameProcess(State, "PlayerOrientationLerpProcess_DelayProcess", "PlayerOrientationLerpProcess_DelayProcess_ChangeBackgroundProcess", ChangeBackgroundProcess);
                    StartGameProcess(State, PlayerOrientationLerpProcess);
                }
                else
                {
                    EndGameProcess(State, Stringify(PlayerOrientationLerpProcess));
                }
            }

            break;
        }
        case GameMode_Editor:
        {
            game_camera *FreeCamera = &State->EditorCamera;

            f32 FreeCameraSpeed = 10.f;

            if (Input->EnableFreeCameraMovement.IsActive)
            {
                Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);

                FreeCamera->Pitch += Input->Camera.Range.y;
                FreeCamera->Pitch = Clamp(FreeCamera->Pitch, RADIANS(-89.f), RADIANS(89.f));

                FreeCamera->Yaw += Input->Camera.Range.x;
                FreeCamera->Yaw = Mod(FreeCamera->Yaw, 2 * PI);

                FreeCamera->Direction = Euler2Direction(FreeCamera->Yaw, FreeCamera->Pitch);
                FreeCamera->Transform.Rotation = Euler2Quat(FreeCamera->Yaw, FreeCamera->Pitch, 0.f);
            }
            else
            {
                Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Cursor);
            }

            FreeCamera->Transform.Translation += (
                Move.x * (Normalize(Cross(FreeCamera->Direction, FreeCamera->Up))) +
                Move.y * FreeCamera->Direction) * FreeCameraSpeed * Parameters->Delta;

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

DLLExport GAME_UPDATE(GameUpdate)
{
    //PROFILE(Memory->Profiler, "GameUpdate");

    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;
    world_area *Area = &State->WorldArea;

    scoped_memory ScopedMemory(&State->TransientArena);

#if 1
    for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = Area->Entities + EntityIndex;

        if (!Entity->Destroyed)
        {
            Entity->DebugColor = Entity->TestColor;

#if 0
            if (Entity->Body)
            {
                rigid_body *Body = Entity->Body;
                Body->Acceleration = vec3(RandomBetween(&State->Entropy, -500.f, 500.f), 0.f, RandomBetween(&State->Entropy, -500.f, 500.f));
            }
#endif
        }
    }
#endif

#if 1
    u32 EntityBatchCount = 100;
    u32 UpdateEntityBatchJobCount = Ceil((f32) Area->EntityCount / (f32) EntityBatchCount);
    job *UpdateEntityBatchJobs = PushArray(ScopedMemory.Arena, UpdateEntityBatchJobCount, job);
    update_entity_batch_job *UpdateEntityBatchJobParams = PushArray(ScopedMemory.Arena, UpdateEntityBatchJobCount, update_entity_batch_job);

    for (u32 EntityBatchIndex = 0; EntityBatchIndex < UpdateEntityBatchJobCount; ++EntityBatchIndex)
    {
        job *Job = UpdateEntityBatchJobs + EntityBatchIndex;
        update_entity_batch_job *JobData = UpdateEntityBatchJobParams + EntityBatchIndex;

        JobData->StartIndex = EntityBatchIndex * EntityBatchCount;
        JobData->EndIndex = Min((i32) JobData->StartIndex + EntityBatchCount, (i32) Area->EntityCount);
        JobData->UpdateRate = Parameters->UpdateRate;
        if (State->Player)
        {
            JobData->PlayerId = State->Player->Id;
        }
        JobData->Entities = Area->Entities;
        JobData->SpatialGrid = &Area->SpatialGrid;
        JobData->Entropy = &State->ParticleEntropy;
        JobData->Arena = SubMemoryArena(ScopedMemory.Arena, Megabytes(1), NoClear());

        Job->EntryPoint = UpdateEntityBatchJob;
        Job->Parameters = JobData;
    }

    Platform->KickJobsAndWait(State->JobQueue, UpdateEntityBatchJobCount, UpdateEntityBatchJobs);
#else
    for (u32 EntityIndex = 0; EntityIndex < State->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = State->Entities + EntityIndex;
        rigid_body *Body = Entity->Body;

        if (Body)
        {
            if (Body->RootMotionEnabled)
            {
                Assert(Entity->Animation);

                vec3 ScaledRootMotion = Entity->Animation->AccRootMotion * Entity->Transform.Scale;
                vec3 RotatedScaledRootMotion = Rotate(ScaledRootMotion, Entity->Transform.Rotation);

                Entity->Body->PrevPosition = Entity->Body->Position;
                Entity->Body->Position += RotatedScaledRootMotion;

                Entity->Animation->AccRootMotion = vec3(0.f);
            }
            else
            {
                Integrate(Body, Parameters->UpdateRate);
            }

            u32 MaxNearbyEntityCount = 100;
            game_entity **NearbyEntities = PushArray(ScopedMemory.Arena, MaxNearbyEntityCount, game_entity *);
            aabb Bounds = { vec3(-5.f), vec3(5.f) };

            u32 NearbyEntityCount = FindNearbyEntities(&State->SpatialGrid, Entity, Bounds, NearbyEntities, MaxNearbyEntityCount);

            for (u32 NearbyEntityIndex = 0; NearbyEntityIndex < NearbyEntityCount; ++NearbyEntityIndex)
            {
                game_entity *NearbyEntity = NearbyEntities[NearbyEntityIndex];
                rigid_body *NearbyBody = NearbyEntity->Body;

                if (Entity->Id == State->Player->Id || Entity->DebugView)
                {
                    NearbyEntity->DebugColor = vec3(0.f, 1.f, 0.f);
                }

                if (NearbyBody)
                {
                    // Collision detection and resolution
                    vec3 mtv;
                    if (TestAABBAABB(GetRigidBodyAABB(Entity->Body), GetRigidBodyAABB(NearbyBody), &mtv))
                    {
                        Entity->Body->Position += mtv;
                    }
                }
            }
        }
    }
#endif
}

DLLExport GAME_RENDER(GameRender)
{
    game_state *State = GetGameState(Memory);
    render_commands *RenderCommands = GetRenderCommands(Memory);
    audio_commands *AudioCommands = GetAudioCommands(Memory);
    platform_api *Platform = Memory->Platform;
    world_area *Area = &State->WorldArea;

    f32 ScreenWidthInUnits = 20.f;
    f32 PixelsPerUnit = (f32) Parameters->WindowWidth / ScreenWidthInUnits;
    f32 ScreenHeightInUnits = (f32) Parameters->WindowHeight / PixelsPerUnit;
    f32 Left = -ScreenWidthInUnits / 2.f;
    f32 Right = ScreenWidthInUnits / 2.f;
    f32 Bottom = -ScreenHeightInUnits / 2.f;
    f32 Top = ScreenHeightInUnits / 2.f;
    f32 Near = -10.f;
    f32 Far = 10.f;
    f32 Lag = Parameters->UpdateLag / Parameters->UpdateRate;

    RenderCommands->Settings.WindowWidth = Parameters->WindowWidth;
    RenderCommands->Settings.WindowHeight = Parameters->WindowHeight;
    RenderCommands->Settings.Time = Parameters->Time;
    RenderCommands->Settings.PixelsPerUnit = PixelsPerUnit;
    RenderCommands->Settings.UnitsPerPixel = 1.f / PixelsPerUnit;

    AudioCommands->Settings.Volume = State->MasterVolume;

    if (State->Assets.State == GameAssetsState_Loaded)
    {
        // todo: render commands buffer is not multithread-safe!
        InitGameModelAssets(State, &State->Assets, RenderCommands);
        InitGameAudioClipAssets(&State->Assets);

        //State->Player = State->PlayableEntities[State->PlayableEntityIndex];
        //State->Player = State->Entities + 1;
        //State->Player->Controllable = true;

        State->Assets.State = GameAssetsState_Ready;

        Play2D(AudioCommands, GetAudioClipAsset(&State->Assets, "Ambient 5"), SetAudioPlayOptions(0.1f, true), 2);
    }

    // todo:
    if (State->DanceMode && !State->PrevDanceMode)
    {
        Pause(AudioCommands, 2);
        Play2D(AudioCommands, GetAudioClipAsset(&State->Assets, "samba"), SetAudioPlayOptions(0.75f, true), 3);
    }

    if (!State->DanceMode && State->PrevDanceMode)
    {
        Stop(AudioCommands, 3);
        Resume(AudioCommands, 2);
    }
    //

    SetTime(RenderCommands, Parameters->Time);
    SetViewport(RenderCommands, 0, 0, Parameters->WindowWidth, Parameters->WindowHeight);
    SetScreenProjection(RenderCommands, Left, Right, Bottom, Top, Near, Far);

    switch (State->Mode)
    {
        case GameMode_World:
        case GameMode_Editor:
        {
            game_camera *Camera = State->Mode == GameMode_World ? &State->GameCamera : &State->EditorCamera;
            b32 EnableFrustrumCulling = State->Mode == GameMode_World;

            SetListener(AudioCommands, Camera->Transform.Translation, -Camera->Direction);

            mat4 WorldToCamera = GetCameraTransform(Camera);

            RenderCommands->Settings.EnableCascadedShadowMaps = true;
            RenderCommands->Settings.ShowCascades = State->Options.ShowCascades;
            RenderCommands->Settings.Camera = Camera;
            RenderCommands->Settings.WorldToCamera = GetCameraTransform(Camera);
            RenderCommands->Settings.CameraToWorld = Inverse(WorldToCamera);
            RenderCommands->Settings.DirectionalLight = &State->DirectionalLight;

            if (State->IsBackgroundHighlighted)
            {
                Clear(RenderCommands, vec4(1.f, 0.f, 1., 1.f));
            }
            else
            {
                Clear(RenderCommands, vec4(State->BackgroundColor, 1.f));
            }

            game_process *GameProcess = State->ProcessSentinel.Next;

            while (GameProcess->OnUpdatePerFrame)
            {
                GameProcess->OnUpdatePerFrame(State, GameProcess, Parameters->Delta);
                GameProcess = GameProcess->Next;
            }

            SetWorldProjection(RenderCommands, Camera->FieldOfView, Camera->AspectRatio, Camera->NearClipPlane, Camera->FarClipPlane);
            SetCamera(RenderCommands, Camera->Transform.Translation, Camera->Direction, Camera->Up);

            SetDirectionalLight(RenderCommands, State->DirectionalLight);

            u32 ShadowPlaneCount = 0;
            plane *ShadowPlanes = 0;

            {
                PROFILE(Memory->Profiler, "GameRender:BuildVisibilityRegion");

                BuildFrustrumPolyhedron(&State->GameCamera, &State->Frustrum);

                polyhedron VisibilityRegion = State->Frustrum;

                // Camera is looking downwards
                if (State->GameCamera.Direction.y < 0.f)
                {
                    f32 MinY = 1.f;
                    plane LowestPlane = ComputePlane(vec3(-1.f, MinY, 0.f), vec3(0.f, MinY, 1.f), vec3(1.f, MinY, 0.f));

                    ClipPolyhedron(&State->Frustrum, LowestPlane, &VisibilityRegion);
                }

                vec4 LightPosition = vec4(-State->DirectionalLight.Direction * 100.f, 0.f);

                ShadowPlanes = PushArray(&State->TransientArena, MaxPolyhedronFaceCount, plane);
                ShadowPlaneCount = CalculateShadowRegion(&VisibilityRegion, LightPosition, ShadowPlanes);

                if (State->Options.ShowCamera)
                {
                    RenderFrustrum(RenderCommands, &VisibilityRegion);

                    // Render camera axes
                    game_camera *Camera = &State->GameCamera;
                    mat4 CameraTransform = GetCameraTransform(Camera);

                    vec3 xAxis = CameraTransform[0].xyz;
                    vec3 yAxis = CameraTransform[1].xyz;
                    vec3 zAxis = CameraTransform[2].xyz;

                    vec3 Origin = Camera->Transform.Translation;
                    f32 AxisLength = 3.f;

                    DrawLine(RenderCommands, Origin, Origin + xAxis * AxisLength, vec4(1.f, 0.f, 0.f, 1.f), 4.f);
                    DrawLine(RenderCommands, Origin, Origin + yAxis * AxisLength, vec4(0.f, 1.f, 0.f, 1.f), 4.f);
                    DrawLine(RenderCommands, Origin, Origin + zAxis * AxisLength, vec4(0.f, 0.f, 1.f, 1.f), 4.f);
                }
            }

            {
                PROFILE(Memory->Profiler, "GameRender:ProcessEntities");

                scoped_memory ScopedMemory(&State->TransientArena);

                u32 EntityBatchCount = 100;
                u32 ProcessEntityBatchJobCount = Ceil((f32) Area->EntityCount / (f32) EntityBatchCount);
                job *ProcessEntityBatchJobs = PushArray(&State->TransientArena, ProcessEntityBatchJobCount, job);
                process_entity_batch_job *ProcessEntityBatchJobParams = PushArray(ScopedMemory.Arena, ProcessEntityBatchJobCount, process_entity_batch_job);

                for (u32 EntityBatchIndex = 0; EntityBatchIndex < ProcessEntityBatchJobCount; ++EntityBatchIndex)
                {
                    job *Job = ProcessEntityBatchJobs + EntityBatchIndex;
                    process_entity_batch_job *JobData = ProcessEntityBatchJobParams + EntityBatchIndex;

                    JobData->StartIndex = EntityBatchIndex * EntityBatchCount;
                    JobData->EndIndex = Min((i32) JobData->StartIndex + EntityBatchCount, (i32) Area->EntityCount);
                    JobData->Lag = Lag;
                    JobData->Entities = Area->Entities;
                    JobData->SpatialGrid = &Area->SpatialGrid;
                    JobData->ShadowPlaneCount = ShadowPlaneCount;
                    JobData->ShadowPlanes = ShadowPlanes;

                    Job->EntryPoint = ProcessEntityBatchJob;
                    Job->Parameters = JobData;
                }

                Platform->KickJobsAndWait(State->JobQueue, ProcessEntityBatchJobCount, ProcessEntityBatchJobs);
            }

            {
                PROFILE(Memory->Profiler, "GameRender:AnimateEntities");

                scoped_memory ScopedMemory(&State->TransientArena);

                u32 AnimationJobCount = 0;
                u32 MaxAnimationJobCount = Area->EntityCount;
                job *AnimationJobs = PushArray(ScopedMemory.Arena, MaxAnimationJobCount, job);
                animate_entity_job *AnimationJobParams = PushArray(ScopedMemory.Arena, MaxAnimationJobCount, animate_entity_job);

                for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
                {
                    game_entity *Entity = Area->Entities + EntityIndex;

                    if (!Entity->Destroyed && Entity->Model && Entity->Model->Skeleton->JointCount > 1)
                    {
                        job *Job = AnimationJobs + AnimationJobCount;
                        animate_entity_job *JobData = AnimationJobParams + AnimationJobCount;

                        ++AnimationJobCount;

                        JobData->State = State;
                        JobData->Entity = Entity;
                        JobData->Arena = SubMemoryArena(ScopedMemory.Arena, Megabytes(1), NoClear());
                        JobData->Delta = Parameters->Delta;

                        Job->EntryPoint = AnimateEntityJob;
                        Job->Parameters = JobData;
                    }
                }

                Platform->KickJobsAndWait(State->JobQueue, AnimationJobCount, AnimationJobs);
            }

            {
                PROFILE(Memory->Profiler, "GameRender:PrepareRenderBuffer");

                u32 MaxPointLightCount = 32;
                u32 PointLightCount = 0;
                point_light *PointLights = PushArray(&State->TransientArena, MaxPointLightCount, point_light);

                State->EntityBatches.Count = 32;
                State->EntityBatches.Values = PushArray(&State->TransientArena, State->EntityBatches.Count, entity_render_batch);
                State->RenderableEntityCount = 0;
                State->ActiveEntitiesCount = 0;

                for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
                {
                    game_entity *Entity = Area->Entities + EntityIndex;

                    if (!Entity->Destroyed)
                    {
                        ++State->ActiveEntitiesCount;

                        if (Entity->Model)
                        {
                            if (!EnableFrustrumCulling || Entity->Visible)
                            {
                                // Grouping entities into render batches
                                entity_render_batch *Batch = GetRenderBatch(State, Entity->Model->Key);

                                if (IsRenderBatchEmpty(Batch))
                                {
                                    InitRenderBatch(Batch, Entity->Model, Area->MaxEntityCount, &State->TransientArena);
                                }

                                AddEntityToRenderBatch(Batch, Entity);

                                ++State->RenderableEntityCount;
                            }
                        }

                        if ((State->SelectedEntity && State->SelectedEntity->Id == Entity->Id) || State->Options.ShowBoundingVolumes)
                        {
                            RenderBoundingBox(RenderCommands, State, Entity);
                        }

                        if (Entity->PointLight)
                        {
                            point_light *PointLight = PointLights + PointLightCount++;

                            Assert(PointLightCount <= MaxPointLightCount);

                            // todo: fix offset position
                            PointLight->Position = Entity->Transform.Translation + vec3(0.f, 0.6f, 0.f);
                            PointLight->Color = Entity->PointLight->Color;
                            PointLight->Attenuation = Entity->PointLight->Attenuation;
                        }

                        if (Entity->ParticleEmitter)
                        {
                            particle_emitter *ParticleEmitter = Entity->ParticleEmitter;

                            for (u32 ParticleIndex = 0; ParticleIndex < ParticleEmitter->ParticleCount; ++ParticleIndex)
                            {
                                particle *Particle = ParticleEmitter->Particles + ParticleIndex;

                                f32 dt = Parameters->Delta;

                                Particle->Position += 0.5f * Particle->Acceleration * Square(dt) + Particle->Velocity * dt;
                                Particle->Velocity += Particle->Acceleration * dt;
                                Particle->Color = Particle->Color + Particle->dColor * dt;
                                Particle->Size = Particle->Size + Particle->dSize * dt;
                            }

                            DrawParticles(RenderCommands, ParticleEmitter->ParticleCount, ParticleEmitter->Particles);
                        }
                    }
                }

                // todo: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
                SetPointLights(RenderCommands, PointLightCount, PointLights);
            }

            {
                PROFILE(Memory->Profiler, "GameRender:PushRenderBuffer");

                // Pushing entities into render buffer
                for (u32 EntityBatchIndex = 0; EntityBatchIndex < State->EntityBatches.Count; ++EntityBatchIndex)
                {
                    entity_render_batch *Batch = State->EntityBatches.Values + EntityBatchIndex;

                    u32 BatchThreshold = 1;

                    // todo: skinning!
                    if (Batch->EntityCount > BatchThreshold && Batch->Model->Skeleton->JointCount == 1)
                    {
                        RenderEntityBatch(RenderCommands, State, Batch);
                    }
                    else
                    {
                        for (u32 EntityIndex = 0; EntityIndex < Batch->EntityCount; ++EntityIndex)
                        {
                            game_entity *Entity = Batch->Entities[EntityIndex];

                            RenderEntity(RenderCommands, State, Entity);
                        }
                    }
                }
            }

            // todo:
            State->PrevDanceMode = State->DanceMode;

            if (State->Options.ShowGrid)
            {
                DrawGround(RenderCommands);
            }

            font *Font = GetFontAsset(&State->Assets, "Consolas");

            if (State->Assets.State != GameAssetsState_Ready)
            {
                DrawText(RenderCommands, "Loading assets...", Font, vec3(0.f, 0.f, 0.f), 1.f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignCenter, DrawText_ScreenSpace);
            }

#if 0
            if (State->Assets.State == GameAssetsState_Ready)
            {
                for (u32 PlaybleEntityIndex = 0; PlaybleEntityIndex < ArrayCount(State->PlayableEntities); ++PlaybleEntityIndex)
                {
                    game_entity *PlayableEntity = State->PlayableEntities[PlaybleEntityIndex];

                    if (PlayableEntity && !PlayableEntity->Destroyed && PlayableEntity->Model)
                    {
                        bounds Bounds = GetEntityBounds(PlayableEntity);
                        vec3 Size = Bounds.Max - Bounds.Min;
                        f32 YOffset = 0.2f;

                        vec3 TextPosition = vec3(
                            PlayableEntity->Transform.Translation.x, 
                            Size.y + YOffset,
                            PlayableEntity->Transform.Translation.z
                        );

                        DrawText(RenderCommands, PlayableEntity->Model->Key, Font, TextPosition, 0.25f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignCenter, DrawText_WorldSpace, true);
                    }
                }
            }
#endif

#if 1
            if (State->Player && State->Player->Model)
            {
                char Text[64];
                FormatString(Text, "Active entity: %s", State->Player->Model->Key);
                DrawText(RenderCommands, Text, Font, vec3(-9.8f, -5.4f, 0.f), 0.5f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignLeft, DrawText_ScreenSpace);
            }
#endif

            break;
        }
        case GameMode_Menu:
        {
            RenderCommands->Settings.EnableCascadedShadowMaps = false;

            Clear(RenderCommands, vec4(0.f));

            vec3 TopLeft = vec3(-3.f, 3.f, 0.f);
            vec3 TopRight = vec3(3.f, 3.f, 0.f);
            vec3 BottomRight = vec3(3.f, -3.f, 0.f);
            vec3 BottomLeft = vec3(-3.f, -3.f, 0.f);

            f32 MoveSpeed = 0.5f;

            for (u32 QuadIndex = 0; QuadIndex < ArrayCount(State->MenuQuads); ++QuadIndex)
            {
                game_menu_quad *Quad = State->MenuQuads + QuadIndex;

                Quad->Move += Parameters->Delta * MoveSpeed;

                if (Quad->Move >= 1.f)
                {
                    Quad->Move = 0.f;
                    Quad->Corner = (Quad->Corner + 1) % 4;
                }

                vec3 From;
                vec3 To;

                switch (Quad->Corner)
                {
                    case 0:
                    {
                        From = TopLeft;
                        To = TopRight;
                        break;
                    }
                    case 1:
                    {
                        From = TopRight;
                        To = BottomRight;
                        break;
                    }
                    case 2:
                    {
                        From = BottomRight;
                        To = BottomLeft;
                        break;
                    }
                    case 3:
                    {
                        From = BottomLeft;
                        To = TopLeft;
                        break;
                    }
                    default:
                    {
                        Assert(!"Wrong side");
                        break;
                    }
                }

                DrawRectangle(
                    RenderCommands,
                    CreateTransform(Lerp(From, Quad->Move, To), vec3(0.5f), quat(0.f)),
                    Quad->Color
                );
            }

            font *Font = GetFontAsset(&State->Assets, "Where My Keys");
            DrawText(RenderCommands, "Dummy", Font, vec3(0.f, 0.f, 0.f), 2.f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignCenter, DrawText_ScreenSpace);

            break;
        }
    }

    {
        PROFILE(Memory->Profiler, "GameRender:ProcessEvents");
        ProcessEvents(State, AudioCommands, RenderCommands);
    }

    // todo: should probably go to FrameEnd ?
    // todo: maybe move it out to platform layer to be more explicit? (maybe create FrameStart/FrameEnd functions?)
    ClearMemoryArena(&State->TransientArena);
}
