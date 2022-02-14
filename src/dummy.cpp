#include "dummy_defs.h"
#include "dummy_math.h"
#include "dummy_random.h"
#include "dummy_memory.h"
#include "dummy_string.h"
#include "dummy_container.h"
#include "dummy_input.h"
#include "dummy_renderer.h"
#include "dummy_platform.h"
#include "dummy_physics.h"
#include "dummy_animation.h"
#include "dummy_assets.h"
#include "dummy.h"

#include "dummy_assets.cpp"
#include "dummy_collision.cpp"
#include "dummy_physics.cpp"
#include "dummy_renderer.cpp"
#include "dummy_animation.cpp"
#include "dummy_entity.cpp"
#include "dummy_process.cpp"

inline vec3
NormalizeRGB(vec3 RGB)
{
    vec3 Result = RGB / 255.f;
    return Result;
}

inline void
InitCamera(game_camera *Camera, f32 Pitch, f32 Yaw, f32 FovY, f32 NearClipPlane, f32 FarClipPlane, vec3 Position, vec3 Up = vec3(0.f, 1.f, 0.f))
{
    Camera->Pitch = Pitch;
    Camera->Yaw = Yaw;
    Camera->FovY = FovY;
    Camera->NearClipPlane = NearClipPlane;
    Camera->FarClipPlane = FarClipPlane;
    Camera->Position = Position;
    Camera->Up = Up;
    Camera->Direction = CalculateDirectionFromEulerAngles(Camera->Pitch, Camera->Yaw);
}

inline material
CreateMaterial(material_type Type, mesh_material *MeshMaterial)
{
    material Result = {};

    Result.Type = Type;
    Result.MeshMaterial = MeshMaterial;

    return Result;
}

inline material
CreateMaterial(material_type Type, vec4 Color, b32 Wireframe = false)
{
    material Result = {};

    Result.Type = Type;
    Result.Color = Color;
    Result.Wireframe = Wireframe;

    return Result;
}

inline void
DrawSkinnedModel(render_commands *RenderCommands, model *Model, skeleton_pose *Pose)
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
        material Material = CreateMaterial(MaterialType_BlinnPhong, MeshMaterial);

        DrawSkinnedMesh(
            RenderCommands, Mesh->Id, {}, Material,
            Model->SkinningMatrixCount, Model->SkinningMatrices
        );
    }
}

inline void
DrawModel(render_commands *RenderCommands, model *Model, transform Transform)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
        material Material = CreateMaterial(MaterialType_BlinnPhong, MeshMaterial);

        DrawMesh(RenderCommands, Mesh->Id, Transform, Material);
    }
}

inline void
DrawModel(render_commands *RenderCommands, model *Model, transform Transform, material Material)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;

        DrawMesh(RenderCommands, Mesh->Id, Transform, Material);
    }
}

inline void
DrawModelInstanced(render_commands *RenderCommands, model *Model, u32 InstanceCount, render_instance *Instances)
{
    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        mesh_material *MeshMaterial = Model->Materials + Mesh->MaterialIndex;
        material Material = CreateMaterial(MaterialType_BlinnPhong, MeshMaterial);

        DrawMeshInstanced(RenderCommands, Mesh->Id, InstanceCount, Instances, Material);
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
InitModel(model_asset *Asset, model *Model, const char *Name, memory_arena *Arena, render_commands *RenderCommands, u32 MaxInstanceCount = 0)
{
    *Model = {};

    CopyString(Name, Model->Name);
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

    Model->AnimationGraph = &Asset->AnimationGraph;
    Model->MeshCount = Asset->MeshCount;
    Model->Meshes = Asset->Meshes;
    Model->MaterialCount = Asset->MaterialCount;
    Model->Materials = Asset->Materials;
    Model->AnimationCount = Asset->AnimationCount;
    Model->Animations = Asset->Animations;

    u32 SkinningMatrixCount = 0;

    if (Model->Skeleton->JointCount > 1)
    {
        Model->SkinningMatrixCount = Model->Skeleton->JointCount;
        Model->SkinningMatrices = PushArray(Arena, Model->SkinningMatrixCount, mat4);

        SkinningMatrixCount = Model->Skeleton->JointCount;
    }

    for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Model->Meshes + MeshIndex;
        Mesh->Id = GenerateMeshId();
        AddMesh(
            RenderCommands, Mesh->Id, Mesh->VertexCount, 
            Mesh->Positions, Mesh->Normals, Mesh->Tangents, Mesh->Bitangents, Mesh->TextureCoords, Mesh->Weights, Mesh->JointIndices, 
            Mesh->IndexCount, Mesh->Indices, SkinningMatrixCount, MaxInstanceCount
        );

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

    Model->Bounds = CalculateAxisAlignedBoundingBox(Model);
}

inline ray
ScreenPointToWorldRay(vec2 ScreenPoint, vec2 ScreenSize, game_camera *Camera)
{
    vec4 ClipRay = vec4(
        2.f * ScreenPoint.x / ScreenSize.x - 1.f,
        1.f - 2.f * ScreenPoint.y / ScreenSize.y,
        -1.f, 1.f
    );

    // todo: pass view and projection matrices to the renderer?
    mat4 View = LookAt(Camera->Position, Camera->Position + Camera->Direction, Camera->Up);

    f32 Aspect = (f32)ScreenSize.x / (f32)ScreenSize.y;
    mat4 Projection = Perspective(Camera->FovY, Aspect, Camera->NearClipPlane, Camera->FarClipPlane);

    vec4 CameraRay = Inverse(Projection) * ClipRay;
    CameraRay = vec4(CameraRay.xy, -1.f, 0.f);

    vec3 WorldRay = (Inverse(View) * CameraRay).xyz;

    ray Result = {};
    Result.Origin = Camera->Position;
    Result.Direction = Normalize(WorldRay);

    return Result;
}

internal model *
GetModelAsset(game_assets *Assets, const char *Name)
{
    model *Result = HashTableLookup(Assets->ModelCount, Assets->Models, (char *)Name);
    return Result;
}

internal entity_render_batch *
GetEntityBatch(game_state *State, char *Name)
{
    entity_render_batch *Result = HashTableLookup(State->EntityBatchCount, State->EntityBatches, Name);
    return Result;
}

internal void
InitGameAssets(game_assets *Assets, platform_api *Platform, render_commands *RenderCommands, memory_arena *Arena)
{
    // Using a prime table size in conjunction with quadratic probing tends to yield 
    // the best coverage of the available table slots with minimal clustering
    Assets->ModelCount = 31;
    Assets->Models = PushArray(Arena, Assets->ModelCount, model);

    u32 ModelIndex = 0;

    {
        char Name[32] = "Pelegrini";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\pelegrini.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands);
    }

    {
        char Name[32] = "xBot";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\xbot.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands);
    }

    {
        char Name[32] = "yBot";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\ybot.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands);
    }

    // todo: sRGB?
    {
        char Name[32] = "Cube";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\cube.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands, 256);
    }

    {
        char Name[32] = "Sphere";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\sphere.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands, 256);
    }

    {
        char Name[32] = "Skull";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\skull.asset", Arena);
        // todo: increasing MaxInstanceCount causes crash in Release mode.
        InitModel(Asset, Model, Name, Arena, RenderCommands, 256);
    }

    {
        char Name[32] = "Floor";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\floor.asset", Arena);
        // todo: render instance count?
        InitModel(Asset, Model, Name, Arena, RenderCommands, 4096);
    }

    {
        char Name[32] = "Wall";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\wall.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands, 4096);
    }

    {
        char Name[32] = "Wall_90";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\wall_90.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands, 4096);
    }

    {
        char Name[32] = "Column";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\column.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands, 256);
    }

    {
        char Name[32] = "Banner Wall";
        model *Model = GetModelAsset(Assets, Name);
        model_asset *Asset = LoadModelAsset(Platform, (char *)"assets\\banner_wall.asset", Arena);
        InitModel(Asset, Model, Name, Arena, RenderCommands, 256);
    }
}

internal void
DrawSkeleton(render_commands *RenderCommands, game_state *State, skeleton_pose *Pose)
{
    for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Pose->Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Pose->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Pose->GlobalJointPoses + JointIndex;

        transform Transform = CreateTransform(GetTranslation(*GlobalJointPose), vec3(0.05f), quat(0.f));
        material Material = CreateMaterial(MaterialType_Unlit, vec4(1.f, 1.f, 0.f, 1.f));

        DrawModel(RenderCommands, GetModelAsset(&State->Assets, "Cube"), Transform, Material);

        if (Joint->ParentIndex > -1)
        {
            mat4 *ParentGlobalJointPose = Pose->GlobalJointPoses + Joint->ParentIndex;

            vec3 LineStart = GetTranslation(*ParentGlobalJointPose);
            vec3 LineEnd = GetTranslation(*GlobalJointPose);

            DrawLine(RenderCommands, LineStart, LineEnd, vec4(1.f, 0.f, 1.f, 1.f), 2.f);
        }
    }
}

inline vec3
GetModelHalfSize(model *Model, transform Transform)
{
    vec3 Result = Transform.Scale * GetAABBHalfSize(Model->Bounds);
    return Result;
}

internal void
RenderBoundingBox(render_commands *RenderCommands, game_state *State, game_entity *Entity)
{
    model *Cube = GetModelAsset(&State->Assets, "Cube");

    if (Entity->Body)
    {
        // Rigid Body
        vec3 HalfSize = Entity->Body->HalfSize;
        vec3 Position = Entity->Transform.Translation;

        transform Transform = CreateTransform(Position, HalfSize, quat(0.f));
        material Material = CreateMaterial(MaterialType_Unlit, vec4(1.f, 0.f, 0.f, 1.f), true);

        DrawModel(RenderCommands, Cube, Transform, Material);
    }
    else
    {
        // Mesh Bounds
        vec3 HalfSize = GetModelHalfSize(Entity->Model, Entity->Transform);
        vec3 Position = Entity->Transform.Translation;

        transform Transform = CreateTransform(Position, HalfSize, quat(0.f));
        material Material = CreateMaterial(MaterialType_Unlit, vec4(0.f, 1.f, 1.f, 1.f), true);

        DrawModel(RenderCommands, Cube, Transform, Material);
    }
}

internal void
RenderEntity(render_commands *RenderCommands, game_state *State, game_entity *Entity)
{
    if (Entity->Model->Skeleton->JointCount > 1)
    {
        DrawSkinnedModel(RenderCommands, Entity->Model, Entity->Model->Pose);
    }
    else
    {
        DrawModel(RenderCommands, Entity->Model, Entity->Transform);
    }

    if (Entity->DebugView)
    {
        RenderBoundingBox(RenderCommands, State, Entity);

        if (Entity->Model->Skeleton->JointCount > 1)
        {
            DrawSkeleton(RenderCommands, State, Entity->Model->Pose);
        }
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

        if (Entity->DebugView)
        {
            RenderBoundingBox(RenderCommands, State, Entity);
        }
    }
}

inline void
InitRenderBatch(entity_render_batch *Batch, model *Model, u32 MaxEntityCount, memory_arena *Arena)
{
    CopyString(Model->Name, Batch->Name);
    Batch->Model = Model;
    Batch->EntityCount = 0;
    Batch->MaxEntityCount = 4096;
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

    Batch->EntityCount++;
}

inline game_entity *
CreateGameEntity(game_state *State)
{
    game_entity *Entity = State->Entities + State->EntityCount++;

    return Entity;
}

inline void
AddModelComponent(game_entity *Entity, game_assets *Assets, const char *ModelName)
{
    Entity->Model = GetModelAsset(Assets, ModelName);
}

inline void
AddRigidBodyComponent(game_entity *Entity, vec3 Position, quat Orientation, vec3 HalfSize, memory_arena *Arena)
{
    Entity->Body = PushType(Arena, rigid_body);
    BuildRigidBody(Entity->Body, Position, Orientation, HalfSize);
}

inline void
AddAnimationComponent(game_entity *Entity, const char *Animator, memory_arena *Arena)
{
    Entity->Animation = PushType(Arena, animation_graph);
    BuildAnimationGraph(Entity->Animation, Entity->Model->AnimationGraph, Entity->Model, Animator, Arena);
}

internal void
GenerateRoom(game_state *State, vec3 Origin, vec2 Size, vec3 Scale)
{
    // https://quaternius.itch.io/lowpoly-modular-dungeon-pack
    model *FloorModel = GetModelAsset(&State->Assets, "Floor");
    model *WallModel = GetModelAsset(&State->Assets, "Wall");
    model *Wall90Model = GetModelAsset(&State->Assets, "Wall_90");
    model *ColumnModel = GetModelAsset(&State->Assets, "Column");

    vec3 FloorModelBoundsSize = FloorModel->Bounds.Max - FloorModel->Bounds.Min;
    vec3 WallModelBoundsSize = WallModel->Bounds.Max - WallModel->Bounds.Min;
    vec3 Wall90ModelBoundsSize = Wall90Model->Bounds.Max - Wall90Model->Bounds.Min;
    vec3 ColumnModelBoundsSize = ColumnModel->Bounds.Max - ColumnModel->Bounds.Min;

    // todo: ?
    f32 TileSize = FloorModelBoundsSize.x;

    // todo: odd sizes
    i32 HalfDimX = (i32) (Size.x / 2.f);
    i32 HalfDimY = (i32) (Size.y / 2.f);

    // Floor Tiles
    for (i32 x = -HalfDimX; x < HalfDimX; ++x)
    {
        for (i32 y = -HalfDimY; y < HalfDimY; ++y)
        {
            game_entity *Entity = CreateGameEntity(State);

            vec3 Offset = vec3(FloorModelBoundsSize.x * x + FloorModelBoundsSize.x / 2.f, 0.f, FloorModelBoundsSize.z * y + FloorModelBoundsSize.z / 2.f) * Scale;
            vec3 Position = Origin + Offset;
            quat Orientation = quat(0.f);

            Entity->Transform = CreateTransform(Position, Scale, Orientation);
            Entity->Model = FloorModel;
        }
    }

    // Wall Tiles
    {
        // Top
        for (i32 x = -HalfDimX; x < HalfDimX; ++x)
        {
            // First Level
            {
                game_entity *Entity = CreateGameEntity(State);

                vec3 Offset = vec3(TileSize * x + WallModelBoundsSize.x / 2.f, 0.f, TileSize * -HalfDimY) * Scale;
                vec3 Position = Origin + Offset;
                quat Orientation = quat(0.f);

                Entity->Transform = CreateTransform(Position, Scale, Orientation);
                Entity->Model = WallModel;
            }

            // Second Level
            {
                game_entity *Entity = CreateGameEntity(State);

                vec3 Offset = vec3(TileSize * x + WallModelBoundsSize.x / 2.f, WallModelBoundsSize.y, TileSize * -HalfDimY) * Scale;
                vec3 Position = Origin + Offset;
                quat Orientation = quat(0.f);

                Entity->Transform = CreateTransform(Position, Scale, Orientation);
                Entity->Model = WallModel;
            }
        }

        // Bottom
        for (i32 x = -HalfDimX; x < HalfDimX; ++x)
        {
            // First Level
            {
                game_entity *Entity = CreateGameEntity(State);

                vec3 Offset = vec3(TileSize * x + WallModelBoundsSize.x / 2.f, 0.f, TileSize * HalfDimY) * Scale;
                vec3 Position = Origin + Offset;
                quat Orientation = quat(0.f);

                Entity->Transform = CreateTransform(Position, Scale, Orientation);
                Entity->Model = WallModel;
            }

            // Second Level
            {
                game_entity *Entity = CreateGameEntity(State);

                vec3 Offset = vec3(TileSize * x + WallModelBoundsSize.x / 2.f, WallModelBoundsSize.y, TileSize * HalfDimY) * Scale;
                vec3 Position = Origin + Offset;
                quat Orientation = quat(0.f);

                Entity->Transform = CreateTransform(Position, Scale, Orientation);
                Entity->Model = WallModel;
            }
        }

        // Left
        for (i32 y = -HalfDimY; y < HalfDimY; ++y)
        {
            // First Level
            {
                game_entity *Entity = CreateGameEntity(State);

                vec3 Offset = vec3(TileSize * -HalfDimX, 0.f, TileSize * y + Wall90ModelBoundsSize.z / 2.f) * Scale;
                vec3 Position = Origin + Offset;
                quat Orientation = quat(0.f);

                Entity->Transform = CreateTransform(Position, Scale, Orientation);
                Entity->Model = Wall90Model;
            }

            // Second Level
            {
                game_entity *Entity = CreateGameEntity(State);

                vec3 Offset = vec3(TileSize * -HalfDimX, Wall90ModelBoundsSize.y, TileSize * y + Wall90ModelBoundsSize.z / 2.f) * Scale;
                vec3 Position = Origin + Offset;
                quat Orientation = quat(0.f);

                Entity->Transform = CreateTransform(Position, Scale, Orientation);
                Entity->Model = Wall90Model;
            }
        }

        // Right
        for (i32 y = -HalfDimY; y < HalfDimY; ++y)
        {
            // First Level
            {
                game_entity *Entity = CreateGameEntity(State);

                vec3 Offset = vec3(TileSize * HalfDimX, 0.f, TileSize * y + Wall90ModelBoundsSize.z / 2.f) * Scale;
                vec3 Position = Origin + Offset;
                quat Orientation = quat(0.f);

                Entity->Transform = CreateTransform(Position, Scale, Orientation);
                Entity->Model = Wall90Model;
            }

            // Second Level
            {
                game_entity *Entity = CreateGameEntity(State);

                vec3 Offset = vec3(TileSize * HalfDimX, Wall90ModelBoundsSize.y, TileSize * y + Wall90ModelBoundsSize.z / 2.f) * Scale;
                vec3 Position = Origin + Offset;
                quat Orientation = quat(0.f);

                Entity->Transform = CreateTransform(Position, Scale, Orientation);
                Entity->Model = Wall90Model;
            }
        }
    }

    // Columns
    {
        // Top-Left
        {
            game_entity *Entity = State->Entities + State->EntityCount++;

            vec3 Offset = vec3(-HalfDimX * TileSize, 0.f, -HalfDimY * TileSize) * Scale;
            vec3 Position = Origin + Offset;
            quat Orientation = quat(0.f);

            Entity->Transform = CreateTransform(Position, Scale, Orientation);
            Entity->Model = ColumnModel;
        }

        // Top-Right
        {
            game_entity *Entity = State->Entities + State->EntityCount++;

            vec3 Offset = vec3(HalfDimX * TileSize, 0.f, -HalfDimY * TileSize) * Scale;
            vec3 Position = Origin + Offset;
            quat Orientation = quat(0.f);

            Entity->Transform = CreateTransform(Position, Scale, Orientation);
            Entity->Model = ColumnModel;
        }

        // Bottom-Left
        {
            game_entity *Entity = State->Entities + State->EntityCount++;

            vec3 Offset = vec3(-HalfDimX * TileSize, 0.f, HalfDimY * TileSize) * Scale;
            vec3 Position = Origin + Offset;
            quat Orientation = quat(0.f);

            Entity->Transform = CreateTransform(Position, Scale, Orientation);
            Entity->Model = ColumnModel;
        }

        // Bottom-Right
        {
            game_entity *Entity = State->Entities + State->EntityCount++;

            vec3 Offset = vec3(HalfDimX * TileSize, 0.f, HalfDimY * TileSize) * Scale;
            vec3 Position = Origin + Offset;
            quat Orientation = quat(0.f);

            Entity->Transform = CreateTransform(Position, Scale, Orientation);
            Entity->Model = ColumnModel;
        }
    }
}

internal void
GenerateDungeon(game_state *State, vec3 Origin, u32 RoomCount, vec3 Scale)
{
    Assert(RoomCount > 0);

#if 0
    u32 RoomWidth = RandomBetween(&State->RNG, 6, 12);
    u32 RoomHeight = RandomBetween(&State->RNG, 6, 12);
#else
    u32 RoomWidth = 8;
    u32 RoomHeight = 8;
#endif

    vec2 RoomSize = vec2((f32)RoomWidth, (f32)RoomHeight);
    vec3 RoomOrigin = Origin;

    GenerateRoom(State, RoomOrigin, RoomSize, Scale);

    vec2 PrevRoomSize = RoomSize;
    vec3 PrevRoomOrigin = RoomOrigin;
    u32 PrevDirection = -1;

    // todo: fix overlapping rooms
    for (u32 RoomIndex = 1; RoomIndex < RoomCount; ++RoomIndex)
    {
#if 0
        u32 RoomWidth = RandomBetween(&State->RNG, 6, 12);
        u32 RoomHeight = RandomBetween(&State->RNG, 6, 12);
#else
        u32 RoomWidth = 8;
        u32 RoomHeight = 6;
#endif

        vec2 RoomSize = vec2((f32)RoomWidth, (f32)RoomHeight);
        vec3 RoomOrigin = PrevRoomOrigin;

        u32 Direction = RandomChoice(&State->Entropy, 4);

        Assert(Direction < 4);

        if (Direction == PrevDirection)
        {
            //Direction = 2;
        }

        switch (Direction)
        {
            case 0:
            {
                RoomOrigin.z += (PrevRoomSize.y + RoomSize.y) * Scale.z;
                break;
            }
            case 1:
            {
                RoomOrigin.x += (PrevRoomSize.x + RoomSize.x) * Scale.x;
                break;
            }
            case 2:
            {
                RoomOrigin.z -= (PrevRoomSize.y + RoomSize.y) * Scale.z;
                break;
            }
            case 3:
            {
                RoomOrigin.x -= (PrevRoomSize.x + RoomSize.x) * Scale.x;
                break;
            }
        }

        GenerateRoom(State, RoomOrigin, RoomSize, Scale);

        PrevRoomSize = RoomSize;
        PrevRoomOrigin = RoomOrigin;
        PrevDirection = Direction;
    }
}

inline animator_params
GameInput2AnimatorParams(game_state *State, game_entity *Entity)
{
    animator_params Result = {};

    game_input *Input = &State->Input;
    b32 Random = Random01(&State->Entropy) > 0.5f;

    // todo:
    Result.MaxTime = 5.f;
    Result.Move = Clamp(Magnitude(State->CurrentMove), 0.f, 1.f);
    Result.MoveMagnitude = State->Mode == GameMode_World ? Clamp(Magnitude(Input->Move.Range), 0.f, 1.f) : 0.f;
    Result.ToStateActionIdle = Entity->FutureControllable;
    Result.ToStateStandingIdle = !Entity->FutureControllable;
    Result.ToStateActionIdleFromDancing = Input->Dance.IsActivated;
    Result.ToStateDancing = Input->Dance.IsActivated;
    Result.ToStateIdle = Input->Dance.IsActivated;
    Result.ToStateActionIdle1 = Random;
    Result.ToStateActionIdle2 = !Random;
    Result.ToStateIdle1 = Random;
    Result.ToStateIdle2 = !Random;

    return Result;
}

inline animator_params
GameLogic2AnimatorParams(game_state *State, game_entity *Entity)
{
    animator_params Result = {};

    // todo:
    Result.ToStateActionIdle = Entity->FutureControllable;
    Result.ToStateStandingIdle = !Entity->FutureControllable;

    return Result;
}

internal void
AnimateEntity(game_state *State, game_entity *Entity, f32 Delta)
{
    Assert(Entity->Animation);

    animator_params Params = {};

    if (Entity->Controllable)
    {
        Params = GameInput2AnimatorParams(State, Entity);
    }
    else
    {
        Params = GameLogic2AnimatorParams(State, Entity);
    }

    skeleton_pose *Pose = Entity->Model->Pose;
    joint_pose *Root = GetRootLocalJointPose(Pose);
    joint_pose *Hips = GetRootTranslationLocalJointPose(Pose);

    AnimatorPerFrameUpdate(&State->Animator, Entity->Animation, Params, Delta);
    AnimationGraphPerFrameUpdate(Entity->Animation, Delta);
    CalculateSkeletonPose(Entity->Animation, Pose, &State->PermanentArena);

    // Root Motion
    {
        Entity->AccRootMotion.x += Pose->RootMotion.x;
        Entity->AccRootMotion.z += Pose->RootMotion.z;
    }

    Root->Translation = Entity->Transform.Translation;
    Root->Rotation = Entity->Transform.Rotation;
    Root->Scale = Entity->Transform.Scale;

    UpdateGlobalJointPoses(Pose);
}

DLLExport GAME_INIT(GameInit)
{
    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;

    umm PermanentArenaSize = Memory->PermanentStorageSize - sizeof(game_state);
    u8 *PermanentArenaBase = (u8 *)Memory->PermanentStorage + sizeof(game_state);
    InitMemoryArena(&State->PermanentArena, PermanentArenaBase, PermanentArenaSize);

    umm TransientArenaSize = Memory->TransientStorageSize;
    u8 *TransientArenaBase = (u8 *)Memory->TransientStorage;
    InitMemoryArena(&State->TransientArena, TransientArenaBase, TransientArenaSize);

    game_process *Sentinel = &State->ProcessSentinel;
    CopyString("Sentinel", Sentinel->Name);
    Sentinel->Next = Sentinel->Prev = Sentinel;

    State->MaxProcessCount = 31;
    State->Processes = PushArray(&State->PermanentArena, State->MaxProcessCount, game_process);

    // Animator Setup
    State->Animator = {};
    State->Animator.ControllerCount = 31;
    State->Animator.Controllers = PushArray(&State->PermanentArena, State->Animator.ControllerCount, animator_controller);

    // todo: this breaks live code reloading (and StartGameProcess too)
    animator_controller *PelegriniController = HashTableLookup(State->Animator.ControllerCount, State->Animator.Controllers, (char *)"Pelegrini");
    PelegriniController->Func = PelegriniAnimatorController;

    animator_controller *BotController = HashTableLookup(State->Animator.ControllerCount, State->Animator.Controllers, (char *)"Bot");
    BotController->Func = BotAnimatorController;
    //

    State->DelayTime = 0.f;
    State->DelayDuration = 0.5f;

    State->Mode = GameMode_World;
    Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);

    InitCamera(&State->FreeCamera, RADIANS(-30.f), RADIANS(-90.f), RADIANS(45.f), 0.1f, 1000.f, vec3(0.f, 16.f, 32.f));
    InitCamera(&State->PlayerCamera, RADIANS(20.f), RADIANS(0.f), RADIANS(45.f), 0.1f, 1000.f, vec3(0.f, 0.f, 0.f));
    // todo:
    State->PlayerCamera.Radius = 16.f;

    State->Ground = ComputePlane(vec3(-1.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f), vec3(1.f, 0.f, 0.f));
    State->BackgroundColor = vec3(0.f, 0.f, 0.f);
    State->DirectionalLight = {};
    State->DirectionalLight.Color = vec3(1.f);
    State->DirectionalLight.Direction = Normalize(vec3(0.4f, -0.8f, -0.4f));

    State->Entropy = RandomSequence(42);

    ClearRenderCommands(Memory);
    render_commands *RenderCommands = GetRenderCommands(Memory);
    InitRenderer(RenderCommands);

    InitGameAssets(&State->Assets, Platform, RenderCommands, &State->PermanentArena);

    State->CurrentMove = vec2(0.f);
    State->TargetMove = vec2(0.f);

    State->EntityCount = 0;
    State->MaxEntityCount = 4096;
    State->Entities = PushArray(&State->PermanentArena, State->MaxEntityCount, game_entity);

    // Pelegrini
    State->Pelegrini = CreateGameEntity(State);
    State->Pelegrini->Transform = CreateTransform(vec3(0.f, 0.f, 0.f), vec3(3.f), quat(0.f, 0.f, 0.f, 1.f));
    AddModelComponent(State->Pelegrini, &State->Assets, "Pelegrini");
    AddRigidBodyComponent(State->Pelegrini, vec3(0.f, 0.f, 0.f), quat(0.f, 0.f, 0.f, 1.f), vec3(1.f, 3.f, 1.f), &State->PermanentArena);
    AddAnimationComponent(State->Pelegrini, "Bot", &State->PermanentArena);

    // xBot
    State->xBot = CreateGameEntity(State);
    State->xBot->Transform = CreateTransform(vec3(6.f, 0.f, 0.f), vec3(3.f), quat(0.f, 0.f, 0.f, 1.f));
    AddModelComponent(State->xBot, &State->Assets, "xBot");
    AddRigidBodyComponent(State->xBot, vec3(6.f, 0.f, 0.f), quat(0.f, 0.f, 0.f, 1.f), vec3(1.f, 3.f, 1.f), &State->PermanentArena);
    AddAnimationComponent(State->xBot, "Bot", &State->PermanentArena);

    // yBot
    State->yBot = CreateGameEntity(State);
    State->yBot->Transform = CreateTransform(vec3(-6.f, 0.f, 0.f), vec3(3.f), quat(0.f, 0.f, 0.f, 1.f));
    AddModelComponent(State->yBot, &State->Assets, "yBot");
    AddRigidBodyComponent(State->yBot, vec3(-6.f, 0.f, 0.f), quat(0.f, 0.f, 0.f, 1.f), vec3(1.f, 3.f, 1.f), &State->PermanentArena);
    AddAnimationComponent(State->yBot, "Bot", &State->PermanentArena);

    // Skull 0
    State->Skulls[0] = CreateGameEntity(State);
    State->Skulls[0]->Transform = CreateTransform(vec3(0.f), vec3(1.f), quat(0.f));
    State->Skulls[0]->Model = GetModelAsset(&State->Assets, "Skull");

    // Skull 1
    State->Skulls[1] = CreateGameEntity(State);
    State->Skulls[1]->Transform = CreateTransform(vec3(0.f), vec3(1.f), quat(0.f));
    State->Skulls[1]->Model = GetModelAsset(&State->Assets, "Skull");

    // Dummy
    /*State->Dummy = CreateGameEntity(State);
    AddRigidBodyComponent(State->Dummy, vec3(0.f), quat(0.f, 0.f, 0.f, 1.f), vec3(1.f), &State->PermanentArena);*/

    // Player
    //State->Player = State->xBot;
    //State->Player = State->yBot;
    State->Player = State->Pelegrini;
    //State->Player = State->Dummy;

    State->Player->FutureControllable = true;
#if 0
    // todo: create GenerateDungeon function wich takes care of generation multiple connected rooms
    GenerateRoom(State, vec3(0.f), vec2(16.f, 10.f), vec3(2.f));
    GenerateRoom(State, vec3(0.f, 0.f, -36.f), vec2(8.f, 8.f), vec3(2.f));
    GenerateRoom(State, vec3(0.f, 0.f, 48.f), vec2(8.f, 14.f), vec3(2.f));
#else
    //GenerateDungeon(State, vec3(0.f), 12, vec3(2.f));
#endif

    State->PointLightCount = 2;
    State->PointLights = PushArray(&State->PermanentArena, State->PointLightCount, point_light);

    {
        point_light *PointLight = State->PointLights + 0;
        vec4 PointLight1Color = vec4(1.f, 1.f, 0.f, 1.f);
        
        PointLight->Position = vec3(0.f);
        PointLight->Color = PointLight1Color.rgb;
        PointLight->Attenuation.Constant = 1.f;
        PointLight->Attenuation.Linear = 0.09f;
        PointLight->Attenuation.Quadratic = 0.032f;
    }

    {
        point_light *PointLight = State->PointLights + 1;
        vec4 PointLight2Color = vec4(1.f, 0.f, 1.f, 1.f);

        PointLight->Position = vec3(0.f);
        PointLight->Color = PointLight2Color.rgb;
        PointLight->Attenuation.Constant = 1.f;
        PointLight->Attenuation.Linear = 0.09f;
        PointLight->Attenuation.Quadratic = 0.032f;
    }
}

inline game_entity *
GetPrevHero(game_state *State)
{
    game_entity *Result = State->Dummy;

    if (State->Player == State->Pelegrini) Result = State->xBot;
    if (State->Player == State->yBot) Result = State->Pelegrini;
    if (State->Player == State->xBot) Result = State->yBot;

    return Result;
}

inline game_entity *
GetNextHero(game_state *State)
{
    game_entity *Result = State->Dummy;

    if (State->Player == State->Pelegrini) Result = State->yBot;
    if (State->Player == State->yBot) Result = State->xBot;
    if (State->Player == State->xBot) Result = State->Pelegrini;

    return Result;
}

DLLExport GAME_PROCESS_INPUT(GameProcessInput)
{
    game_state *State = GetGameState(Memory);
    platform_api *Platform = Memory->Platform;

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

    if (Input->ChoosePrevHero.IsActivated)
    {
        State->Player->FutureControllable = false;
        State->Player->Body->Acceleration = vec3(0.f);

        State->Player = GetPrevHero(State);

        State->Player->FutureControllable = true;
    }

    if (Input->ChooseNextHero.IsActivated)
    {
        State->Player->FutureControllable = false;
        State->Player->Body->Acceleration = vec3(0.f);

        State->Player = GetNextHero(State);

        State->Player->FutureControllable = true;
    }

    if (Input->LeftClick.IsActivated)
    {
        ray Ray = ScreenPointToWorldRay(Input->MouseCoords, vec2((f32)Parameters->WindowWidth, (f32)Parameters->WindowHeight), &State->FreeCamera);

        f32 MinDistance = F32_MAX;

        game_entity *SelectedEntity = 0;

        for (u32 EntityIndex = 0; EntityIndex < State->EntityCount; ++EntityIndex)
        {
            game_entity *Entity = State->Entities + EntityIndex;

            if (Entity->Model)
            {
                Entity->DebugView = false;

                vec3 HalfSize = Entity->Body 
                    ? Entity->Body->HalfSize
                    : GetModelHalfSize(Entity->Model, Entity->Transform);

                vec3 Center = Entity->Transform.Translation + vec3(0.f, HalfSize.y, 0.f);
                aabb Box = CreateAABBCenterHalfSize(Center, HalfSize);

                vec3 IntersectionPoint;
                if (HitBoundingBox(Ray, Box, IntersectionPoint))
                {
                    f32 Distance = Magnitude(IntersectionPoint - State->FreeCamera.Position);

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
            SelectedEntity->DebugView = true;
        }
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
            // Camera
            // https://www.gamasutra.com/blogs/YoannPignole/20150928/249412/Third_person_camera_design_with_free_move_zone.php
            game_camera *PlayerCamera = &State->PlayerCamera;

            f32 PlayerCameraSensitivity = 0.01f;

            PlayerCamera->Pitch -= Input->Camera.Range.y * PlayerCameraSensitivity;
            PlayerCamera->Pitch = Clamp(PlayerCamera->Pitch, RADIANS(0.f), RADIANS(89.f));

            PlayerCamera->Yaw += Input->Camera.Range.x * PlayerCameraSensitivity;
            PlayerCamera->Yaw = Mod(PlayerCamera->Yaw, 2 * PI);

            PlayerCamera->Radius -= Input->ZoomDelta * 1.0f;
            PlayerCamera->Radius = Clamp(PlayerCamera->Radius, 10.f, 70.f);

            f32 CameraHeight = Max(0.1f, PlayerCamera->Radius * Sin(PlayerCamera->Pitch));

            vec3 PlayerPosition = State->Player->Transform.Translation;

            if (PlayerPosition != PlayerCamera->PivotPosition)
            {
                f32 Distance = Magnitude(PlayerPosition - PlayerCamera->PivotPosition);
                f32 Duration = LogisticFunction(0.4f, 0.5f, 5.f, Distance);
                
                SetVec3Lerp(&PlayerCamera->PivotPositionLerp, 0.f, Duration, PlayerCamera->PivotPosition, PlayerPosition);
                StartGameProcess(State, CameraPivotPositionLerpProcess);
            }

            PlayerCamera->Position.x = PlayerCamera->PivotPosition.x +
                Sqrt(Square(PlayerCamera->Radius) - Square(CameraHeight)) * Sin(PlayerCamera->Yaw);
            PlayerCamera->Position.y = PlayerCamera->PivotPosition.y + CameraHeight;
            PlayerCamera->Position.z = PlayerCamera->PivotPosition.z -
                Sqrt(Square(PlayerCamera->Radius) - Square(CameraHeight)) * Cos(PlayerCamera->Yaw);

            // todo:
            vec3 CameraLookAtPoint = PlayerCamera->PivotPosition + vec3(0.f, 4.f, 0.f);
            PlayerCamera->Direction = Normalize(CameraLookAtPoint - State->PlayerCamera.Position);
            
            /*
*               todo:
                Rotate the characters towards their desired lookat vector, and then take the speed of the character + 
                their relative angle between their velocity and their current forward vector to feed into a 360 motion locomotion blendspace.
            */

            game_entity *Player = State->Player;

            // todo:
            if (Player->Controllable)
            {
                vec3 yMoveAxis = Normalize(Projection(State->PlayerCamera.Direction, State->Ground));
                vec3 xMoveAxis = Normalize(Orthogonal(yMoveAxis, State->Ground));

                f32 xMoveY = Dot(yMoveAxis, xAxis) * Move.y;
                f32 zMoveY = Dot(yMoveAxis, zAxis) * Move.y;

                f32 xMoveX = Dot(xMoveAxis, xAxis) * Move.x;
                f32 zMoveX = Dot(xMoveAxis, zAxis) * Move.x;

                vec3 PlayerDirection = vec3(Dot(vec3(Move.x, 0.f, Move.y), xMoveAxis), 0.f, Dot(vec3(Move.x, 0.f, Move.y), yMoveAxis));

                State->TargetMove = Move;

                //Player->Body->Acceleration.x = (xMoveX + xMoveY) * 120.f;
                //Player->Body->Acceleration.z = (zMoveX + zMoveY) * 120.f;

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
        case GameMode_Edit:
        {
            game_camera *FreeCamera = &State->FreeCamera;

            f32 FreeCameraSpeed = 30.f;
            f32 FreeCameraSensitivity = 0.02f;

            if (Input->EnableFreeCameraMovement.IsActive)
            {
                Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Navigation);

                FreeCamera->Pitch += Input->Camera.Range.y * FreeCameraSensitivity;
                FreeCamera->Pitch = Clamp(FreeCamera->Pitch, RADIANS(-89.f), RADIANS(89.f));

                FreeCamera->Yaw += Input->Camera.Range.x * FreeCameraSensitivity;
                FreeCamera->Yaw = Mod(FreeCamera->Yaw, 2 * PI);

                FreeCamera->Direction = CalculateDirectionFromEulerAngles(FreeCamera->Pitch, FreeCamera->Yaw);
            }
            else
            {
                Platform->SetMouseMode(Platform->PlatformHandle, MouseMode_Cursor);
            }

            FreeCamera->Position += (
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
    game_state *State = GetGameState(Memory);

    //if (State->Advance)
    {
        State->Advance = false;

        game_entity *Player = State->Player;

        if (Player->Body)
        {
#if 1
            aabb PlayerBox = GetRigidBodyAABB(Player->Body);
#else
            vec3 BoxCenter = Player->Transform.Translation;
            vec3 BoxHalfSize = Player->Transform.Scale * GetAABBHalfSize(Player->MeshBounds);
            aabb PlayerBox = CreateAABBCenterHalfSize(BoxCenter, BoxHalfSize);
#endif

            for (u32 EntityIndex = 0; EntityIndex < State->EntityCount; ++EntityIndex)
            {
                game_entity *Entity = State->Entities + EntityIndex;
                rigid_body *Body = Entity->Body;

                if (Body)
                { 
                    //AddGravityForce(Body, vec3(0.f, -10.f, 0.f));
                    Integrate(Body, Parameters->UpdateRate);

                    aabb BodyAABB = GetRigidBodyAABB(Body);
#if 0
                    // todo: dymamic intersection test
                    if (TestAABBPlane(BodyAABB, State->Ground))
                    {
                        ResolveVelocity(Body, &State->Ground, Parameters->UpdateRate, 0.f);

                        f32 Overlap = GetAABBPlaneMinDistance(BodyAABB, State->Ground);
                        ResolveIntepenetration(Body, &State->Ground, Overlap);
                        Body->Acceleration = vec3(0.f);
                    }
#endif
                    /*for (u32 OtherRigidBodyIndex = RigidBodyIndex + 1; OtherRigidBodyIndex < State->RigidBodiesCount; ++OtherRigidBodyIndex)
                    {
                        rigid_body *OtherBody = State->RigidBodies + OtherRigidBodyIndex;

                        aabb OtherBodyAABB = GetRigidBodyAABB(OtherBody);

                        if (TestAABBAABB(BodyAABB, OtherBodyAABB))
                        {
                            State->BackgroundColor = vec3(1.f, 0.f, 0.f);
                        }
                    }*/
                }
            }
        }
    }

    for (u32 EntityIndex = 0; EntityIndex < State->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = State->Entities + EntityIndex;

        if (Entity->Body)
        {
            vec3 ScaledRootMotion = Entity->AccRootMotion * Entity->Transform.Scale;
            vec3 RotatedScaledRootMotion = Rotate(ScaledRootMotion, Entity->Transform.Rotation);

            Entity->Body->PrevPosition = Entity->Body->Position;
            Entity->Body->Position += RotatedScaledRootMotion;

            Entity->AccRootMotion = vec3(0.f);
        }
    }
}

DLLExport GAME_RENDER(GameRender)
{
    game_state *State = GetGameState(Memory);
    render_commands *RenderCommands = GetRenderCommands(Memory);

    ClearMemoryArena(&State->TransientArena);

    RenderCommands->WindowWidth = Parameters->WindowWidth;
    RenderCommands->WindowHeight = Parameters->WindowHeight;
    RenderCommands->Time = Parameters->Time;

    f32 Lag = Parameters->UpdateLag / Parameters->UpdateRate;

    SetViewport(RenderCommands, 0, 0, Parameters->WindowWidth, Parameters->WindowHeight);
    
    switch (State->Mode)
    {
        case GameMode_World:
        case GameMode_Edit:
        {
            game_camera *Camera = State->Mode == GameMode_World 
                ? &State->PlayerCamera
                : &State->FreeCamera;

            if (State->IsBackgroundHighlighted)
            {
                Clear(RenderCommands, vec4(1.f, 0.f, 1., 1.f));
            }
            else
            {
                Clear(RenderCommands, vec4(State->BackgroundColor, 1.f));
            }

            SetTime(RenderCommands, Parameters->Time);

            game_process *GameProcess = State->ProcessSentinel.Next;

            while (GameProcess->OnUpdatePerFrame)
            {
                GameProcess->OnUpdatePerFrame(State, GameProcess, Parameters->Delta);
                GameProcess = GameProcess->Next;
            }

            f32 Aspect = (f32)Parameters->WindowWidth / (f32)Parameters->WindowHeight;
            SetPerspectiveProjection(RenderCommands, Camera->FovY, Aspect, Camera->NearClipPlane, Camera->FarClipPlane);
            SetCamera(RenderCommands, Camera->Position, Camera->Position + Camera->Direction, Camera->Up);

            // Scene lighting
            SetDirectionalLight(RenderCommands, State->DirectionalLight);

            f32 PointLightRadius = 4.f;
            vec3 PointLight1Position = State->Player->Transform.Translation + vec3(0.f, 3.f, 0.f) +
                vec3(Cos(Parameters->Time * 2.f) * PointLightRadius, 1.f, Sin(Parameters->Time * 2.f) * PointLightRadius);
            vec3 PointLight2Position = State->Player->Transform.Translation + vec3(0.f, 3.f, 0.f) +
                vec3(Cos(Parameters->Time * 2.f - PI) * PointLightRadius, -1.f, Sin(Parameters->Time * 2.f - PI) * PointLightRadius);

            point_light *PointLight1 = State->PointLights + 0;
            PointLight1->Position = PointLight1Position;

            point_light *PointLight2 = State->PointLights + 1;
            PointLight2->Position = PointLight2Position;

            // todo: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
            SetPointLights(RenderCommands, State->PointLightCount, State->PointLights);

            // Flying skulls
            // todo: get entity by id ?
            if (State->Player->Body)
            {
                {
                    game_entity *Skull = State->Skulls[0];

                    Skull->Transform.Translation = PointLight1Position;
                    Skull->Transform.Rotation = State->Player->Body->Orientation;
                }

                {
                    game_entity *Skull = State->Skulls[1];

                    Skull->Transform.Translation = PointLight2Position;
                    Skull->Transform.Rotation = State->Player->Body->Orientation;
                }
            }

            // todo: ???
            f32 InterpolationTime = 0.2f;
            vec2 dMove = (State->TargetMove - State->CurrentMove) / InterpolationTime;
            State->CurrentMove += dMove * Parameters->Delta;

            State->EntityBatchCount = 32;
            State->EntityBatches = PushArray(&State->TransientArena, State->EntityBatchCount, entity_render_batch);

            for (u32 EntityIndex = 0; EntityIndex < State->EntityCount; ++EntityIndex)
            {
                game_entity *Entity = State->Entities + EntityIndex;

                // Rigid bodies movement
                if (Entity->Body)
                {
                    if (Entity->Body->PrevPosition == Entity->Body->Position)
                    {
                        int t = 0;
                    }

                    Entity->Transform.Translation = Lerp(Entity->Body->PrevPosition, Lag, Entity->Body->Position);
                    Entity->Transform.Rotation = Entity->Body->Orientation;
                }

                if (Entity->Animation)
                {
                    AnimateEntity(State, Entity, Parameters->Delta);
                }

                // Grouping entities into render batches
                if (Entity->Model)
                {
                    entity_render_batch *Batch = GetEntityBatch(State, Entity->Model->Name);

                    if (StringEquals(Batch->Name, ""))
                    {
                        InitRenderBatch(Batch, Entity->Model, 256, &State->TransientArena);
                    }

                    AddEntityToRenderBatch(Batch, Entity);
                }

                Entity->Controllable = Entity->FutureControllable;
            }

            // Pushing entities into render buffer
            for (u32 EntityBatchIndex = 0; EntityBatchIndex < State->EntityBatchCount; ++EntityBatchIndex)
            {
                entity_render_batch *Batch = State->EntityBatches + EntityBatchIndex;

                u32 BatchThreshold = 1;

                if (Batch->EntityCount > BatchThreshold)
                {
                    // todo: need to add mesh instanced first :(
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

            DrawGround(RenderCommands);

            break;
        }
        case GameMode_Menu:
        {
            Clear(RenderCommands, vec4(0.f));

            f32 FrustrumWidthInUnits = 20.f;
            f32 PixelsPerUnit = (f32)Parameters->WindowWidth / FrustrumWidthInUnits;
            f32 FrustrumHeightInUnits = (f32)Parameters->WindowHeight / PixelsPerUnit;

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
    }
}
