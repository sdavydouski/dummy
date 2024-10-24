﻿#include "dummy_assets.cpp"
#include "dummy_audio.cpp"
#include "dummy_renderer.cpp"
#include "dummy_events.cpp"
#include "dummy_bounds.cpp"
#include "dummy_body.cpp"
#include "dummy_collision.cpp"
#include "dummy_contact.cpp"
#include "dummy_spatial.cpp"
#include "dummy_camera.cpp"
#include "dummy_particles.cpp"
#include "dummy_animation.cpp"
#include "dummy_animator.cpp"
#include "dummy_process.cpp"
#include "dummy_visibility.cpp"
#include "dummy_save.cpp"

inline vec3
NormalizeRGB(vec3 RGB)
{
    vec3 Result = RGB / 255.f;
    return Result;
}

inline vec3
RandomColor(random_sequence *Random)
{
    vec3 Result = vec3(Random01(Random), Random01(Random), Random01(Random));
    return Result;
}

inline material_options
DefaultMaterialOptions()
{
    material_options Result = {};
    Result.Wireframe = false;
    Result.CastShadow = true;

    return Result;
}

inline material
CreateBasicMaterial(vec4 Color, material_options Options = DefaultMaterialOptions())
{
    material Result = {};

    Result.Type = MaterialType_Basic;
    Result.Color = Color;
    Result.Options = Options;

    return Result;
}

inline material
CreatePhongMaterial(mesh_material *MeshMaterial, material_options Options = DefaultMaterialOptions())
{
    material Result = {};

    Result.Type = MaterialType_Phong;
    Result.MeshMaterial = MeshMaterial;
    Result.Options = Options;

    return Result;
}

inline material
CreateStandardMaterial(mesh_material *MeshMaterial, material_options Options = DefaultMaterialOptions())
{
    material Result = {};

    Result.Type = MaterialType_Standard;
    Result.MeshMaterial = MeshMaterial;
    Result.Options = Options;

    return Result;
}

inline material
CreateMaterial(mesh_material *MeshMaterial, material_options Options = DefaultMaterialOptions())
{
    material Result = {};

    switch (MeshMaterial->ShadingMode)
    {
        case ShadingMode_Phong:
        {
            Result = CreatePhongMaterial(MeshMaterial, Options);
            break;
        }
        case ShadingMode_PBR:
        {
            Result = CreateStandardMaterial(MeshMaterial, Options);
            break;
        }
    }

    return Result;
}

inline void
DrawModel(render_commands *RenderCommands, model *Model, transform Transform, vec3 Color)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        if (Mesh->Visible)
        {
            mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
            material Material = CreateMaterial(MeshMaterial);
            Material.Color = vec4(Color, 1.f);

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
            material Material = CreateMaterial(MeshMaterial);

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
            material Material = CreateMaterial(MeshMaterial);

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
            material Material = CreateMaterial(MeshMaterial);

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

inline u32
GenerateGameProcessId(game_state *State)
{
    u32 Result = State->NextFreeProcessId++;
    return Result;
}

inline u32
GenerateAudioSourceId(game_state *State)
{
    u32 Result = State->NextFreeAudioSourceId++;
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
    Model->BoundsOBB = Asset->BoundsOBB;
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
                MaterialProperty->Type == MaterialProperty_Texture_Albedo ||
                MaterialProperty->Type == MaterialProperty_Texture_Metalness ||
                MaterialProperty->Type == MaterialProperty_Texture_Roughness ||
                MaterialProperty->Type == MaterialProperty_Texture_Normal
                )
            {
                MaterialProperty->TextureId = GenerateTextureId(State);
                AddTexture(RenderCommands, MaterialProperty->TextureId, &MaterialProperty->Bitmap);
            }
        }
    }
}

dummy_internal void
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

dummy_internal void
InitTexture(game_state *State, texture_asset *Asset, texture *Texture, const char *Name, render_commands *RenderCommands)
{
    CopyString(Name, Texture->Key);
    Texture->Bitmap = Asset->Bitmap;
    Texture->TextureId = GenerateTextureId(State);

    AddTexture(RenderCommands, Texture->TextureId, &Texture->Bitmap);
}

dummy_internal void
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

    Skinning->BindPose = Model->BindPose;

    Skinning->Pose = PushType(Arena, skeleton_pose);
    Skinning->Pose->Skeleton = Model->Skeleton;
    Skinning->Pose->LocalJointPoses = PushArray(Arena, Model->Skeleton->JointCount, transform);
    Skinning->Pose->GlobalJointPoses = PushArray(Arena, Model->Skeleton->JointCount, mat4, Align(16));

    for (u32 JointIndex = 0; JointIndex < Model->BindPose->Skeleton->JointCount; ++JointIndex)
    {
        transform *SourceLocalJointPose = Model->BindPose->LocalJointPoses + JointIndex;
        transform *DestLocalJointPose = Skinning->Pose->LocalJointPoses + JointIndex;

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
    Result.Origin = Camera->Position;
    Result.Direction = Normalize(WorldRay);

    return Result;
}

dummy_internal entity_render_batch *
GetRenderBatch(game_state *State, char *Name)
{
    entity_render_batch *Result = HashTableLookup(&State->EntityBatches, Name);
    return Result;
}

dummy_internal game_asset *
GetGameAssets(platform_api *Platform, const wchar *Wildcard, memory_arena *Arena, u32 *AssetCount)
{
    get_files_result GetFilesResult = Platform->GetFiles((wchar *)Wildcard, Arena);
    *AssetCount = GetFilesResult.FileCount;

    game_asset *GameAssets = PushArray(Arena, *AssetCount, game_asset);

    for (u32 AssetIndex = 0; AssetIndex < *AssetCount; ++AssetIndex)
    {
        platform_file *ModelFile = GetFilesResult.Files + AssetIndex;
        game_asset *GameAsset = GameAssets + AssetIndex;

        char AssetFileNameName[256];
        ConvertToString(ModelFile->FileName, AssetFileNameName);

        char AssetName[256];
        RemoveExtension(AssetFileNameName, AssetName);

        char AssetPath[256];
        FormatString(AssetPath, "assets\\%s", AssetFileNameName);

        CopyString(AssetName, GameAsset->Name);
        CopyString(AssetPath, GameAsset->Path);
    }

    return GameAssets;
}

dummy_internal void
InitGameModelAssets(game_state *State, game_assets *Assets, render_commands *RenderCommands)
{
    InitHashTable(&Assets->Models, 1021, &Assets->Arena);

    Assert(Assets->Models.Count > Assets->ModelAssetCount);

    for (u32 GameAssetModelIndex = 0; GameAssetModelIndex < Assets->ModelAssetCount; ++GameAssetModelIndex)
    {
        game_asset_model *GameAssetModel = Assets->ModelAssets + GameAssetModelIndex;

        model *Model = GetModelAsset(Assets, GameAssetModel->GameAsset.Name);
        InitModel(State, GameAssetModel->ModelAsset, Model, GameAssetModel->GameAsset.Name, &Assets->Arena, RenderCommands);
    }
}

dummy_internal void
InitGameFontAssets(game_state *State, game_assets *Assets, render_commands *RenderCommands)
{
    InitHashTable(&Assets->Fonts, 31, &Assets->Arena);

    Assert(Assets->Fonts.Count > Assets->FontAssetCount);

    for (u32 GameAssetFontIndex = 0; GameAssetFontIndex < Assets->FontAssetCount; ++GameAssetFontIndex)
    {
        game_asset_font *GameAssetFont = Assets->FontAssets + GameAssetFontIndex;

        font *Font = GetFontAsset(Assets, GameAssetFont->GameAsset.Name);
        InitFont(State, GameAssetFont->FontAsset, Font, GameAssetFont->GameAsset.Name, RenderCommands);
    }
}

dummy_internal void
InitGameTextureAssets(game_state *State, game_assets *Assets, render_commands *RenderCommands)
{
    InitHashTable(&Assets->Textures, 127, &Assets->Arena);

    Assert(Assets->Textures.Count > Assets->TextureAssetCount);

    for (u32 GameAssetTextureIndex = 0; GameAssetTextureIndex < Assets->TextureAssetCount; ++GameAssetTextureIndex)
    {
        game_asset_texture *GameAssetTexture = Assets->TextureAssets + GameAssetTextureIndex;

        texture *Texture = GetTextureAsset(Assets, GameAssetTexture->GameAsset.Name);
        InitTexture(State, GameAssetTexture->TextureAsset, Texture, GameAssetTexture->GameAsset.Name, RenderCommands);
    }
}

dummy_internal void
InitGameAudioClipAssets(game_assets *Assets)
{
    InitHashTable(&Assets->AudioClips, 251, &Assets->Arena);

    Assert(Assets->AudioClips.Count > Assets->AudioClipAssetCount);

    for (u32 GameAssetAudioClipIndex = 0; GameAssetAudioClipIndex < Assets->AudioClipAssetCount; ++GameAssetAudioClipIndex)
    {
        game_asset_audio_clip *GameAssetAudioClip = Assets->AudioClipAssets + GameAssetAudioClipIndex;

        audio_clip *AudioClip = GetAudioClipAsset(Assets, GameAssetAudioClip->GameAsset.Name);
        InitAudioClip(GameAssetAudioClip->AudioAsset, AudioClip, GameAssetAudioClip->GameAsset.Name);
    }
}

dummy_internal void
LoadModelAssets(game_assets *Assets, platform_api *Platform)
{
    u32 ModelAssetCount;
    game_asset *ModelAssets = GetGameAssets(Platform, L"assets\\*.model.asset", &Assets->Arena, &ModelAssetCount);

    Assets->ModelAssetCount = ModelAssetCount;
    Assets->ModelAssets = PushArray(&Assets->Arena, Assets->ModelAssetCount, game_asset_model);

    for (u32 ModelAssetIndex = 0; ModelAssetIndex < ModelAssetCount; ++ModelAssetIndex)
    {
        game_asset GameAsset = ModelAssets[ModelAssetIndex];
        game_asset_model *GameAssetModel = Assets->ModelAssets + ModelAssetIndex;

        model_asset *LoadedAsset = LoadModelAsset(Platform, GameAsset.Path, &Assets->Arena);

        GameAssetModel->GameAsset = GameAsset;
        GameAssetModel->ModelAsset = LoadedAsset;
    }
}

dummy_internal void
LoadFontAssets(game_assets *Assets, platform_api *Platform)
{
    u32 FontAssetCount;
    game_asset *FontAssets = GetGameAssets(Platform, L"assets\\*.font.asset", &Assets->Arena, &FontAssetCount);

    Assets->FontAssetCount = FontAssetCount;
    Assets->FontAssets = PushArray(&Assets->Arena, Assets->FontAssetCount, game_asset_font);

    for (u32 FontAssetIndex = 0; FontAssetIndex < FontAssetCount; ++FontAssetIndex)
    {
        game_asset GameAsset = FontAssets[FontAssetIndex];

        game_asset_font *GameAssetFont = Assets->FontAssets + FontAssetIndex;

        font_asset *LoadedAsset = LoadFontAsset(Platform, GameAsset.Path, &Assets->Arena);

        GameAssetFont->GameAsset = GameAsset;
        GameAssetFont->FontAsset = LoadedAsset;
    }
}

dummy_internal void
LoadAudioClipAssets(game_assets *Assets, platform_api *Platform)
{
    u32 AudioClipAssetCount;
    game_asset *AudioClipAssets = GetGameAssets(Platform, L"assets\\*.audio.asset", &Assets->Arena, &AudioClipAssetCount);

    Assets->AudioClipAssetCount = AudioClipAssetCount;
    Assets->AudioClipAssets = PushArray(&Assets->Arena, Assets->AudioClipAssetCount, game_asset_audio_clip);

    for (u32 AudioClipAssetIndex = 0; AudioClipAssetIndex < AudioClipAssetCount; ++AudioClipAssetIndex)
    {
        game_asset GameAsset = AudioClipAssets[AudioClipAssetIndex];

        game_asset_audio_clip *GameAssetAudioClip = Assets->AudioClipAssets + AudioClipAssetIndex;

        audio_clip_asset *LoadedAsset = LoadAudioClipAsset(Platform, GameAsset.Path, &Assets->Arena);

        GameAssetAudioClip->GameAsset = GameAsset;
        GameAssetAudioClip->AudioAsset = LoadedAsset;
    }
}

dummy_internal void
LoadTextureAssets(game_assets *Assets, platform_api *Platform)
{
    u32 TextureAssetCount;
    game_asset *TextureAssets = GetGameAssets(Platform, L"assets\\*.texture.asset", &Assets->Arena, &TextureAssetCount);

    Assets->TextureAssetCount = TextureAssetCount;
    Assets->TextureAssets = PushArray(&Assets->Arena, Assets->TextureAssetCount, game_asset_texture);

    for (u32 TextureAssetIndex = 0; TextureAssetIndex < TextureAssetCount; ++TextureAssetIndex)
    {
        game_asset GameAsset = TextureAssets[TextureAssetIndex];

        game_asset_texture *GameAssetTexture = Assets->TextureAssets + TextureAssetIndex;

        texture_asset *LoadedAsset = LoadTextureAsset(Platform, GameAsset.Path, &Assets->Arena);

        GameAssetTexture->GameAsset = GameAsset;
        GameAssetTexture->TextureAsset = LoadedAsset;
    }
}

dummy_internal void
LoadGameAssets(game_assets *Assets, platform_api *Platform)
{
    // todo:
    Assets->State = GameAssetsState_Unloaded;

    LoadModelAssets(Assets, Platform);

    LoadAudioClipAssets(Assets, Platform);
    InitGameAudioClipAssets(Assets);

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

dummy_internal void
DrawSkeleton(render_commands *RenderCommands, game_state *State, skeleton_pose *Pose)
{
    font *Font = GetFontAsset(&State->Assets, "Consolas");

    for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Pose->Skeleton->Joints + JointIndex;
        transform LocalJointPose = Pose->LocalJointPoses[JointIndex];
        mat4 GlobalJointPose = Pose->GlobalJointPoses[JointIndex];

        transform Transform = CreateTransform(GetTranslation(GlobalJointPose), vec3(0.01f), LocalJointPose.Rotation);
        vec4 Color = vec4(1.f, 1.f, 0.f, 1.f);

        DrawBox(RenderCommands, Transform, Color);
        DrawText(RenderCommands, Joint->Name, Font, Transform.Translation, 0.05f, vec4(0.f, 0.f, 1.f, 1.f), DrawText_AlignCenter, DrawMode_WorldSpace, true);

        if (Joint->ParentIndex > -1)
        {
            mat4 ParentGlobalJointPose = Pose->GlobalJointPoses[Joint->ParentIndex];

            vec3 LineStart = GetTranslation(ParentGlobalJointPose);
            vec3 LineEnd = GetTranslation(GlobalJointPose);

            DrawLine(RenderCommands, LineStart, LineEnd, vec4(1.f, 0.f, 1.f, 1.f), 2.f, DrawMode_WorldSpace);
        }
    }
}

dummy_internal void
RenderFrustrum(render_commands *RenderCommands, polyhedron *Frustrum)
{
    // Draw edges
    for (u32 EdgeIndex = 0; EdgeIndex < Frustrum->EdgeCount; ++EdgeIndex)
    {
        edge Edge = Frustrum->Edges[EdgeIndex];

        vec3 LineStart = Frustrum->Vertices[Edge.VertexIndex[0]];
        vec3 LineEnd = Frustrum->Vertices[Edge.VertexIndex[1]];
        DrawLine(RenderCommands, LineStart, LineEnd, vec4(1.f, 0.f, 1.f, 1.f), 4.f, DrawMode_WorldSpace);
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

dummy_internal void
RenderSpatialGrid(render_commands *RenderCommands, game_state *State, spatial_hash_grid *Grid)
{
    font *Font = GetFontAsset(&State->Assets, "Consolas");
    vec3 HalfCellSize = Grid->CellSize / 2.f;

    u32 CellsWithEntitiesCount = 0;
    for (u32 CellIndex = 0; CellIndex < Grid->TotalCellCount; ++CellIndex)
    {
        spatial_hash_grid_cell *Cell = Grid->Cells + CellIndex;

        if (Cell->EntityCount > 0)
        {
            vec3 NormalizedPosition = vec3(
                (f32)Cell->Coords.x / Grid->CellCount.x,
                (f32)Cell->Coords.y / Grid->CellCount.y,
                (f32)Cell->Coords.z / Grid->CellCount.z
            );

            vec3 GridBoundsMin = Grid->Bounds.Min();
            vec3 GridBoundsMax = Grid->Bounds.Max();

            vec3 CellPosition = NormalizedPosition * (GridBoundsMax - GridBoundsMin) + GridBoundsMin + HalfCellSize;

            transform Transform = CreateTransform(CellPosition, HalfCellSize);
            vec4 Color = vec4(0.f, 1.f, 1.f, 1.f);

            char Line1[256];
            FormatString(Line1, "Cell Coords: %d, %d, %d", Cell->Coords.x, Cell->Coords.y, Cell->Coords.z);

            char Line2[256];
            FormatString(Line2, "Entity Count: %d", Cell->EntityCount);

            vec3 TextOffset = vec3(0.f, HalfCellSize.y, 0.f);

            DrawBox(RenderCommands, Transform, Color);
            DrawText(RenderCommands, Line1, Font, CellPosition + TextOffset, 0.25f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignCenter, DrawMode_WorldSpace, true);
            DrawText(RenderCommands, Line2, Font, CellPosition + TextOffset * 0.9f, 0.25f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignCenter, DrawMode_WorldSpace, true);
        }
    }
}

dummy_internal void
RenderBoundingBox(render_commands *RenderCommands, game_state *State, game_entity *Entity)
{
    aabb Bounds = GetEntityBounds(Entity);
    transform BoxTransform = CreateTransform(Bounds.Center, Bounds.HalfExtent);
    vec4 BoxColor = vec4(0.f, 1.f, 0.f, 1.f);

    if (Entity->Model)
    {
        BoxColor = vec4(0.f, 1.f, 1.f, 1.f);
    }

    if (Entity->Collider)
    {
        BoxColor = vec4(0.f, 0.f, 1.f, 1.f);

        transform T1 = Decompose(Entity->Collider->Box.Transform);
        transform T2 = CreateTransform(Entity->Transform.Translation, T1.Scale, T1.Rotation);
        mat4 M = Transform(T2) * Entity->Collider->Box.Offset;

        BoxTransform.Translation = GetTranslation(M);
        BoxTransform.Scale = T1.Scale * Entity->Collider->Box.HalfSize;
        BoxTransform.Rotation = T1.Rotation;
    }

    if (Entity->Body)
    {
        BoxColor = vec4(1.f, 0.f, 1.f, 1.f);
    }

    // Pivot point
    DrawPoint(RenderCommands, Entity->Transform.Translation, vec4(1.f), 10.f);
    DrawBox(RenderCommands, BoxTransform, BoxColor);
}

dummy_internal void
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
        DrawModel(RenderCommands, Entity->Model, Entity->Transform, Entity->DebugColor);
    }
}

dummy_internal void
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
    Batch->Entities = PushArray(Arena, Batch->MaxEntityCount, game_entity *, NoClear());

    if (Batch->Skinning)
    {
        Batch->SkinnedMeshInstances = PushArray(Arena, Batch->MaxEntityCount, skinned_mesh_instance, NoClear());
    }
    else
    {
        Batch->MeshInstances = PushArray(Arena, Batch->MaxEntityCount, mesh_instance, NoClear());
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
    Entity->DebugColor = vec3(1.f);

    return Entity;
}

inline void
RemoveGameEntity(game_state *State, game_entity *Entity)
{
    Assert(!Entity->Destroyed);

    RemoveFromSpacialGrid(&State->WorldArea.SpatialGrid, Entity);

    Entity->Destroyed = true;
}

inline game_entity *
GetGameEntity(game_state *State, u32 EntityId)
{
    // todo:
    world_area *Area = &State->WorldArea;
    game_entity *Entity = Area->Entities + (EntityId - 1);
    return Entity;
}

inline void
AddModel(game_state *State, game_entity *Entity, game_assets *Assets, const char *ModelName, render_commands *RenderCommands, memory_arena *Arena)
{
    Entity->Model = GetModelAsset(Assets, ModelName);

    if (HasJoints(Entity->Model->Skeleton))
    {
        Entity->Skinning = PushType(Arena, skinning_data, Align(16));
        InitSkinningBuffer(State, Entity->Skinning, Entity->Model, Arena, RenderCommands);

        if (Entity->Model->AnimationCount > 0)
        {
            Entity->Animation = PushType(Arena, animation_graph, Align(16));
            BuildAnimationGraph(Entity->Animation, Entity->Model->AnimationGraph, Entity->Model, Arena);
        }
    }
}

inline void
AddBoxCollider(game_entity *Entity, vec3 HalfSize, mat4 Offset, memory_arena *Arena)
{
    Entity->Collider = PushType(Arena, collider, Align(16));
    Entity->Collider->Type = Collider_Box;
    Entity->Collider->Box.HalfSize = HalfSize;
    Entity->Collider->Box.Offset = Offset;

    CalculateColliderState(Entity);

    // todo:
    if (Entity->Body)
    {
        Entity->Collider->Box.Body = Entity->Body;
    }
}

inline void
AddBoxCollider(game_entity *Entity, memory_arena *Arena)
{
    Assert(Entity->Model);

    aabb Bounds = Entity->Model->Bounds;

    vec3 HalfSize = Bounds.HalfExtent;
    mat4 Offset = Translate(Bounds.Center);

    AddBoxCollider(Entity, HalfSize, Offset, Arena);
}

inline void
AddRigidBody(game_state *State, game_entity *Entity, memory_arena *Arena)
{
    Entity->Body = PushType(Arena, rigid_body, Align(16));

    f32 Mass = 10.f;
    vec3 Size = vec3(1.f);

    SetPosition(Entity->Body, Entity->Transform.Translation);
    SetOrientation(Entity->Body, Entity->Transform.Rotation);
    SetMass(Entity->Body, Mass);
    SetCenterOfMass(Entity->Body, vec3(0.f));
    SetInertiaTensor(Entity->Body, GetCuboidInertiaTensor(Mass, Size));
    SetLinearDamping(Entity->Body, 0.95f);
    SetAngularDamping(Entity->Body, 0.8f);

    CalculateRigidBodyState(Entity->Body);

    // todo:
    if (Entity->Collider)
    {
        Entity->Collider->Box.Body = Entity->Body;
    }
}

inline void
AddPointLight(game_entity *Entity, vec3 Color, light_attenuation Attenuation, memory_arena *Arena)
{
    Entity->PointLight = PushType(Arena, point_light, Align(16));
    Entity->PointLight->Position = Entity->Transform.Translation;
    Entity->PointLight->Color = Color;
    Entity->PointLight->Attenuation = Attenuation;
}

inline void
AddParticleEmitter(game_entity *Entity, u32 ParticleCount, u32 ParticlesSpawn, vec4 Color, vec2 Size, memory_arena *Arena)
{
    Entity->ParticleEmitter = PushType(Arena, particle_emitter, Align(16));
    Entity->ParticleEmitter->ParticleCount = ParticleCount;
    Entity->ParticleEmitter->Particles = PushArray(Arena, ParticleCount, particle, Align(16));
    Entity->ParticleEmitter->ParticlesSpawn = ParticlesSpawn;
    Entity->ParticleEmitter->Color = Color;
    Entity->ParticleEmitter->Size = Size;
    Entity->ParticleEmitter->NextParticleIndex = 0;
}

inline void
AddAudioSource(game_state *State, game_entity *Entity, audio_clip *AudioClip, vec3 Position, f32 Volume, f32 MinDistance, f32 MaxDistance, audio_commands *AudioCommands, memory_arena *Arena)
{
    Entity->AudioSource = PushType(Arena, audio_source, Align(16));
    Entity->AudioSource->AudioClip = AudioClip;
    Entity->AudioSource->Volume = Volume;
    Entity->AudioSource->MinDistance = MinDistance;
    Entity->AudioSource->MaxDistance = MaxDistance;
    Entity->AudioSource->IsPlaying = true;
    Entity->AudioSource->Id = GenerateAudioSourceId(State);

    // todo:
    bool32 IsLooping = true;
    Play3D(AudioCommands, AudioClip, Position, MinDistance, MaxDistance, SetAudioPlayOptions(Volume, IsLooping), Entity->AudioSource->Id);
}

inline void
CopyGameEntity(game_state *State, render_commands *RenderCommands, audio_commands *AudioCommands, game_entity *Source, game_entity *Dest)
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

        switch (Source->Collider->Type)
        {
            case Collider_Box:
            {
                AddBoxCollider(Dest, Source->Collider->Box.HalfSize, Source->Collider->Box.Offset, &Area->Arena);
                break;
            }
            default:
            {
                Assert(!"Invalid collider type");
            }
        }

        
    }

    if (Source->Body)
    {
        AddRigidBody(State, Dest, &Area->Arena);
    }

    if (Source->PointLight)
    {
        AddPointLight(Dest, Source->PointLight->Color, Source->PointLight->Attenuation, &Area->Arena);
    }

    if (Source->ParticleEmitter)
    {
        AddParticleEmitter(Dest, Source->ParticleEmitter->ParticleCount, Source->ParticleEmitter->ParticlesSpawn, Source->ParticleEmitter->Color, Source->ParticleEmitter->Size, &Area->Arena);
    }

    if (Source->AudioSource)
    {
        AddAudioSource(State, Dest, Source->AudioSource->AudioClip, Source->Transform.Translation, Source->AudioSource->Volume, Source->AudioSource->MinDistance, Source->AudioSource->MaxDistance, AudioCommands, &Area->Arena);
    }
}

dummy_internal void *
GetAnimatorParams(game_state *State, game_input *Input, game_entity *Entity, memory_arena *Arena)
{
    void *Params = 0;

    switch (SID(Entity->Animation->Animator))
    {
        case SID("Bot"):
        {
            Params = PushType(Arena, bot_animator_params);

            if (State->Player == Entity && State->Mode == GameMode_World)
            {
                GameInput2BotAnimatorParams(State, Input, Entity, (bot_animator_params *)Params);
            }
            else
            {
                GameLogic2BotAnimatorParams(State, Entity, (bot_animator_params *)Params);
            }

            break;
        }
        case SID("Paladin"):
        {
            Params = PushType(Arena, paladin_animator_params);

            if (State->Player == Entity && State->Mode == GameMode_World)
            {
                GameInput2PaladinAnimatorParams(State, Input, Entity, (paladin_animator_params *)Params);
            }
            else
            {
                GameLogic2PaladinAnimatorParams(State, Entity, (paladin_animator_params *)Params);
            }

            break;
        }
        case SID("Monstar"):
        {
            Params = PushType(Arena, monstar_animator_params);

            if (State->Player == Entity && State->Mode == GameMode_World)
            {
                GameInput2MonstarAnimatorParams(State, Input, Entity, (monstar_animator_params *)Params);
            }
            else
            {
                GameLogic2MonstarAnimatorParams(State, Entity, (monstar_animator_params *)Params);
            }

            break;
        }
        default:
        {
            Assert(!"Animator is not supported");
            break;
        }
    }

    return Params;
}

dummy_internal void
AnimateEntity(game_state *State, game_input *Input, game_entity *Entity, memory_arena *Arena, f32 Delta)
{
    Assert(Entity->Skinning);

    skeleton_pose *Pose = Entity->Skinning->Pose;
    skeleton_pose *BindPose = Entity->Skinning->BindPose;
    transform *Root = GetRootLocalJointPose(Pose);

    if (Entity->Animation)
    {
        void *Params = GetAnimatorParams(State, Input, Entity, Arena);

        AnimatorPerFrameUpdate(&State->Animator, Entity->Animation, Params, Delta);
        AnimationGraphPerFrameUpdate(Entity->Animation, Delta);
        CalculateSkeletonPose(Entity->Animation, BindPose, Pose, Entity->Id, &State->EventList, Arena);

        // Root Motion
        Entity->Animation->AccRootMotion.x += Pose->RootMotion.x;
        Entity->Animation->AccRootMotion.z += Pose->RootMotion.z;

        vec3 ScaledRootMotion = Entity->Animation->AccRootMotion * Entity->Transform.Scale;
        vec3 RotatedScaledRootMotion = Rotate(ScaledRootMotion, Entity->Transform.Rotation);

#if 1
        if (Entity->Body)
        {
            if (NearlyEqual(Entity->Body->Velocity.x, 0.f))
            {
                Entity->Body->Position.x += RotatedScaledRootMotion.x;
            }

            if (NearlyEqual(Entity->Body->Velocity.z, 0.f))
            {
                Entity->Body->Position.z += RotatedScaledRootMotion.z;
            }
        }
#endif

        Entity->Animation->AccRootMotion = vec3(0.f);
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
    game_state *State;
    platform_api *Platform;
};

JOB_ENTRY_POINT(UpdateEntityBatchJob)
{
    update_entity_batch_job *Data = (update_entity_batch_job *) Parameters;

    game_state *State = Data->State;
    platform_api *Platform = Data->Platform;
    contact_resolver *ContactResolver = &State->ContactResolver;

    f32 dt = Data->UpdateRate;

    for (u32 EntityIndex = Data->StartIndex; EntityIndex < Data->EndIndex; ++EntityIndex)
    {
        game_entity *Entity = Data->Entities + EntityIndex;

        if (!Entity->Destroyed)
        {
            rigid_body *Body = Entity->Body;
            collider *Collider = Entity->Collider;

            if (Body)
            {
                bool32 OnTheGround = NearlyEqual(Entity->Body->Position.y, 0.f);

                if (Collider)
                {
                    // Collision with the ground
#if 0
                    {
                        aabb EntityBounds = Entity->Collider->Bounds;

                        //
                        vec3 Acceleration = Body->Acceleration + Body->ForceAccumulator * Body->InverseMass;
                        vec3 Velocity = Body->Velocity + Body->Acceleration * dt;
                        Velocity *= Power(Body->LinearDamping, dt);
                        Velocity *= dt;
                        //

                        f32 CollisionTime;
                        vec3 ContactPoint;
                        if (IntersectMovingAABBPlane(EntityBounds, State->Ground, Velocity, &CollisionTime, &ContactPoint) && InRange(CollisionTime, 0.f, 1.f))
                        {
                            //Body->Position.y = ContactPoint.y;
                            OnTheGround = true;
                        }
                    }
#else
                    if (TestBoxPlane(&Collider->Box, State->Ground))
                    {
                        OnTheGround = true;
                    }
#endif
                }

                u32 MaxNearbyEntityCount = 10;
                game_entity **NearbyEntities = PushArray(&Data->Arena, MaxNearbyEntityCount, game_entity *);
                aabb Bounds = CreateAABBMinMax(vec3(0.f, -0.01f, 0.f), vec3(0.f, 0.f, 0.f));

                u32 NearbyEntityCount = FindNearbyEntities(Data->SpatialGrid, Entity, Bounds, NearbyEntities, MaxNearbyEntityCount);

                f32 MinDistance = F32_MAX;

                for (u32 NearbyEntityIndex = 0; NearbyEntityIndex < NearbyEntityCount; ++NearbyEntityIndex)
                {
                    game_entity *NearbyEntity = NearbyEntities[NearbyEntityIndex];
                    collider *NearbyBodyCollider = NearbyEntity->Collider;

                    if (NearbyBodyCollider)
                    {
                        ray Ray = {};
                        Ray.Origin = Body->Position;
                        Ray.Direction = vec3(0.f, -1.f, 0.f);

                        vec3 IntersectionPoint;
                        if (IntersectRayAABB(Ray, NearbyEntity->Collider->Bounds, IntersectionPoint))
                        {
                            f32 Distance = Magnitude(Body->Position - IntersectionPoint);

                            if (Distance < MinDistance)
                            {
                                MinDistance = Distance;
                            }
                        }
                    }
                }

                bool32 OnTheSurface = (MinDistance <= 0.01f);

                Entity->IsGrounded = (OnTheGround || OnTheSurface);

                // todo:
#if 1
                Body->PrevPosition = Body->Position;
                Body->PrevVelocity = Body->Velocity;
                Body->PrevAcceleration = Body->Acceleration;
#endif

                if (Entity->IsGrounded)
                {
                    if (Body->Position.y < 0.f)
                    {
                        Body->Position.y = 0.f;
                    }

                    if (Body->Velocity.y < 0.f)
                    {
                        Body->Velocity.y = 0.f;
                    }

                    if (Body->Acceleration.y < 0.f)
                    {
                        Body->Acceleration.y = 0.f;
                    }
                }

                if (!Entity->IsGrounded && !Entity->IsManipulated)
                {
                    vec3 Gravity = vec3(0.f, -20.f, 0.f) * GetMass(Body);
;                   AddForce(Body, Gravity);
                }

                vec2 HorizontalVelocity = vec2(Body->Velocity.x, Body->Velocity.z);

                if (Entity->IsGrounded && Magnitude(HorizontalVelocity) > 0.f)
                {
                    vec3 Drag = -10.f * Body->Velocity * Max(Magnitude(HorizontalVelocity), 5.f);
                    Drag.y = 0.f;
                    AddForce(Body, Drag);
                }

                Integrate(Body, dt);

                if (Collider)
                {
                    CalculateColliderState(Entity);

                    // Collisions with nearby entities
                    u32 MaxNearbyEntityCount = 100;
                    game_entity **NearbyEntities = PushArray(&Data->Arena, MaxNearbyEntityCount, game_entity *);
                    aabb Bounds = CreateAABBMinMax(vec3(-1.0f), vec3(1.0f));

                    u32 NearbyEntityCount = FindNearbyEntities(Data->SpatialGrid, Entity, Bounds, NearbyEntities, MaxNearbyEntityCount);

                    for (u32 NearbyEntityIndex = 0; NearbyEntityIndex < NearbyEntityCount; ++NearbyEntityIndex)
                    {
                        game_entity *NearbyEntity = NearbyEntities[NearbyEntityIndex];
                        rigid_body *NearbyBody = NearbyEntity->Body;
                        collider *NearbyBodyCollider = NearbyEntity->Collider;

                        if (NearbyEntity->Id == Entity->Id) continue;

#if 0
                        if (Entity->Id == Data->PlayerId)
                        {
                            NearbyEntity->DebugColor = vec3(0.f, 1.f, 0.f);
                        }
#endif

                        if (NearbyBodyCollider)
                        {
                            //CalculateColliderInternalState(Entity);
                            //CalculateColliderInternalState(NearbyEntity);
                            vec3 mtv;
                            if (TestAABBAABB(Collider->Bounds, NearbyBodyCollider->Bounds, &mtv))
                            {
                                Body->Position += mtv;
                            }
#if 0
                            if (TestBoxBox(&Collider->Box, &NearbyBodyCollider->Box))
                            {
                                //Entity->DebugColor = vec3(0.f, 1.f, 0.f);
                                //NearbyEntity->DebugColor = vec3(0.f, 1.f, 0.f);

                                contact_params ContactParams =
                                {
                                    .Friction = 0.6f,
                                    .Restitution = 0.2f
                                };
                                u32 ContactCount = CalculateBoxBoxContacts(&Collider->Box, &NearbyBodyCollider->Box, ContactResolver->Contacts + ContactResolver->ContactCount, ContactParams);
                                ContactResolver->ContactCount += ContactCount;

                                Assert(ContactResolver->ContactCount < ContactResolver->MaxContactCount);
                            }
#endif
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
    game_input *Input;
    game_entity *Entity;
    memory_arena Arena;
    f32 Delta;
};

JOB_ENTRY_POINT(AnimateEntityJob)
{
    animate_entity_job *Data = (animate_entity_job *) Parameters;
    AnimateEntity(Data->State, Data->Input, Data->Entity, &Data->Arena, Data->Delta);
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
                CalculateColliderState(Entity);
            }

            if (Entity->Model)
            {
                aabb BoundingBox = GetEntityBounds(Entity);

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

dummy_internal void
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

    animator_controller *SimpleController = HashTableLookup(&Animator->Controllers, (char *) "Simple");
    SimpleController->Func = SimpleAnimatorController;
}

inline void
Entity2Spec(game_entity *Entity, game_entity_spec *Spec)
{
    CopyString(Entity->Name, Spec->Name);
    Spec->Transform = Entity->Transform;
    Spec->DebugColor = Entity->DebugColor;

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

    if (Entity->AudioSource)
    {
        AudioSource2Spec(Entity->AudioSource, &Spec->AudioSourceSpec);
    }
}

dummy_internal void
Spec2Entity(game_entity_spec *Spec, game_entity *Entity, game_state *State, render_commands *RenderCommands, audio_commands *AudioCommands, memory_arena *Arena)
{
    CopyString(Spec->Name, Entity->Name);
    Entity->Transform = Spec->Transform;

    if (Spec->DebugColor != vec3(0.f))
    {
        Entity->DebugColor = Spec->DebugColor;
    }

    if (Spec->ModelSpec.Has)
    {
        model *Model = GetModelAsset(&State->Assets, Spec->ModelSpec.ModelRef);
        AddModel(State, Entity, &State->Assets, Model->Key, RenderCommands, Arena);
    }

    if (Spec->ColliderSpec.Has)
    {
        collider_spec *ColliderSpec = &Spec->ColliderSpec;

        switch (ColliderSpec->Type)
        {
            case Collider_Box:
            {
                AddBoxCollider(Entity, ColliderSpec->Box.HalfSize, ColliderSpec->Box.Offset, Arena);
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
        AddRigidBody(State, Entity, Arena);
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

    if (Spec->AudioSourceSpec.Has)
    {
        audio_source_spec AudioSourceSpec = Spec->AudioSourceSpec;

        audio_clip *AudioClip = GetAudioClipAsset(&State->Assets, Spec->AudioSourceSpec.AudioClipRef);
        AddAudioSource(State, Entity, AudioClip, Entity->Transform.Translation, AudioSourceSpec.Volume, AudioSourceSpec.MinDistance, AudioSourceSpec.MaxDistance, AudioCommands, Arena);
    }
}

dummy_internal void
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

dummy_internal void
LoadWorldAreaFromFile(game_state *State, char *FileName, platform_api *Platform, render_commands *RenderCommands, audio_commands *AudioCommands, memory_arena *TempArena)
{
    read_file_result Result = Platform->ReadFile(FileName, TempArena, ReadBinary());

    world_area *Area = &State->WorldArea;

    game_file_header *Header = GET_DATA_AT(Result.Contents, 0, game_file_header);
    game_entity_spec *Specs = GET_DATA_AT(Result.Contents, sizeof(game_file_header), game_entity_spec);

    Assert(Header->MagicValue == 0x44332211);
    Assert(Header->Version == 1);
    Assert(Header->Type == GameFile_Area);

    u32 EntityCount = Header->EntityCount;

    aabb WorldBounds = CreateAABBMinMax(vec3(-100.f, 0.f, -100.f), vec3(100.f, 20.f, 100.f));
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

        Spec2Entity(Spec, Entity, State, RenderCommands, AudioCommands, &Area->Arena);
    }

    Out(&State->PermanentStream, "Loaded: %s (Entity Count: %d)", FileName, EntityCount);
}

dummy_internal void
LoadEntityFromFile(game_state *State,  game_entity *Entity, char *FileName, platform_api *Platform, render_commands *RenderCommands, audio_commands *AudioCommands, memory_arena *TempArena)
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

    Spec2Entity(Spec, Entity, State, RenderCommands, AudioCommands, &Area->Arena);
}

dummy_internal void
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

dummy_internal void
ClearWorldArea(game_state *State)
{
#if 0
    for (u32 EntityIndex = 0; EntityIndex < State->WorldArea.EntityCount; ++EntityIndex)
    {
        game_entity *Entity = State->WorldArea.Entities + EntityIndex;
        RemoveFromSpacialGrid(&State->WorldArea.SpatialGrid, Entity);
    }
#endif

    ClearMemoryArena(&State->WorldArea.Arena);

    InitSpatialHashGrid(&State->WorldArea.SpatialGrid, State->WorldArea.SpatialGrid.Bounds, State->WorldArea.SpatialGrid.CellSize, &State->WorldArea.Arena);

    State->WorldArea.EntityCount = 0;
    State->WorldArea.Entities = PushArray(&State->WorldArea.Arena, State->WorldArea.MaxEntityCount, game_entity);

    State->NextFreeEntityId = 1;
    State->SelectedEntity = 0;
    State->Player = 0;
}

inline game_camera *
GetActiveCamera(game_state *State)
{
    game_camera *Camera = 0;

    switch (State->Mode)
    {
        case GameMode_World:
        {
            Camera = &State->PlayerCamera;
            break;
        }
        case GameMode_Menu:
        case GameMode_Editor:
        {
            Camera = &State->EditorCamera;
            break;
        }
    }

    return Camera;
}

DLLExport GAME_INIT(GameInit)
{
    PROFILE(Memory->Profiler, "GameInit");

    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;

    umm PermanentArenaSize = Memory->PermanentStorageSize - sizeof(game_state);
    u8 *PermanentArenaBase = (u8 *) Memory->PermanentStorage + sizeof(game_state);
    InitMemoryArena(&State->PermanentArena, PermanentArenaBase, PermanentArenaSize);

    umm FrameArenaSize = Memory->FrameStorageSize;
    u8 *FrameArenaBase = Memory->FrameStorage;
    InitMemoryArena(&State->FrameArena, FrameArenaBase, FrameArenaSize);

    State->PermanentStream = CreateStream(SubMemoryArena(&State->PermanentArena, Megabytes(4)));
    State->FrameStream = CreateStream(SubMemoryArena(&State->PermanentArena, Megabytes(2)));

    State->WorldArea = {};
    State->WorldArea.Arena = SubMemoryArena(&State->PermanentArena, Megabytes(128));
    State->WorldArea.EntityCount = 0;
    State->WorldArea.MaxEntityCount = 10000;
    State->WorldArea.Entities = PushArray(&State->WorldArea.Arena, State->WorldArea.MaxEntityCount, game_entity);

    aabb WorldBounds = CreateAABBMinMax(vec3(-100.f, 0.f, -100.f), vec3(100.f, 20.f, 100.f));
    vec3 CellSize = vec3(5.f);
    InitSpatialHashGrid(&State->WorldArea.SpatialGrid, WorldBounds, CellSize, &State->WorldArea.Arena);

    State->JobQueue = Memory->JobQueue;

    State->NextFreeEntityId = 1;
    State->NextFreeMeshId = 1;
    State->NextFreeTextureId = 1;
    State->NextFreeSkinningId = 1;
    State->NextFreeProcessId = 1;
    State->NextFreeAudioSourceId = 1;

    // todo: temp
    State->SkyboxId = 1;

    InitBool32State(&State->DanceMode, false);

    State->ContactResolver = {};
    State->ContactResolver.PositionEpsilon = 0.001f;
    State->ContactResolver.VelocityEpsilon = 0.001f;
    State->ContactResolver.ContactCount = 0;
    State->ContactResolver.MaxContactCount = 1024;
    State->ContactResolver.Contacts = PushArray(&State->PermanentArena, State->ContactResolver.MaxContactCount, contact);

    game_process *Sentinel = &State->ProcessSentinel;
    Sentinel->Key = GenerateGameProcessId(State);
    Sentinel->Next = Sentinel->Prev = Sentinel;

    InitHashTable(&State->Processes, 251, &State->PermanentArena);

    // Event System
    game_event_list *EventList = &State->EventList;
    EventList->MaxEventCount = 4096;
    EventList->EventCount = 0;
    EventList->Events = PushArray(&State->PermanentArena, EventList->MaxEventCount, game_event);
    EventList->Platform = Platform;
    EventList->Arena = SubMemoryArena(&State->PermanentArena, Megabytes(4));

    // Animator Setup
    State->Animator = {};
    InitHashTable(&State->Animator.Controllers, 31, &State->PermanentArena);

    LoadAnimators(&State->Animator);
    //

    State->Mode = GameMode_Editor;

    Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);

    f32 AspectRatio = (f32)Params->WindowWidth / (f32)Params->WindowHeight;
    f32 FieldOfView = RADIANS(45.f);
    InitCamera(&State->EditorCamera, FieldOfView, AspectRatio, 0.1f, 1000.f, vec3(0.f, 4.f, 8.f), vec3(0.f, RADIANS(-90.f), RADIANS(-20.f)));
    InitCamera(&State->PlayerCamera, FieldOfView, AspectRatio, 0.1f, 320.f, vec3(0.f, 5.f, 0.f), vec3(4.f, RADIANS(0.f), RADIANS(20.f)));

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

    State->Options = {};
    State->Options.EnableShadows = true;
    State->Options.ShowBoundingVolumes = false;
    State->Options.ShowGrid = true;
    State->Options.ShowSkybox = true;
    State->Options.ShowSkeletons = false;
    State->Options.ShowCamera = false;
    State->Options.ShowSpatialGrid = false;
    State->Options.WireframeMode = false;

    InitGameMenu(State);

    State->Assets = {};

    umm AssetsArenaSize = Memory->AssetsStorageSize;
    u8 *AssetsArenaBase = Memory->AssetsStorage;
    InitMemoryArena(&State->Assets.Arena, AssetsArenaBase, AssetsArenaSize);

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

    State->MasterVolume = 0.f;

    {
        game_process_params Params = {};
        Params.InterpolationTime = 0.2f;
        StartGameProcess(State, SmoothInputMoveProcess, Params);
    }
}

DLLExport GAME_RELOAD(GameReload)
{
    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;

    // Restarting game processes
    game_process *GameProcess = State->ProcessSentinel.Next;

    while (GameProcess->OnUpdatePerFrame)
    {
        GameProcess->OnUpdatePerFrame = (game_process_on_update *) Platform->LoadFunction(Platform->PlatformHandle, GameProcess->Name);
        GameProcess = GameProcess->Next;
    }

    // Reloading animators
    LoadAnimators(&State->Animator);
}

dummy_internal void
SpawnBox(game_state *State, render_commands *RenderCommands)
{
    game_entity *Entity = CreateGameEntity(State);

#if 0
    Entity->Transform = CreateTransform(vec3(RandomBetween(&State->GeneralEntropy, -5.f, 5.f), 5.f, RandomBetween(&State->GeneralEntropy, -5.f, 5.f)));
#else
    Entity->Transform = CreateTransform(vec3(0.f, 5.f, 0.f), vec3(1.f));
#endif

    AddModel(State, Entity, &State->Assets, "box", RenderCommands, &State->WorldArea.Arena);
    AddBoxCollider(Entity, &State->WorldArea.Arena);
    AddRigidBody(State, Entity, &State->WorldArea.Arena);

    //State->SelectedEntity = Entity;
}

dummy_internal void
SpawnPlayer(game_state *State, render_commands *RenderCommands)
{
    game_entity *Entity = CreateGameEntity(State);

    f32 InverseMass = 0.f;
    vec3 Size = vec3(0.3f, 1.8f, 0.3f);

    Entity->Transform = CreateTransform(vec3(0.f, 0.f, 0.f));

    AddModel(State, Entity, &State->Assets, "ybot", RenderCommands, &State->WorldArea.Arena);
    AddBoxCollider(Entity, vec3(0.3f, 0.9f, 0.3f), Translate(vec3(0.f, 0.9f, 0.f)), &State->WorldArea.Arena);
    AddRigidBody(State, Entity, &State->WorldArea.Arena);

    State->SelectedEntity = Entity;
    State->Player = Entity;
}

DLLExport GAME_FRAME_START(GameFrameStart)
{
    game_state *State = GetGameState(Memory);


}

DLLExport GAME_FRAME_END(GameFrameEnd)
{
    game_state *State = GetGameState(Memory);

    ClearStream(&State->FrameStream);
    ClearMemoryArena(&State->FrameArena);
}

DLLExport GAME_INPUT(GameInput)
{
    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;
    render_commands *RenderCommands = GetRenderCommands(Memory);
    audio_commands *AudioCommands = GetAudioCommands(Memory);

    if (Input->Menu.IsActivated)
    {
        if (State->Mode == GameMode_Menu)
        {
            State->Mode = GameMode_Editor;
        }
        else
        {
            State->Mode = GameMode_Menu;
            InitGameMenu(State);
        }
    }

    if (Input->EditMode.IsActivated)
    {
        if (State->Mode == GameMode_World)
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

    if (Input->SpawnBox.IsActivated)
    {
#if 0
        SpawnBox(State, RenderCommands);
#else
        SpawnPlayer(State, RenderCommands);
#endif
    }

    switch (State->Mode)
    {
        case GameMode_World:
        {
            vec2 Move = Input->Move.Range;

            if (Abs(Move.x) == 1.f && Abs(Move.y) == 1.f)
            {
                Move = Normalize(Move);
            }
            if (Abs(Move.x) == 2.f && Abs(Move.y) == 2.f)
            {
                Move = Normalize(Move) * 2.f;
            }

            State->TargetMove = Move;

            game_entity *Player = State->Player;

            if (Player)
            {
                aabb PlayerBounds = GetEntityBounds(Player);
                vec3 PlayerSize = PlayerBounds.HalfExtent * 2.f;

                vec3 PlayerPosition = Player->Transform.Translation;
                vec3 TargetPosition = PlayerPosition + vec3(0.f, 0.8f * PlayerSize.y, 0.f);

                ChaseCameraPerFrameUpdate(&State->PlayerCamera, Input, State, TargetPosition, Params->Delta);
                ChaseCameraSceneCollisions(&State->PlayerCamera, &State->WorldArea.SpatialGrid, State->Player, &State->FrameArena);

                if (Player->Body)
                {
                    if (Input->Jump.IsActivated)
                    {
                        if (Player->IsGrounded)
                        {
                            Player->Body->Acceleration.y = 400.f;
                        }
                    }

                    vec3 xAxis = vec3(1.f, 0.f, 0.f);
                    vec3 yAxis = vec3(0.f, 1.f, 0.f);
                    vec3 zAxis = vec3(0.f, 0.f, 1.f);

                    vec3 yMoveAxis = Normalize(Projection(State->PlayerCamera.Direction, State->Ground));
                    vec3 xMoveAxis = Normalize(Orthogonal(yMoveAxis, State->Ground));

                    f32 xMoveY = Dot(yMoveAxis, xAxis) * Move.y;
                    f32 zMoveY = Dot(yMoveAxis, zAxis) * Move.y;

                    f32 xMoveX = Dot(xMoveAxis, xAxis) * Move.x;
                    f32 zMoveX = Dot(xMoveAxis, zAxis) * Move.x;

                    vec3 NewPlayerDirection = vec3(Dot(vec3(Move.x, 0.f, Move.y), xMoveAxis), 0.f, Dot(vec3(Move.x, 0.f, Move.y), yMoveAxis));

                    if (Player->IsGrounded)
                    {
                        Player->Body->Acceleration.x = (xMoveX + xMoveY) * 20.f;
                        Player->Body->Acceleration.z = (zMoveX + zMoveY) * 20.f;
                    }

                    quat NewPlayerOrientation = AxisAngle2Quat(vec4(yAxis, Atan2(NewPlayerDirection.x, NewPlayerDirection.z)));

                    f32 MoveMagnitude = Clamp(Magnitude(Move), 0.f, 1.f);

                    if (MoveMagnitude > 0.f)
                    {
                        f32 t = 5.f * Params->Delta;
                        quat OldPlayerOrientation = Player->Body->Orientation;

                        Player->Body->Orientation = Slerp(OldPlayerOrientation, t, NewPlayerOrientation);
                    }
                }
            }

            break;
        }
        case GameMode_Editor:
        {
            if (Input->EnableFreeCameraMovement.IsActive)
            {
                Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);
            }
            else
            {
                Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Cursor);
            }

            if (Input->LeftClick.IsActivated)
            {
                ray Ray = ScreenPointToWorldRay(Input->MouseCoords, vec2((f32)Params->WindowWidth, (f32)Params->WindowHeight), &State->EditorCamera);

                f32 MinDistance = F32_MAX;

                game_entity *SelectedEntity = 0;

                world_area *Area = &State->WorldArea;

                for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
                {
                    game_entity *Entity = Area->Entities + EntityIndex;

                    if (!Entity->Destroyed)
                    {
                        State->SelectedEntity = 0;
                        aabb Box = GetEntityBounds(Entity);

                        vec3 IntersectionPoint;
                        if (IntersectRayAABB(Ray, Box, IntersectionPoint))
                        {
                            f32 Distance = Magnitude(IntersectionPoint - State->EditorCamera.Position);

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

            if (Input->EnableFreeCameraMovement.IsActive)
            {
                FreeCameraPerFrameUpdate(&State->EditorCamera, Input, Params->UnscaledDelta);
            }

            break;
        }
        case GameMode_Menu:
        {
            break;
        }
    }
}

DLLExport GAME_UPDATE(GameUpdate)
{
    //PROFILE(Memory->Profiler, "GameUpdate");

    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;
    world_area *Area = &State->WorldArea;

    scoped_memory ScopedMemory(&State->FrameArena);

#if 1
    for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = Area->Entities + EntityIndex;

        if (Entity->Destroyed) continue;

        Entity->DebugColor = vec3(1.f); //Entity->TestColor;
    }
#endif

#if 1
    contact_resolver *ContactResolver = &State->ContactResolver;

    // todo: contact coherence across frames!
    for (u32 ContactIndex = 0; ContactIndex < ContactResolver->ContactCount; ++ContactIndex)
    {
        contact *Contact = ContactResolver->Contacts + ContactIndex;
        *Contact = {};
    }
    ContactResolver->ContactCount = 0;
    //
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
        JobData->UpdateRate = Params->UpdateRate;
        if (State->Player)
        {
            JobData->PlayerId = State->Player->Id;
        }
        JobData->Entities = Area->Entities;
        JobData->SpatialGrid = &Area->SpatialGrid;
        JobData->Entropy = &State->ParticleEntropy;
        JobData->Arena = SubMemoryArena(ScopedMemory.Arena, Megabytes(1), NoClear());
        JobData->State = State;
        JobData->Platform = Platform;

        Job->EntryPoint = UpdateEntityBatchJob;
        Job->Parameters = JobData;
    }

    if (UpdateEntityBatchJobCount > 0)
    {
        Platform->KickJobsAndWait(State->JobQueue, UpdateEntityBatchJobCount, UpdateEntityBatchJobs);
    }
#else
    // Single thread
    f32 dt = Params->UpdateRate;

    // todo: special code for player controller ?

    // Update bodies
    for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = Area->Entities + EntityIndex;

        if (Entity->Destroyed) continue;

        rigid_body *Body = Entity->Body;
        collider *Collider = Entity->Collider;
        particle_emitter *ParticleEmitter = Entity->ParticleEmitter;

        if (Body)
        {
            /*if (Entity->IsGrounded)
            {
                if (Body->Velocity.y < 0.f)
                {
                    Body->Velocity.y = 0.f;
                }

                if (Body->Acceleration.y < 0.f)
                {
                    Body->Acceleration.y = 0.f;
                }
            }*/

            if (!Entity->IsManipulated)
            {
                vec3 Gravity = vec3(0.f, -10.f, 0.f) * GetMass(Body);
                AddForce(Body, Gravity);
            }

            Integrate(Body, dt);
        }

        if (Collider)
        {
            CalculateColliderState(Entity);
        }

        if (ParticleEmitter)
        {
            for (u32 ParticleSpawnIndex = 0; ParticleSpawnIndex < ParticleEmitter->ParticlesSpawn; ++ParticleSpawnIndex)
            {
                particle *Particle = ParticleEmitter->Particles + ParticleEmitter->NextParticleIndex++;

                if (ParticleEmitter->NextParticleIndex >= ParticleEmitter->ParticleCount)
                {
                    ParticleEmitter->NextParticleIndex = 0;
                }

                random_sequence *Entropy = &State->ParticleEntropy;

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

    // Contact generation
    for (u32 EntityIndex = 0; EntityIndex < Area->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = Area->Entities + EntityIndex;

        if (Entity->Destroyed) continue;

        collider *Collider = Entity->Collider;
        rigid_body *Body = Entity->Body;

        if (Collider && Body)
        {
            bool32 OnTheGround = false;

            // Collision with the ground
            if (TestBoxPlane(&Collider->Box, State->Ground))
            {
                OnTheGround = true;

                contact_params ContactParams =
                {
                    .Friction = 0.5f,
                    .Restitution = 0.4f
                };
                State->ContactResolver.ContactCount += CalculateBoxPlaneContacts(&Entity->Collider->Box, State->Ground, State->ContactResolver.Contacts + State->ContactResolver.ContactCount, ContactParams);
                Assert(State->ContactResolver.ContactCount < State->ContactResolver.MaxContactCount);
            }

            Entity->IsGrounded = OnTheGround;

            // Collisions with nearby entities
            u32 MaxNearbyEntityCount = 100;
            game_entity **NearbyEntities = PushArray(ScopedMemory.Arena, MaxNearbyEntityCount, game_entity *);
            aabb Bounds = CreateAABBMinMax(vec3(-1.0f), vec3(1.0f));

            u32 NearbyEntityCount = FindNearbyEntities(&Area->SpatialGrid, Entity, Bounds, NearbyEntities, MaxNearbyEntityCount);

            for (u32 NearbyEntityIndex = 0; NearbyEntityIndex < NearbyEntityCount; ++NearbyEntityIndex)
            {
                game_entity *NearbyEntity = NearbyEntities[NearbyEntityIndex];
                rigid_body *NearbyBody = NearbyEntity->Body;
                collider *NearbyBodyCollider = NearbyEntity->Collider;

                if (NearbyEntity->Id == Entity->Id) continue;

                if (NearbyBodyCollider)
                {
                    //if (TestBoxBox(&Collider->Box, &NearbyBodyCollider->Box))
                    {
                        contact_params ContactParams =
                        {
                            .Friction = 0.6f,
                            .Restitution = 0.1f
                        };
                        State->ContactResolver.ContactCount += CalculateBoxBoxContacts(&Collider->Box, &NearbyBodyCollider->Box, State->ContactResolver.Contacts + State->ContactResolver.ContactCount, ContactParams);
                        Assert(State->ContactResolver.ContactCount < State->ContactResolver.MaxContactCount);
                    }
                }
            }

            CalculateColliderState(Entity);
        }
    }
#endif

    ResolveContacts(&State->ContactResolver, Params->UpdateRate);
}

DLLExport GAME_RENDER(GameRender)
{
    game_state *State = GetGameState(Memory);
    render_commands *RenderCommands = GetRenderCommands(Memory);
    audio_commands *AudioCommands = GetAudioCommands(Memory);
    platform_api *Platform = Memory->Platform;
    world_area *Area = &State->WorldArea;

    f32 ScreenWidthInUnits = 20.f;
    f32 PixelsPerUnit = (f32)Params->WindowWidth / ScreenWidthInUnits;
    f32 ScreenHeightInUnits = (f32)Params->WindowHeight / PixelsPerUnit;
    f32 Left = -ScreenWidthInUnits / 2.f;
    f32 Right = ScreenWidthInUnits / 2.f;
    f32 Bottom = -ScreenHeightInUnits / 2.f;
    f32 Top = ScreenHeightInUnits / 2.f;
    f32 Near = -10.f;
    f32 Far = 10.f;

    RenderCommands->Settings.WindowWidth = Params->WindowWidth;
    RenderCommands->Settings.WindowHeight = Params->WindowHeight;
    RenderCommands->Settings.Samples = Params->Samples;
    RenderCommands->Settings.Time = Params->Time;
    RenderCommands->Settings.ScreenWidthInUnits = ScreenWidthInUnits;
    RenderCommands->Settings.ScreenHeightInUnits = ScreenHeightInUnits;

    AudioCommands->Settings.Volume = State->MasterVolume;

#if 0
    char GameLog[256];
    FormatString(GameLog, "Contact Count: %d", State->ContactResolver.ContactCount);
    Out(&State->Stream, GameLog);
#endif
    //

    if (State->Assets.State == GameAssetsState_Loaded)
    {
#if 1
        // todo: render commands buffer is not multithread-safe!
        InitGameModelAssets(State, &State->Assets, RenderCommands);
        InitGameTextureAssets(State, &State->Assets, RenderCommands);

        AddSkybox(RenderCommands, 1, 1024, GetTextureAsset(&State->Assets, "environment_sky"));
        //AddSkybox(RenderCommands, 2, 1024, GetTextureAsset(&State->Assets, "environment_desert"));
        //AddSkybox(RenderCommands, 3, 1024, GetTextureAsset(&State->Assets, "environment_hill"));

        State->Assets.State = GameAssetsState_Ready;
#endif

        Play2D(AudioCommands, GetAudioClipAsset(&State->Assets, "Ambient 5"), SetAudioPlayOptions(0.1f, true), 2);

#if 0
        {
            scoped_memory ScopedMemory(&State->PermanentArena);
            LoadWorldAreaFromFile(State, (char *)"data\\scene_4.dummy", Platform, RenderCommands, AudioCommands, ScopedMemory.Arena);
            State->Player = (State->WorldArea.Entities + 4);
        }
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

    SetTime(RenderCommands, Params->Time);
    SetViewport(RenderCommands, 0, 0, Params->WindowWidth, Params->WindowHeight);
    SetScreenProjection(RenderCommands, Left, Right, Bottom, Top, Near, Far);

    switch (State->Mode)
    {
        case GameMode_World:
        case GameMode_Editor:
        {
            game_camera *Camera = GetActiveCamera(State);
            bool32 EnableFrustrumCulling = State->Mode == GameMode_World;

            // todo:
            Camera->AspectRatio = (f32)Params->WindowWidth / (f32)Params->WindowHeight;

            SetListener(AudioCommands, Camera->Position, -Camera->Direction);

            mat4 WorldToCamera = GetCameraTransform(Camera);

            RenderCommands->Settings.WireframeMode = State->Options.WireframeMode;
            RenderCommands->Settings.EnableShadows = State->Options.EnableShadows;
            RenderCommands->Settings.ShowCascades = State->Options.ShowCascades;
            RenderCommands->Settings.Camera = Camera;
            RenderCommands->Settings.WorldToCamera = GetCameraTransform(Camera);
            RenderCommands->Settings.CameraToWorld = Inverse(WorldToCamera);
            RenderCommands->Settings.DirectionalLight = &State->DirectionalLight;
            RenderCommands->Settings.CascadeBounds[0] = vec2(-0.1f, -5.f);
            RenderCommands->Settings.CascadeBounds[1] = vec2(-3.f, -15.f);
            RenderCommands->Settings.CascadeBounds[2] = vec2(-10.f, -40.f);
            RenderCommands->Settings.CascadeBounds[3] = vec2(-30.f, -120.f);

            Clear(RenderCommands, vec4(State->BackgroundColor, 1.f));

            game_process *GameProcess = State->ProcessSentinel.Next;

            while (GameProcess->OnUpdatePerFrame)
            {
                GameProcess->OnUpdatePerFrame(State, GameProcess, Params);
                GameProcess = GameProcess->Next;
            }

            SetViewProjection(RenderCommands, Camera);

            SetDirectionalLight(RenderCommands, State->DirectionalLight);

            SetSkybox(RenderCommands, State->SkyboxId);

            if (State->Assets.State == GameAssetsState_Ready && State->Options.ShowSkybox)
            {
                DrawSkybox(RenderCommands, State->SkyboxId);
            }

            if (State->Options.ShowGrid)
            {
                DrawGrid(RenderCommands);
            }

            if (State->Options.ShowSpatialGrid)
            {
                RenderSpatialGrid(RenderCommands, State, &State->WorldArea.SpatialGrid);
            }

            u32 ShadowPlaneCount = 0;
            plane *ShadowPlanes = 0;

            {
                PROFILE(Memory->Profiler, "GameRender:BuildVisibilityRegion");

                BuildFrustrumPolyhedron(&State->PlayerCamera, &State->Frustrum);

                polyhedron VisibilityRegion = State->Frustrum;

                // Camera is looking downwards
                if (State->PlayerCamera.Direction.y < 0.f)
                {
                    //f32 MinY = 1.f;
                    //plane LowestPlane = ComputePlane(vec3(-1.f, MinY, 0.f), vec3(0.f, MinY, 1.f), vec3(1.f, MinY, 0.f));

                    ClipPolyhedron(&State->Frustrum, State->Ground, &VisibilityRegion);
                }

                vec4 LightPosition = vec4(-State->DirectionalLight.Direction * 100.f, 0.f);

                ShadowPlanes = PushArray(&State->FrameArena, MaxPolyhedronFaceCount, plane);
                ShadowPlaneCount = CalculateShadowRegion(&VisibilityRegion, LightPosition, ShadowPlanes);

                if (State->Options.ShowCamera)
                {
                    RenderFrustrum(RenderCommands, &VisibilityRegion);

                    // Render camera axes
                    game_camera *Camera = &State->PlayerCamera;
                    mat4 CameraTransform = GetCameraTransform(Camera);

                    vec3 xAxis = CameraTransform[0].xyz;
                    vec3 yAxis = CameraTransform[1].xyz;
                    vec3 zAxis = CameraTransform[2].xyz;

                    vec3 Origin = Camera->Position;
                    f32 AxisLength = 3.f;

#if 1
                    DrawLine(RenderCommands, Origin, Origin + xAxis * AxisLength, vec4(1.f, 0.f, 0.f, 1.f), 4.f, DrawMode_WorldSpace);
                    DrawLine(RenderCommands, Origin, Origin + yAxis * AxisLength, vec4(0.f, 1.f, 0.f, 1.f), 4.f, DrawMode_WorldSpace);
                    DrawLine(RenderCommands, Origin, Origin + zAxis * AxisLength, vec4(0.f, 0.f, 1.f, 1.f), 4.f, DrawMode_WorldSpace);
#endif

                    if (State->Assets.State == GameAssetsState_Ready)
                    {
                        DrawBillboard(RenderCommands, Camera->Position, vec2(0.2f), GetTextureAsset(&State->Assets, "camera"));
                    }
                }
            }

            {
                PROFILE(Memory->Profiler, "GameRender:AnimateEntities");

                scoped_memory ScopedMemory(&State->FrameArena);

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
                        JobData->Input = Input;
                        JobData->Entity = Entity;
                        JobData->Arena = SubMemoryArena(ScopedMemory.Arena, Kilobytes(512), NoClear());
                        JobData->Delta = Params->Delta;

                        Job->EntryPoint = AnimateEntityJob;
                        Job->Parameters = JobData;
                    }
                }

                if (AnimationJobCount > 0)
                {
                    Platform->KickJobsAndWait(State->JobQueue, AnimationJobCount, AnimationJobs);
                }
            }

            {
                PROFILE(Memory->Profiler, "GameRender:ProcessEntities");

                scoped_memory ScopedMemory(&State->FrameArena);

                u32 EntityBatchCount = 100;
                u32 ProcessEntityBatchJobCount = Ceil((f32)Area->EntityCount / (f32)EntityBatchCount);
                job *ProcessEntityBatchJobs = PushArray(&State->FrameArena, ProcessEntityBatchJobCount, job);
                process_entity_batch_job *ProcessEntityBatchJobParams = PushArray(ScopedMemory.Arena, ProcessEntityBatchJobCount, process_entity_batch_job);

                for (u32 EntityBatchIndex = 0; EntityBatchIndex < ProcessEntityBatchJobCount; ++EntityBatchIndex)
                {
                    job *Job = ProcessEntityBatchJobs + EntityBatchIndex;
                    process_entity_batch_job *JobData = ProcessEntityBatchJobParams + EntityBatchIndex;

                    JobData->StartIndex = EntityBatchIndex * EntityBatchCount;
                    JobData->EndIndex = Min((i32)JobData->StartIndex + EntityBatchCount, (i32)Area->EntityCount);
                    JobData->Lag = Params->UpdateLag;
                    JobData->Entities = Area->Entities;
                    JobData->SpatialGrid = &Area->SpatialGrid;
                    JobData->ShadowPlaneCount = ShadowPlaneCount;
                    JobData->ShadowPlanes = ShadowPlanes;

                    Job->EntryPoint = ProcessEntityBatchJob;
                    Job->Parameters = JobData;
                }

                if (ProcessEntityBatchJobCount > 0)
                {
                    Platform->KickJobsAndWait(State->JobQueue, ProcessEntityBatchJobCount, ProcessEntityBatchJobs);
                }
            }

            {
                PROFILE(Memory->Profiler, "GameRender:PrepareRenderBuffer");

                u32 MaxPointLightCount = 32;
                u32 PointLightCount = 0;
                point_light *PointLights = PushArray(&State->FrameArena, MaxPointLightCount, point_light, NoClear());

                InitHashTable(&State->EntityBatches, 521, &State->FrameArena);

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

                                if (IsSlotEmpty(Batch->Key))
                                {
                                    InitRenderBatch(Batch, Entity, Area->MaxEntityCount, &State->FrameArena);
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
                                DrawBillboard(RenderCommands, PointLight->Position, vec2(0.2f), GetTextureAsset(&State->Assets, "point_light"));
                            }
                        }

                        if (Entity->AudioSource)
                        {
                            audio_source *AudioSource = Entity->AudioSource;

                            SetEmitter(AudioCommands, AudioSource->Id, Entity->Transform.Translation, AudioSource->Volume, AudioSource->MinDistance, AudioSource->MaxDistance);

                            if (State->Mode == GameMode_Editor)
                            {
                                DrawBillboard(RenderCommands, Entity->Transform.Translation, vec2(0.2f), GetTextureAsset(&State->Assets, "audio_source"));
                            }
                        }
                    }
                }

                // todo: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
                {
                    SetPointLights(RenderCommands, PointLightCount, PointLights);
                }
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

                scoped_memory ScopedMemory(&State->FrameArena);

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
                        JobData->CameraPosition = Camera->Position;
                        JobData->Delta = Params->Delta;

                        Job->EntryPoint = ProcessParticlesJob;
                        Job->Parameters = JobData;
                    }
                }

                if (ParticleJobCount > 0)
                {
                    Platform->KickJobsAndWait(State->JobQueue, ParticleJobCount, ParticleJobs);
                }
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
                                DrawBillboard(RenderCommands, Entity->Transform.Translation, vec2(0.2f), GetTextureAsset(&State->Assets, "particle_emitter"));
                            }
                        }
                    }
                }
            }

            SaveBool32State(&State->DanceMode);

            font *Font = GetFontAsset(&State->Assets, "Consolas");

            if (State->Assets.State != GameAssetsState_Ready)
            {
                DrawText(RenderCommands, "Loading assets...", Font, vec3(0.f, 0.f, 0.f), 0.5f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignCenter, DrawMode_ScreenSpace);
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

#if 0
            for (u32 ContactIndex = 0; ContactIndex < State->ContactResolver.ContactCount; ++ContactIndex)
            {
                contact *Contact = State->ContactResolver.Contacts + ContactIndex;

                DrawBox(RenderCommands, CreateTransform(Contact->Point, vec3(0.05f)), vec4(1.f, 1.f, 0.f, 1.f));

                char Label[32];
                FormatString(Label, "%x, %x", Contact->Features[0], Contact->Features[1]);

                //DrawText(RenderCommands, Label, Font, Contact->Point, 0.2f, vec4(1.f, 0.f, 1.f, 1.f), DrawText_AlignCenter, DrawMode_WorldSpace);
                //DrawLine(RenderCommands, Contact->Point, Contact->Point + Contact->Normal * 0.125f, vec4(1.f, 1.f, 0.f, 1.f), 8.f);

                // Draw contact basis axis
                DrawLine(RenderCommands, Contact->Point, Contact->Point + Contact->ContactToWorld.Column(0) * 0.25f, vec4(0.f, 0.f, 1.f, 1.f), 4.f, DrawMode_WorldSpace);
                DrawLine(RenderCommands, Contact->Point, Contact->Point + Contact->ContactToWorld.Column(1) * 0.25f, vec4(0.f, 1.f, 0.f, 1.f), 4.f, DrawMode_WorldSpace);
                DrawLine(RenderCommands, Contact->Point, Contact->Point + Contact->ContactToWorld.Column(2) * 0.25f, vec4(1.f, 0.f, 0.f, 1.f), 4.f, DrawMode_WorldSpace);
            }
#endif

#if 0
            if (State->Player && State->Player->Model)
            {
                char Text[256];
                FormatString(Text, "Active entity: %s", State->Player->Name);
                DrawText(RenderCommands, Text, Font, vec3(Left, Bottom, 0.f) + vec3(0.2f, 0.2f, 0.f), 0.4f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignLeft, DrawText_ScreenSpace);
            }
#endif

            // Draw world-axes
            vec3 Start = vec3(Left + 0.75f, Bottom + 0.75f, 0.f);
            mat4 CameraTransform = GetCameraTransform(Camera);

            vec4 Red = vec4(1.f, 0.f, 0.f, 1.f);
            vec4 Green = vec4(0.f, 1.f, 0.f, 1.f);
            vec4 Blue = vec4(0.f, 0.f, 1.f, 1.f);

            vec3 CameraX = CameraTransform.Column(0).xyz;
            vec3 CameraY = CameraTransform.Column(1).xyz;
            vec3 CameraZ = CameraTransform.Column(2).xyz;

            vec3 EndX = Start + CameraX * 0.5f;
            vec3 EndY = Start + CameraY * 0.5f;
            vec3 EndZ = Start + CameraZ * 0.5f;

            DrawLine(RenderCommands, Start, EndX, Red, 4.f, DrawMode_ScreenSpace);
            DrawLine(RenderCommands, Start, EndY, Green, 4.f, DrawMode_ScreenSpace);
            DrawLine(RenderCommands, Start, EndZ, Blue, 4.f, DrawMode_ScreenSpace);

            DrawText(RenderCommands, "X", Font, EndX + CameraX * 0.1f, 0.4f, Red, DrawText_AlignCenter, DrawMode_ScreenSpace);
            DrawText(RenderCommands, "Y", Font, EndY + CameraY * 0.1f, 0.4f, Green, DrawText_AlignCenter, DrawMode_ScreenSpace);
            DrawText(RenderCommands, "Z", Font, EndZ + CameraZ * 0.1f, 0.4f, Blue, DrawText_AlignCenter, DrawMode_ScreenSpace);

            break;
        }
        case GameMode_Menu:
        {
            RenderCommands->Settings.EnableShadows = false;

            Clear(RenderCommands, vec4(0.f));

            vec3 TopLeft = vec3(-3.f, 3.f, 0.f);
            vec3 TopRight = vec3(3.f, 3.f, 0.f);
            vec3 BottomRight = vec3(3.f, -3.f, 0.f);
            vec3 BottomLeft = vec3(-3.f, -3.f, 0.f);

            f32 MoveSpeed = 0.5f;

            for (u32 QuadIndex = 0; QuadIndex < ArrayCount(State->MenuQuads); ++QuadIndex)
            {
                game_menu_quad *Quad = State->MenuQuads + QuadIndex;

                Quad->Move += Params->Delta * MoveSpeed;

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
            DrawText(RenderCommands, "Dummy", Font, vec3(0.f, 0.f, 0.f), 1.5f, vec4(1.f, 1.f, 1.f, 1.f), DrawText_AlignCenter, DrawMode_ScreenSpace);

            break;
        }
    }

    {
        PROFILE(Memory->Profiler, "GameRender:ProcessEvents");
        ProcessEvents(State, AudioCommands, RenderCommands);
    }
}
