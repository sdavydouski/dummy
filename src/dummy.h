#pragma once

enum game_mode
{
    GameMode_World,
    GameMode_Menu,
    GameMode_Editor
};

struct game_entity
{
    u32 Id;
    ivec3 GridCellCoords[2];
    transform Transform;

    model *Model;
    skinning_data *Skinning;
    collider *Collider;
    rigid_body *Body;
    animation_graph *Animation;
    point_light *PointLight;
    particle_emitter *ParticleEmitter;

    b32 Visible;
    b32 Controllable;
    b32 Destroyed;
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
    b32 Has;
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
    b32 Has;
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
    b32 Has;
    b32 RootMotionEnabled;
};

inline void
RigidBody2Spec(rigid_body *Body, rigid_body_spec *Spec)
{
    Spec->Has = true;
    Spec->RootMotionEnabled = Body->RootMotionEnabled;
}

struct point_light_spec
{
    b32 Has;
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
    b32 Has;
    u32 ParticleCount;
    vec4 Color;
};

inline void
ParticleEmitter2Spec(particle_emitter *ParticleEmitter, particle_emitter_spec *Spec)
{
    Spec->Has = true;
    Spec->ParticleCount = ParticleEmitter->ParticleCount;
    Spec->Color = ParticleEmitter->Color;
}

struct game_entity_spec
{
    u32 Id; // <-- todo: remove
    transform Transform;
    model_spec ModelSpec;
    collider_spec ColliderSpec;
    rigid_body_spec RigidBodySpec;
    point_light_spec PointLightSpec;
    particle_emitter_spec ParticleEmitterSpec;
};

struct entity_render_batch
{
    char Key[256];
    u32 EntityCount;
    u32 MaxEntityCount;
    game_entity **Entities;
    render_instance *Instances;
    model *Model;
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
    hash_table<audio_clip> AudioClips;

    u32 ModelAssetCount;
    game_asset_model *ModelAssets;

    u32 FontAssetCount;
    game_asset_font *FontAssets;

    u32 AudioClipAssetCount;
    game_asset_audio_clip *AudioClipAssets;

    game_assets_state State;

    memory_arena Arena;
};

struct game_options
{
    b32 ShowCascades;
    b32 ShowBoundingVolumes;
    b32 ShowSkeletons;
    b32 ShowCamera;
    b32 ShowGrid;
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

    b32 IsBackgroundHighlighted;

    // for testing purpuses
    f32 DelayTime;
    f32 DelayDuration;

    game_options Options;
    game_menu_quad MenuQuads[4];

    b32 DanceMode;
    b32 PrevDanceMode;
};
