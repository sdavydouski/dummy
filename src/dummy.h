#pragma once

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
    vec3 Direction;
    vec3 Up;

    f32 Pitch;
    f32 Yaw;
    f32 FovY;

    f32 Radius;
};

enum entity_state
{
    EntityState_Idle,
    EntityState_Moving,
    EntityState_Dance
};

struct entity
{
    model *Model;
    rigid_body *Body;
    animation_graph *Animation;
};

struct entity_batch
{
    u32 EntityCount;
    u32 MaxEntityCount;
    entity *Entities;
    render_instance *Instances;
};

struct game_player
{
    vec3 Offset;
    entity_state State;

    model *Model;
    rigid_body *RigidBody;

    animation_graph AnimationGraph;
};

// todo: !!!
struct lerper_quat
{
    b32 IsEnabled;
    f32 Duration;
    f32 Time;

    quat From;
    quat To;
    quat *Result;
};

struct lerper_vec2
{
    b32 IsEnabled;
    f32 Duration;
    f32 Time;

    vec2 From;
    vec2 To;
    vec2 *Result;
};

struct game_state
{
    memory_arena WorldArena;

    game_mode Mode;

    game_camera FreeCamera;
    game_camera PlayerCamera;

    u32 RigidBodiesCount;
    rigid_body *RigidBodies;

    plane Ground;
    
    model YBotModel;
    model SkullModel;
    model CubeModel;
    model SphereModel;
    model FloorModel;
    model WallModel;

    game_player Player;
    entity_batch Batch;

    lerper_quat LerperQuat;

    vec2 CurrentMove;
    vec2 TargetMove;

    random_sequence RNG;

    vec3 DirectionalColor;
    vec3 BackgroundColor;

    b32 IsBackgroundHighlighted;
    b32 Advance;

    b32 ShowModel;
    b32 ShowSkeleton;

    vec2 FloorDim;
    u32 InstanceCount;
    render_instance *Instances;

    ray Ray;
};
