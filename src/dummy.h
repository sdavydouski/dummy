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

struct game_camera
{
    vec3 Position;
    vec3 Pivot;
    vec3 Direction;
    vec3 Up;

    f32 Pitch;
    f32 Yaw;
    f32 FovY;

    f32 Radius;
    f32 NearClipPlane;
    f32 FarClipPlane;

    vec3_lerp PositionLerp;
};

struct game_entity
{
    transform Transform;

    model *Model;
    rigid_body *Body;
    animation_graph *Animation;

    b32 DebugView;
};

struct entity_render_batch
{
    char Name[256];
    u32 EntityCount;
    u32 MaxEntityCount;
    game_entity **Entities;
    render_instance *Instances;
    model *Model;
};

struct game_process
{
    char Name[256];
    game_process_on_update *OnUpdatePerFrame;
    game_process *Child;

    game_process *Prev;
    game_process *Next;
};

struct game_assets
{
    u32 ModelCount;
    model *Models;
};

struct game_state
{
    memory_arena PermanentArena;
    memory_arena TransientArena;

    game_mode Mode;

    vec2 ViewFrustrumSize;

    game_camera FreeCamera;
    game_camera PlayerCamera;

    plane Ground;

    game_assets Assets;

    game_entity *Player;
    game_entity *xBot;
    game_entity *yBot;

    u32 MaxEntityCount;
    u32 EntityCount;
    game_entity *Entities;

    u32 EntityBatchCount;
    entity_render_batch *EntityBatches;

    u32 PointLightCount;
    point_light *PointLights;

    // linked-list (for efficient adding/removal and traversing)
    game_process ProcessSentinel;

    // hashtable (for storage)
    u32 ProcessCount;
    game_process *Processes;

    vec2 CurrentMove;
    vec2 TargetMove;

    random_sequence RNG;

    vec3 DirectionalColor;
    vec3 BackgroundColor;

    b32 IsBackgroundHighlighted;
    b32 Advance;

    b32 SelectAll;

    // for testing purpuses
    f32 DelayTime;
    f32 DelayDuration;
};
