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
CreateBasicMaterial(vec4 Color, bool32 Wireframe = false, bool32 CastShadow = false)
{
    material Result = {};

    Result.Type = MaterialType_Basic;
    Result.Color = Color;
    Result.Wireframe = Wireframe;
    Result.CastShadow = CastShadow;

    return Result;
}

inline material
CreatePhongMaterial(mesh_material *MeshMaterial, bool32 Wireframe = false, bool32 CastShadow = true)
{
    material Result = {};

    Result.Type = MaterialType_Phong;
    Result.MeshMaterial = MeshMaterial;
    Result.Wireframe = Wireframe;
    Result.CastShadow = CastShadow;

    return Result;
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

            DrawMesh(RenderCommands, Mesh->MeshId, Transform, Material);
        }
    }
}

inline void
DrawModelInstanced(render_commands *RenderCommands, model *Model, u32 InstanceCount, mesh_instance *Instances)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        if (Mesh->Visible)
        {
            mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
            material Material = CreatePhongMaterial(MeshMaterial);

            DrawMeshInstanced(RenderCommands, Mesh->MeshId, InstanceCount, Instances, Material);
        }
    }
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
                RenderCommands, Mesh->MeshId, Material,
                Model->SkinningBufferId, Skinning->SkinningMatrixCount, Skinning->SkinningMatrices
            );
        }
    }
}

inline void
DrawSkinnedModelInstanced(render_commands *RenderCommands, model *Model, skinning_data *Skinning, u32 InstanceCount, skinned_mesh_instance *Instances)
{
    Assert(Model->Skeleton);

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        if (Mesh->Visible)
        {
            mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
            material Material = CreatePhongMaterial(MeshMaterial);

            DrawSkinnedMeshInstanced(RenderCommands, Mesh->MeshId, Material, Model->SkinningBufferId, InstanceCount, Instances);
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

inline bool32
HasJoints(skeleton *Skeleton)
{
    bool32 Result = Skeleton && Skeleton->JointCount > 1;
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

    if (HasJoints(Model->Skeleton))
    {
        Model->SkinningBufferId = GenerateSkinningBufferId(State);
    }

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        Mesh->MeshId = GenerateMeshId(State);
        Mesh->Visible = true;

        AddMesh(
            RenderCommands, Mesh->MeshId, Mesh->VertexCount,
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
InitTexture(game_state *State, texture_asset *Asset, texture *Texture, const char *Name, render_commands *RenderCommands)
{
    CopyString(Name, Texture->Key);
    Texture->Bitmap = Asset->Bitmap;
    Texture->TextureId = GenerateTextureId(State);

    AddTexture(RenderCommands, Texture->TextureId, &Texture->Bitmap);
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

    AddSkinningBuffer(RenderCommands, Model->SkinningBufferId, Skinning->SkinningMatrixCount);
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

inline bool32
IsRenderBatchEmpty(entity_render_batch *Batch)
{
    bool32 Result = StringEquals(Batch->Key, "");
    return Result;
}

internal void
LoadModelAssets(game_assets *Assets, platform_api *Platform)
{
    game_asset ModelAssets[] = {
        {
            "pelegrini",
            "assets\\pelegrini.model.asset"
        },
        {
            "xbot",
            "assets\\xbot.model.asset"
        },
        {
            "ybot",
            "assets\\ybot.model.asset"
        },
        {
            "paladin",
            "assets\\paladin.model.asset"
        },
        {
            "warrok",
            "assets\\warrok.model.asset"
        },
        {
            "maw",
            "assets\\maw.model.asset"
        },
        {
            "Ganfaul",
            "assets\\Ganfaul.model.asset"
        },
        {
            "cerberus",
            "assets\\cerberus.model.asset"
        },
        {
            "cube",
            "assets\\cube.model.asset"
        },
        {
            "sphere",
            "assets\\sphere.model.asset"
        },
        {
            "quad",
            "assets\\quad.model.asset",
        },

        // Dungeon Assets
        {
            "cleric",
            "assets\\cleric.model.asset"
        },

        {
            "floor_standard",
            "assets\\Floor_Standard.model.asset"
        },
        {
            "wall",
            "assets\\Wall.model.asset"
        },

        {
            "barrel",
            "assets\\Barrel.model.asset"
        },
        {
            "crate",
            "assets\\Crate.model.asset"
        },
        {
            "torch",
            "assets\\torch.model.asset"
        }
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
            "assets\\Consolas.font.asset"
        },
        {
            "Where My Keys",
            "assets\\Where My Keys.font.asset"
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
            "assets\\Ambient 5.audio.asset"
        },
        {
            "samba",
            "assets\\samba.audio.asset"
        },
        {
            "step_metal",
            "assets\\step_metal.audio.asset"
        },
        {
            "step_cloth1",
            "assets\\step_cloth1.audio.asset"
        },
        {
            "step_lth1",
            "assets\\step_lth1.audio.asset"
        },
        {
            "step_lth4",
            "assets\\step_lth4.audio.asset"
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
LoadTextureAssets(game_assets *Assets, platform_api *Platform)
{
    game_asset TextureAssets[] = {
        {
            "Environment",
            "assets\\environment.texture.asset"
        },

        {
            "PointLight",
            "assets\\point_light.texture.asset"
        },
        {
            "ParticleEmitter",
            "assets\\particle_emitter.texture.asset"
        }
    };

    Assets->TextureAssetCount = ArrayCount(TextureAssets);
    Assets->TextureAssets = PushArray(&Assets->Arena, Assets->TextureAssetCount, game_asset_texture);

    for (u32 TextureAssetIndex = 0; TextureAssetIndex < ArrayCount(TextureAssets); ++TextureAssetIndex)
    {
        game_asset GameAsset = TextureAssets[TextureAssetIndex];

        game_asset_texture *GameAssetTexture = Assets->TextureAssets + TextureAssetIndex;

        texture_asset *LoadedAsset = LoadTextureAsset(Platform, GameAsset.Path, &Assets->Arena);

        GameAssetTexture->GameAsset = GameAsset;
        GameAssetTexture->TextureAsset = LoadedAsset;
    }
}

internal void
LoadGameAssets(game_assets *Assets, platform_api *Platform)
{
    // todo:
    Assets->State = GameAssetsState_Unloaded;

    LoadModelAssets(Assets, Platform);
    LoadAudioClipAssets(Assets, Platform);
    LoadTextureAssets(Assets, Platform);

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
    Assets->Fonts.Count = 251;
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
InitGameTextureAssets(game_state *State, game_assets *Assets, render_commands *RenderCommands)
{
    // todo:
    Assets->Textures.Count = 251;
    Assets->Textures.Values = PushArray(&Assets->Arena, Assets->Textures.Count, texture);

    Assert(Assets->Textures.Count > Assets->TextureAssetCount);

    for (u32 GameAssetTextureIndex = 0; GameAssetTextureIndex < Assets->TextureAssetCount; ++GameAssetTextureIndex)
    {
        game_asset_texture *GameAssetTexture = Assets->TextureAssets + GameAssetTextureIndex;

        texture *Texture = GetTextureAsset(Assets, GameAssetTexture->GameAsset.Name);
        InitTexture(State, GameAssetTexture->TextureAsset, Texture, GameAssetTexture->GameAsset.Name, RenderCommands);
    }
}

internal void
InitGameAudioClipAssets(game_assets *Assets)
{
    // todo:
    Assets->AudioClips.Count = 251;
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
    //DrawPoint(RenderCommands, Entity->Transform.Translation, vec4(1.f, 0.f, 0.f, 1.f), 10.f);

    bounds Bounds = GetEntityBounds(Entity);

    vec3 HalfSize = (Bounds.Max - Bounds.Min) / 2.f;
    //vec3 BottomCenterPosition = Bounds.Min + vec3(HalfSize.x, 0.f, HalfSize.z);
    vec3 Position = Bounds.Min + vec3(HalfSize);
    quat Rotation = quat(0.f, 0.f, 0.f, 1.f);

    transform Transform = CreateTransform(Position, HalfSize, Rotation);
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
    if (HasJoints(Entity->Model->Skeleton))
    {
        if (State->Options.ShowSkeletons)
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
    if (Batch->Skinning)
    {
        if (State->Options.ShowSkeletons)
        {
            for (u32 EntityIndex = 0; EntityIndex < Batch->EntityCount; ++EntityIndex)
            {
                game_entity *Entity = Batch->Entities[EntityIndex];

                DrawSkeleton(RenderCommands, State, Entity->Skinning->Pose);
            }
        }
        else
        {
            DrawSkinnedModelInstanced(RenderCommands, Batch->Model, Batch->Skinning, Batch->EntityCount, Batch->SkinnedMeshInstances);
        }
    }
    else
    {
        DrawModelInstanced(RenderCommands, Batch->Model, Batch->EntityCount, Batch->MeshInstances);
    }

    // debug drawing
    // todo: instancing
    for (u32 EntityIndex = 0; EntityIndex < Batch->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = Batch->Entities[EntityIndex];

        if ((State->SelectedEntity && State->SelectedEntity->Id == Entity->Id) || State->Options.ShowBoundingVolumes)
        {
            RenderBoundingBox(RenderCommands, State, Entity);
        }
    }
}

inline void
InitRenderBatch(entity_render_batch *Batch, game_entity *Entity, u32 MaxEntityCount, memory_arena *Arena)
{
    Assert(Entity->Model);

    CopyString(Entity->Model->Key, Batch->Key);
    Batch->EntityCount = 0;
    Batch->MaxEntityCount = MaxEntityCount;
    Batch->Model = Entity->Model;
    Batch->Skinning = Entity->Skinning;
    Batch->Entities = PushArray(Arena, Batch->MaxEntityCount, game_entity *);

    if (Batch->Skinning)
    {
        Batch->SkinnedMeshInstances = PushArray(Arena, Batch->MaxEntityCount, skinned_mesh_instance);
    }
    else
    {
        Batch->MeshInstances = PushArray(Arena, Batch->MaxEntityCount, mesh_instance);
    }
}

inline void
AddEntityToRenderBatch(entity_render_batch *Batch, game_entity *Entity)
{
    Assert(Batch->EntityCount < Batch->MaxEntityCount);

    game_entity **NextFreeEntity = Batch->Entities + Batch->EntityCount;
    *NextFreeEntity = Entity;

    if (Entity->Skinning)
    {
        skinned_mesh_instance *NextFreeInstance = Batch->SkinnedMeshInstances + Batch->EntityCount;

        NextFreeInstance->SkinningMatrixCount = Entity->Skinning->SkinningMatrixCount;
        NextFreeInstance->SkinningMatrices = Entity->Skinning->SkinningMatrices;
    }
    else
    {
        mesh_instance *NextFreeInstance = Batch->MeshInstances + Batch->EntityCount;

        NextFreeInstance->Model = Transform(Entity->Transform);
        NextFreeInstance->Color = Entity->DebugColor;
    }

    Batch->EntityCount++;
}

inline game_entity *
CreateGameEntity(game_state *State)
{
    world_area *Area = &State->WorldArea;

    game_entity *Entity = Area->Entities + Area->EntityCount++;

    *Entity = {};

    Assert(Area->EntityCount <= Area->MaxEntityCount);

    Entity->Id = GenerateEntityId(State);
    CopyString("Unnamed", Entity->Name);
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

    if (HasJoints(Entity->Model->Skeleton))
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
AddRigidBody(game_entity *Entity, bool32 RootMotionEnabled, memory_arena *Arena)
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
AddParticleEmitter(game_entity *Entity, u32 ParticleCount, u32 ParticlesSpawn, vec4 Color, vec2 Size, memory_arena *Arena)
{
    Entity->ParticleEmitter = PushType(Arena, particle_emitter);
    Entity->ParticleEmitter->ParticleCount = ParticleCount;
    Entity->ParticleEmitter->Particles = PushArray(Arena, ParticleCount, particle, Align(16));
    Entity->ParticleEmitter->ParticlesSpawn = ParticlesSpawn;
    Entity->ParticleEmitter->Color = Color;
    Entity->ParticleEmitter->Size = Size;
    Entity->ParticleEmitter->NextParticleIndex = 0;
}

inline void
CopyGameEntity(game_state *State, render_commands *RenderCommands, game_entity *Source, game_entity *Dest)
{
    world_area *Area = &State->WorldArea;

    Dest->Transform = Source->Transform;

    if (Source->Model)
    {
        AddModel(State, Dest, &State->Assets, Source->Model->Key, RenderCommands, &Area->Arena);
    }

    if (Source->Collider)
    {
        Assert(Source->Collider->Type == Collider_Box);

        AddBoxCollider(Dest, Source->Collider->BoxCollider.Size, &Area->Arena);
    }

    if (Source->Body)
    {
        AddRigidBody(Dest, Source->Body->RootMotionEnabled, &Area->Arena);
    }

    if (Source->PointLight)
    {
        AddPointLight(Dest, Source->PointLight->Color, Source->PointLight->Attenuation, &Area->Arena);
    }

    if (Source->ParticleEmitter)
    {
        AddParticleEmitter(Dest, Source->ParticleEmitter->ParticleCount, Source->ParticleEmitter->ParticlesSpawn, Source->ParticleEmitter->Color, Source->ParticleEmitter->Size, &Area->Arena);
    }
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

    Params->ToStateActionIdle = State->Player == Entity;
    Params->ToStateStandingIdle = !(State->Player == Entity);

    Params->ToStateActionIdle1 = Random < 0.5f;
    Params->ToStateActionIdle2 = Random >= 0.5f;
    Params->ToStateIdle1 = Random < 0.5f;
    Params->ToStateIdle2 = Random >= 0.5f;

    if (Input->Dance.IsActivated || Changed(State->DanceMode))
    {
        Params->ToStateDancing = Input->Dance.IsActivated || State->DanceMode.Value;
        Params->ToStateActionIdleFromDancing = Input->Dance.IsActivated || !State->DanceMode.Value;
    }
}

inline void
GameLogic2BotAnimatorParams(game_state *State, game_entity *Entity, bot_animator_params *Params)
{
    f32 Random = Random01(&State->GeneralEntropy);

    // todo:
    Params->MaxTime = 8.f;
    Params->ToStateActionIdle = State->Player == Entity;
    Params->ToStateStandingIdle = !(State->Player == Entity);
    Params->ToStateActionIdle1 = 0.f <= Random && Random < 0.33f;
    Params->ToStateActionIdle2 = 0.33f <= Random && Random < 0.66f;

    if (Changed(State->DanceMode))
    {
        Params->ToStateDancing = State->DanceMode.Value;
        Params->ToStateActionIdleFromDancing = !State->DanceMode.Value;
    }
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

    if (Input->Dance.IsActivated || Changed(State->DanceMode))
    {
        Params->ToStateDancing = Input->Dance.IsActivated || State->DanceMode.Value;
        Params->ToStateActionIdleFromDancing = Input->Dance.IsActivated || !State->DanceMode.Value;
    }

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

    if (Changed(State->DanceMode))
    {
        Params->ToStateDancing = State->DanceMode.Value;
        Params->ToStateActionIdleFromDancing = !State->DanceMode.Value;
    }
}

inline void
GameInput2MonstarAnimatorParams(game_state *State, game_entity *Entity, monstar_animator_params *Params)
{
    game_input *Input = &State->Input;

    Params->Move = Clamp(Magnitude(State->CurrentMove), 0.f, 1.f);
    Params->MoveMagnitude = State->Mode == GameMode_World ? Clamp(Magnitude(Input->Move.Range), 0.f, 1.f) : 0.f;
    Params->Attack = State->Mode == GameMode_World && Input->LightAttack.IsActivated;

    if (Input->Dance.IsActivated || Changed(State->DanceMode))
    {
        Params->ToStateDancing = Input->Dance.IsActivated || State->DanceMode.Value;
        Params->ToStateIdleFromDancing = Input->Dance.IsActivated || !State->DanceMode.Value;
    }
}

inline void
GameLogic2MonstarAnimatorParams(game_state *State, game_entity *Entity, monstar_animator_params *Params)
{
    if (Changed(State->DanceMode))
    {
        Params->ToStateDancing = State->DanceMode.Value;
        Params->ToStateIdleFromDancing = !State->DanceMode.Value;
    }
}

inline void
GameInput2ClericAnimatorParams(game_state *State, game_entity *Entity, cleric_animator_params *Params)
{
    game_input *Input = &State->Input;

    Params->Move = Clamp(Magnitude(State->CurrentMove), 0.f, 1.f);
    Params->MoveMagnitude = State->Mode == GameMode_World ? Clamp(Magnitude(Input->Move.Range), 0.f, 1.f) : 0.f;

    if (Input->Dance.IsActivated || Changed(State->DanceMode))
    {
        Params->ToStateDancing = Input->Dance.IsActivated || State->DanceMode.Value;
        Params->ToStateIdleFromDancing = Input->Dance.IsActivated || !State->DanceMode.Value;
    }
}

inline void
GameLogic2ClericAnimatorParams(game_state *State, game_entity *Entity, cleric_animator_params *Params)
{
    if (Changed(State->DanceMode))
    {
        Params->ToStateDancing = State->DanceMode.Value;
        Params->ToStateIdleFromDancing = !State->DanceMode.Value;
    }
}

internal void *
GetAnimatorParams(game_state *State, game_entity *Entity, memory_arena *Arena)
{
    void *Params = 0;

    if (StringEquals(Entity->Animation->Animator, "Bot"))
    {
        Params = PushType(Arena, bot_animator_params);

        if (State->Player == Entity)
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

        if (State->Player == Entity)
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

        if (State->Player = Entity)
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

        if (State->Player == Entity)
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

internal void
SwapParticles(particle *A, particle *B)
{
    particle Temp = *A;
    *A = *B;
    *B = Temp;
}

internal i32
PartitionParticles(particle *Particles, i32 LowIndex, i32 HighIndex)
{
    i32 MiddleIndex = (HighIndex + LowIndex) / 2;
    particle Pivot = Particles[MiddleIndex];

    i32 LeftIndex = LowIndex - 1;
    i32 RightIndex = HighIndex + 1;

    while (true)
    {
        do
        {
            LeftIndex += 1;
        } 
        while (Particles[LeftIndex].CameraDistanceSquared > Pivot.CameraDistanceSquared);

        do
        {
            RightIndex -= 1;
        } 
        while (Particles[RightIndex].CameraDistanceSquared < Pivot.CameraDistanceSquared);

        if (LeftIndex >= RightIndex)
        {
            return RightIndex;
        }

        SwapParticles(Particles + LeftIndex, Particles + RightIndex);
    }
}

internal void
QuickSortParticles(particle *Particles, i32 LowIndex, i32 HighIndex)
{
    if (LowIndex >= 0 && HighIndex >= 0 && LowIndex < HighIndex)
    {
        i32 PivotIndex = PartitionParticles(Particles, LowIndex, HighIndex);

        QuickSortParticles(Particles, LowIndex, PivotIndex);
        QuickSortParticles(Particles, PivotIndex + 1, HighIndex);
    }
}

internal void
SortParticles(u32 ParticleCount, particle *Particles)
{
    QuickSortParticles(Particles, 0, ParticleCount - 1);
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
                            NearbyEntity->DebugColor = vec3(0.f, 1.f, 0.f);
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

                for (u32 ParticleSpawnIndex = 0; ParticleSpawnIndex < ParticleEmitter->ParticlesSpawn; ++ParticleSpawnIndex)
                {
                    particle *Particle = ParticleEmitter->Particles + ParticleEmitter->NextParticleIndex++;

                    if (ParticleEmitter->NextParticleIndex >= ParticleEmitter->ParticleCount)
                    {
                        ParticleEmitter->NextParticleIndex = 0;
                    }

                    random_sequence *Entropy = Data->Entropy;

                    Particle->Position = Entity->Transform.Translation + vec3(
                        RandomBetween(Entropy, -0.05f, 0.05f),
                        RandomBetween(Entropy, 0.f, 0.1f),
                        RandomBetween(Entropy, -0.05f, 0.05f)
                    );
                    Particle->Velocity = Rotate(vec3(RandomBetween(Entropy, -0.5f, 0.5f), RandomBetween(Entropy, 3.f, 6.f), RandomBetween(Entropy, -0.5f, 0.5f)), Entity->Transform.Rotation);
                    Particle->Acceleration = Rotate(vec3(0.f, -10.f, 0.f), Entity->Transform.Rotation);
                    Particle->Color = ParticleEmitter->Color;
                    Particle->dColor = vec4(0.f, 0.f, 0.f, -0.5f);
                    Particle->Size = ParticleEmitter->Size;
                    Particle->dSize = vec2(-0.1f);
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

struct process_particles_job
{
    particle_emitter *ParticleEmitter;
    vec3 CameraPosition;
    f32 Delta;
};

JOB_ENTRY_POINT(ProcessParticlesJob)
{
    process_particles_job *Data = (process_particles_job *) Parameters;

    particle_emitter *ParticleEmitter = Data->ParticleEmitter;

    for (u32 ParticleIndex = 0; ParticleIndex < ParticleEmitter->ParticleCount; ++ParticleIndex)
    {
        particle *Particle = ParticleEmitter->Particles + ParticleIndex;

        f32 dt = Data->Delta;

        Particle->Position += 0.5f * Particle->Acceleration * Square(dt) + Particle->Velocity * dt;
        Particle->Velocity += Particle->Acceleration * dt;
        Particle->Color += Particle->dColor * dt;
        Particle->Size += Particle->dSize * dt;

        Particle->Color.a = Max(Particle->Color.a, 0.f);
        Particle->Size.x = Max(Particle->Size.x, 0.f);
        Particle->Size.y = Max(Particle->Size.y, 0.f);

        Particle->CameraDistanceSquared = SquaredMagnitude(Particle->Position - Data->CameraPosition);
    }

    SortParticles(ParticleEmitter->ParticleCount, ParticleEmitter->Particles);
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
    CopyString(Entity->Name, Spec->Name);
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
    CopyString(Spec->Name, Entity->Name);
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
        AddParticleEmitter(Entity, ParticleEmitterSpec.ParticleCount, ParticleEmitterSpec.ParticlesSpawn, ParticleEmitterSpec.Color, ParticleEmitterSpec.Size, Arena);
    }
}

internal void
SaveWorldAreaToFile(game_state *State, char *FileName,  platform_api *Platform, memory_arena *Arena)
{
    u32 HeaderSize = sizeof(game_file_header);
    u32 BufferSize = HeaderSize + State->ActiveEntitiesCount * sizeof(game_entity_spec);
    void *Buffer = PushSize(Arena, BufferSize);

    game_file_header *Header = GET_DATA_AT(Buffer, 0, game_file_header);

    Header->MagicValue = 0x44332211;
    Header->Version = 1;
    Header->EntityCount = State->ActiveEntitiesCount;
    Header->Type = GameFile_Area;

    game_entity_spec *Specs = GET_DATA_AT(Buffer, HeaderSize, game_entity_spec);

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
LoadWorldAreaFromFile(game_state *State, char *FileName, platform_api *Platform, render_commands *RenderCommands, memory_arena *TempArena)
{
    read_file_result Result = Platform->ReadFile(FileName, TempArena, ReadBinary());

    world_area *Area = &State->WorldArea;

    game_file_header *Header = GET_DATA_AT(Result.Contents, 0, game_file_header);
    game_entity_spec *Specs = GET_DATA_AT(Result.Contents, sizeof(game_file_header), game_entity_spec);

    Assert(Header->MagicValue == 0x44332211);
    Assert(Header->Version == 1);
    Assert(Header->Type == GameFile_Area);

    u32 EntityCount = Header->EntityCount;

    bounds WorldBounds = { vec3(-100.f, 0.f, -100.f), vec3(100.f, 20.f, 100.f) };
    vec3 CellSize = vec3(5.f);
    InitSpatialHashGrid(&Area->SpatialGrid, WorldBounds, CellSize, &Area->Arena);

    // todo:
    Area->MaxEntityCount = 10000;
    Area->EntityCount = 0;
    Area->Entities = PushArray(&Area->Arena, Area->MaxEntityCount, game_entity);

    for (u32 EntityIndex = 0; EntityIndex < EntityCount; ++EntityIndex)
    {
        game_entity_spec *Spec = Specs + EntityIndex;

        game_entity *Entity = CreateGameEntity(State);

        Spec2Entity(Spec, Entity, State, RenderCommands, &Area->Arena);
        AddToSpacialGrid(&Area->SpatialGrid, Entity);
    }

    Out(&State->Stream, "Loaded %s (Entity Count: %d)", FileName, EntityCount);
}

internal void
LoadEntityFromFile(game_state *State,  game_entity *Entity, char *FileName, platform_api *Platform, render_commands *RenderCommands, memory_arena *TempArena)
{
    read_file_result Result = Platform->ReadFile(FileName, TempArena, ReadBinary());

    game_file_header *Header = GET_DATA_AT(Result.Contents, 0, game_file_header);
    game_entity_spec *Spec = GET_DATA_AT(Result.Contents, sizeof(game_file_header), game_entity_spec);

    Assert(Header->MagicValue == 0x44332211);
    Assert(Header->Version == 1);
    Assert(Header->Type == GameFile_Area);

    u32 EntityCount = Header->EntityCount;

    Assert(EntityCount == 1);

    world_area *Area = &State->WorldArea;

    Spec2Entity(Spec, Entity, State, RenderCommands, &Area->Arena);
    AddToSpacialGrid(&Area->SpatialGrid, Entity);
}

internal void
SaveEntityToFile(game_entity *Entity, char *FileName, platform_api *Platform, memory_arena *Arena)
{
    u32 HeaderSize = sizeof(game_file_header);
    u32 BufferSize = HeaderSize + sizeof(game_entity_spec);
    void *Buffer = PushSize(Arena, BufferSize);

    game_file_header *Header = GET_DATA_AT(Buffer, 0, game_file_header);

    Header->MagicValue = 0x44332211;
    Header->Version = 1;
    Header->EntityCount = 1;
    // todo: different type?
    Header->Type = GameFile_Area;

    game_entity_spec *Spec = GET_DATA_AT(Buffer, HeaderSize, game_entity_spec);

    Entity2Spec(Entity, Spec);
    Spec->Transform.Translation = vec3(0.f);

    Platform->WriteFile(FileName, Buffer, BufferSize);
}

internal void
ClearWorldArea(game_state *State)
{
    State->NextFreeEntityId = 0;

    State->WorldArea.EntityCount = 0;
    ClearMemoryArena(&State->WorldArea.Arena);

    State->SelectedEntity = 0;
    State->Player = 0;
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

    State->Stream = CreateStream(SubMemoryArena(&State->PermanentArena, Megabytes(4)));

    State->WorldArea = {};
    State->WorldArea.Arena = SubMemoryArena(&State->PermanentArena, Megabytes(64));
    State->WorldArea.EntityCount = 0;
    State->WorldArea.MaxEntityCount = 10000;
    State->WorldArea.Entities = PushArray(&State->WorldArea.Arena, State->WorldArea.MaxEntityCount, game_entity);

    bounds WorldBounds = { vec3(-100.f, 0.f, -100.f), vec3(100.f, 20.f, 100.f) };
    vec3 CellSize = vec3(5.f);
    InitSpatialHashGrid(&State->WorldArea.SpatialGrid, WorldBounds, CellSize, &State->WorldArea.Arena);

    State->JobQueue = Memory->JobQueue;

    State->NextFreeEntityId = 0;
    State->NextFreeMeshId = 1;
    State->NextFreeTextureId = 1;
    State->NextFreeSkinningId = 1;

    // todo: temp
    State->SkyboxId = 1234;

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
    State->Assets.Arena = SubMemoryArena(&State->PermanentArena, Megabytes(512));

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

    if (Input->Reset.IsActivated)
    {
        // ...
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

            if (!Entity->Destroyed)
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

            vec3 PlayerPosition = State->Player ? State->Player->Transform.Translation : vec3(0.f);

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
            if (Player && Player->Body)
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
                Move.y * FreeCamera->Direction) * FreeCameraSpeed * Parameters->UnscaledDelta;

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
    RenderCommands->Settings.Samples = Parameters->Samples;
    RenderCommands->Settings.Time = Parameters->Time;
    RenderCommands->Settings.PixelsPerUnit = PixelsPerUnit;
    RenderCommands->Settings.UnitsPerPixel = 1.f / PixelsPerUnit;

    AudioCommands->Settings.Volume = State->MasterVolume;

    if (State->Assets.State == GameAssetsState_Loaded)
    {
        // todo: render commands buffer is not multithread-safe!
        InitGameModelAssets(State, &State->Assets, RenderCommands);
        InitGameAudioClipAssets(&State->Assets);
        InitGameTextureAssets(State, &State->Assets, RenderCommands);

        State->Assets.State = GameAssetsState_Ready;

        AddSkybox(RenderCommands, State->SkyboxId, 4096, GetTextureAsset(&State->Assets, "Environment"));

        Play2D(AudioCommands, GetAudioClipAsset(&State->Assets, "Ambient 5"), SetAudioPlayOptions(0.1f, true), 2);

#if 0
        game_entity *ParentEntity = CreateGameEntity(State);
        ParentEntity->Transform.Translation = vec3(0.f);
        ParentEntity->Transform.Scale = vec3(1.f);
        AddModel(State, ParentEntity, &State->Assets, "cube", RenderCommands, &State->PermanentArena);
        AddBoxCollider(ParentEntity, vec3(1.f), &State->PermanentArena);

        game_entity *ChildEntity = CreateGameEntity(State);
        ChildEntity->Transform.Translation = vec3(1.f, 0.f, 1.f);
        ChildEntity->Transform.Scale = vec3(0.5f);
        AddModel(State, ChildEntity, &State->Assets, "cube", RenderCommands, &State->PermanentArena);
        AddBoxCollider(ChildEntity, vec3(0.5f), &State->PermanentArena);
#endif
    }

    if (Changed(State->DanceMode))
    {
        if (State->DanceMode.Value)
        {
            Pause(AudioCommands, 2);
            Play2D(AudioCommands, GetAudioClipAsset(&State->Assets, "samba"), SetAudioPlayOptions(0.75f, true), 3);
        }
        else
        {
            Stop(AudioCommands, 3);
            Resume(AudioCommands, 2);
        }
    }

    SetTime(RenderCommands, Parameters->Time);
    SetViewport(RenderCommands, 0, 0, Parameters->WindowWidth, Parameters->WindowHeight);
    SetScreenProjection(RenderCommands, Left, Right, Bottom, Top, Near, Far);

    switch (State->Mode)
    {
        case GameMode_World:
        case GameMode_Editor:
        {
            game_camera *Camera = State->Mode == GameMode_World ? &State->GameCamera : &State->EditorCamera;
            bool32 EnableFrustrumCulling = State->Mode == GameMode_World;

            SetListener(AudioCommands, Camera->Transform.Translation, -Camera->Direction);

            mat4 WorldToCamera = GetCameraTransform(Camera);

            RenderCommands->Settings.EnableCascadedShadowMaps = true;
            RenderCommands->Settings.ShowCascades = State->Options.ShowCascades;
            RenderCommands->Settings.Camera = Camera;
            RenderCommands->Settings.WorldToCamera = GetCameraTransform(Camera);
            RenderCommands->Settings.CameraToWorld = Inverse(WorldToCamera);
            RenderCommands->Settings.DirectionalLight = &State->DirectionalLight;

            Clear(RenderCommands, vec4(State->BackgroundColor, 1.f));

            game_process *GameProcess = State->ProcessSentinel.Next;

            while (GameProcess->OnUpdatePerFrame)
            {
                GameProcess->OnUpdatePerFrame(State, GameProcess, Parameters->Delta);
                GameProcess = GameProcess->Next;
            }

            SetViewProjection(RenderCommands, Camera);

            SetDirectionalLight(RenderCommands, State->DirectionalLight);

            if (State->Assets.State == GameAssetsState_Ready)
            {
                DrawSkybox(RenderCommands, State->SkyboxId);
            }

            if (State->Options.ShowGrid)
            {
                DrawGrid(RenderCommands);
            }

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

                    if (!Entity->Destroyed && Entity->Model && HasJoints(Entity->Model->Skeleton))
                    {
                        job *Job = AnimationJobs + AnimationJobCount;
                        animate_entity_job *JobData = AnimationJobParams + AnimationJobCount;

                        ++AnimationJobCount;

                        JobData->State = State;
                        JobData->Entity = Entity;
                        JobData->Arena = SubMemoryArena(ScopedMemory.Arena, Kilobytes(512), NoClear());
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
                                    InitRenderBatch(Batch, Entity, Area->MaxEntityCount, &State->TransientArena);
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

                            PointLight->Position = Entity->Transform.Translation;
                            PointLight->Color = Entity->PointLight->Color;
                            PointLight->Attenuation = Entity->PointLight->Attenuation;

                            if (State->Mode == GameMode_Editor)
                            {
                                DrawBillboard(RenderCommands, PointLight->Position, vec2(0.2f), GetTextureAsset(&State->Assets, "PointLight"));
                            }
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

                    if (Batch->EntityCount > BatchThreshold)
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

            {
                PROFILE(Memory->Profiler, "GameRender:ProcessParticles");

                scoped_memory ScopedMemory(&State->TransientArena);

                u32 ParticleJobCount = 0;
                u32 MaxParticleJobCount = Area->EntityCount;
                job *ParticleJobs = PushArray(ScopedMemory.Arena, MaxParticleJobCount, job);
                process_particles_job *ParticleJobParams = PushArray(ScopedMemory.Arena, MaxParticleJobCount, process_particles_job);

                for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
                {
                    game_entity *Entity = Area->Entities + EntityIndex;

                    if (!Entity->Destroyed && Entity->ParticleEmitter)
                    {
                        job *Job = ParticleJobs + ParticleJobCount;
                        process_particles_job *JobData = ParticleJobParams + ParticleJobCount;

                        ++ParticleJobCount;

                        JobData->ParticleEmitter = Entity->ParticleEmitter;
                        JobData->CameraPosition = Camera->Transform.Translation;
                        JobData->Delta = Parameters->Delta;

                        Job->EntryPoint = ProcessParticlesJob;
                        Job->Parameters = JobData;
                    }
                }

                Platform->KickJobsAndWait(State->JobQueue, ParticleJobCount, ParticleJobs);
            }   

            {
                PROFILE(Memory->Profiler, "GameRender:PushParticles");

                for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
                {
                    game_entity *Entity = Area->Entities + EntityIndex;

                    if (!Entity->Destroyed)
                    {
                        if (Entity->ParticleEmitter)
                        {
                            particle_emitter *ParticleEmitter = Entity->ParticleEmitter;

                            DrawParticles(RenderCommands, ParticleEmitter->ParticleCount, ParticleEmitter->Particles, 0);

                            if (State->Mode == GameMode_Editor)
                            {
                                DrawBillboard(RenderCommands, Entity->Transform.Translation, vec2(0.2f), GetTextureAsset(&State->Assets, "ParticleEmitter"));
                            }
                        }
                    }
                }
            }

            SavePrevValueState(&State->DanceMode);

            if (State->Assets.State == GameAssetsState_Ready)
            {
                texture *GrassTexture = GetTextureAsset(&State->Assets, "Grass");

                //DrawTexturedQuad(RenderCommands, CreateTransform(vec3(0.f, 0.5f, 0.f), vec3(0.5f), quat(0.f, 0.f, 0.f, 1.f)), GrassTexture);
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
                char Text[256];
                FormatString(Text, "Active entity: %s", State->Player->Name);
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
