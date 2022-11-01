#pragma once

#define GAME_PROCESS_ON_UPDATE(Name) void Name(struct game_state *State, struct game_process *Process, f32 Delta)
typedef GAME_PROCESS_ON_UPDATE(game_process_on_update);

enum game_mode
{
    GameMode_None,
    GameMode_World,
    GameMode_Menu,
    GameMode_Edit
};

struct audio_source
{
    audio_clip *Clip;
};

struct game_entity
{
    u32 Id;
    ivec3 GridCellCoords[2];

    transform Transform;

    model *Model;
    rigid_body *Body;
    animation_graph *Animation;
    skinning_data *Skinning;
    //audio_source *AudioSource;

    b32 Visible;
    b32 Controllable;   // todo: come up with smth better
    b32 FutureControllable;   // todo: come up with smth better
    b32 DebugView;
    vec3 TestColor;
    vec3 DebugColor;
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
    // todo:
    u32 MaxInstanceCount;
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
};

struct game_options
{
    b32 ShowCascades;
    b32 ShowBoundingVolumes;
    b32 ShowSkeletons;
    b32 ShowCamera;
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

    job_queue *JobQueue;

    game_mode PrevMode;
    game_mode Mode;

    game_input Input;

    vec2 ViewFrustrumSize;

    game_camera FreeCamera;
    game_camera PlayerCamera;

    polyhedron Frustrum;

    plane Ground;

    game_assets Assets;
    animator Animator;

    u32 RenderableEntityCount;
    u32 EntityCount;
    u32 MaxEntityCount;
    game_entity *Entities;

    spatial_hash_grid SpatialGrid;

    hash_table<entity_render_batch> EntityBatches;

    u32 PointLightCount;
    point_light *PointLights;

    directional_light DirectionalLight;

    hash_table<game_process> Processes;
    // linked-list (for efficient adding/removal and traversing)
    game_process ProcessSentinel;

    game_entity *Player;

    i32 PlayableEntityIndex;
    union
    {
        struct
        {
            game_entity *Pelegrini;
            game_entity *yBot;
            game_entity *xBot;
            game_entity *Paladin;
            game_entity *Warrok;
            game_entity *Maw;
        };

        game_entity *PlayableEntities[6];
    };

    game_entity *Dummy;

    vec2 CurrentMove;
    vec2 TargetMove;

    random_sequence Entropy;

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
