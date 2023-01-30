#pragma once

#define MAX_ENTITY_NAME 256

enum game_mode
{
    GameMode_World,
    GameMode_Menu,
    GameMode_Editor
};

struct game_entity
{
    u32 Id;
    char Name[MAX_ENTITY_NAME];
    ivec3 GridCellCoords[2];

    transform Transform;

    model *Model;
    skinning_data *Skinning;
    animation_graph *Animation;
    collider *Collider;
    rigid_body *Body;
    point_light *PointLight;
    particle_emitter *ParticleEmitter;

    bool32 Visible;
    bool32 Destroyed;
    vec3 TestColor;
    vec3 DebugColor;
};

struct world_area
{
    u32 MaxEntityCount;
    u32 EntityCount;
    game_entity *Entities;
    spatial_hash_grid SpatialGrid;

    memory_arena Arena;
};

struct model_spec
{
    bool32 Has;
    char ModelRef[64];
};

inline void
Model2Spec(model *Model, model_spec *Spec)
{
    Spec->Has = true;
    CopyString(Model->Key, Spec->ModelRef);
}

struct collider_spec
{
    bool32 Has;
    collider_type Type;
    union
    {
        vec3 Size;
        f32 Radius;
    };
};

inline void
Collider2Spec(collider *Collider, collider_spec *Spec)
{
    Spec->Has = true;
    Spec->Type = Collider->Type;

    switch (Collider->Type)
    {
        case Collider_Box:
        {
            Spec->Size = Collider->BoxCollider.Size;

            break;
        }
        case Collider_Sphere:
        {
            Spec->Radius = Collider->SphereCollider.Radius;

            break;
        }
        default:
        {
            Assert(!"Not implemented");
        }
    }
}

struct rigid_body_spec
{
    bool32 Has;
    bool32 RootMotionEnabled;
};

inline void
RigidBody2Spec(rigid_body *Body, rigid_body_spec *Spec)
{
    Spec->Has = true;
    Spec->RootMotionEnabled = Body->RootMotionEnabled;
}

struct point_light_spec
{
    bool32 Has;
    vec3 Color;
    light_attenuation Attenuation;
};

inline void
PointLight2Spec(point_light *PointLight, point_light_spec *Spec)
{
    Spec->Has = true;
    Spec->Color = PointLight->Color;
    Spec->Attenuation = PointLight->Attenuation;
}

struct particle_emitter_spec
{
    bool32 Has;
    u32 ParticleCount;
    u32 ParticlesSpawn;
    vec4 Color;
    vec2 Size;
};

inline void
ParticleEmitter2Spec(particle_emitter *ParticleEmitter, particle_emitter_spec *Spec)
{
    Spec->Has = true;
    Spec->ParticleCount = ParticleEmitter->ParticleCount;
    Spec->ParticlesSpawn = ParticleEmitter->ParticlesSpawn;
    Spec->Color = ParticleEmitter->Color;
    Spec->Size = ParticleEmitter->Size;
}

struct game_entity_spec
{
    char Name[MAX_ENTITY_NAME];
    transform Transform;
    model_spec ModelSpec;
    collider_spec ColliderSpec;
    rigid_body_spec RigidBodySpec;
    point_light_spec PointLightSpec;
    particle_emitter_spec ParticleEmitterSpec;
};

#pragma pack(push, 1)

enum game_file_type
{
    GameFile_Area,
    GameFile_Entity
};

struct game_file_header
{
    u32 MagicValue;
    u32 Version;
    u32 EntityCount;
    game_file_type Type;
};

#pragma pack(pop)

struct entity_render_batch
{
    char Key[256];
    u32 EntityCount;
    u32 MaxEntityCount;
    game_entity **Entities;

    model *Model;
    skinning_data *Skinning;

    union
    {
        mesh_instance *MeshInstances;
        skinned_mesh_instance *SkinnedMeshInstances;
    };
};

struct game_process
{
    game_process_on_update *OnUpdatePerFrame;
    game_process *Child;

    char Key[256];
    game_process *Prev;
    game_process *Next;
};

struct game_asset
{
    char Name[64];
    char Path[64];
};

struct game_asset_model
{
    game_asset GameAsset;
    model_asset *ModelAsset;
};

struct game_asset_font
{
    game_asset GameAsset;
    font_asset *FontAsset;
};

struct game_asset_texture
{
    game_asset GameAsset;
    texture_asset *TextureAsset;
};

struct game_asset_audio_clip
{
    game_asset GameAsset;
    audio_clip_asset *AudioAsset;
};

enum game_assets_state
{
    GameAssetsState_Unloaded,
    GameAssetsState_Loaded,
    GameAssetsState_Ready,
};

struct game_assets
{
    hash_table<model> Models;
    hash_table<font> Fonts;
    hash_table<texture> Textures;
    hash_table<audio_clip> AudioClips;

    u32 ModelAssetCount;
    game_asset_model *ModelAssets;

    u32 FontAssetCount;
    game_asset_font *FontAssets;

    u32 TextureAssetCount;
    game_asset_texture *TextureAssets;

    u32 AudioClipAssetCount;
    game_asset_audio_clip *AudioClipAssets;

    game_assets_state State;

    memory_arena Arena;
};

struct game_options
{
    bool32 ShowCascades;
    bool32 ShowBoundingVolumes;
    bool32 ShowSkeletons;
    bool32 ShowCamera;
    bool32 ShowGrid;
};

struct game_menu_quad
{
    vec4 Color;
    u32 Corner;
    f32 Move;
};

struct game_state
{
    memory_arena PermanentArena;
    memory_arena TransientArena;

    world_area WorldArea;

    job_queue *JobQueue;

    game_mode PrevMode;
    game_mode Mode;

    game_input Input;

    vec2 ViewFrustrumSize;

    game_camera GameCamera;
    game_camera EditorCamera;

    polyhedron Frustrum;

    plane Ground;

    game_assets Assets;
    animator Animator;

    u32 NextFreeEntityId;
    u32 NextFreeMeshId;
    u32 NextFreeTextureId;
    u32 NextFreeSkinningId;

    u32 RenderableEntityCount;
    u32 ActiveEntitiesCount;

    game_entity *Player;
    game_entity *SelectedEntity;

    hash_table<entity_render_batch> EntityBatches;

    directional_light DirectionalLight;

    hash_table<game_process> Processes;
    // linked-list (for efficient adding/removal and traversing)
    game_process ProcessSentinel;

    vec2 CurrentMove;
    vec2 TargetMove;

    random_sequence GeneralEntropy;
    random_sequence ParticleEntropy;

    game_event_list EventList;

    f32 MasterVolume;

    vec3 BackgroundColor;

    bool32 IsBackgroundHighlighted;

    // for testing purpuses
    f32 DelayTime;
    f32 DelayDuration;

    game_options Options;
    game_menu_quad MenuQuads[4];

    bool32 DanceMode;
    bool32 PrevDanceMode;
};
