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

struct game_player
{
    quat Orientation;
    vec3 Offset;
    entity_state State;

    model *Model;
    rigid_body *RigidBody;
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
    model MutantModel;
    model ArissaModel;
    //model NinjaModel;
    model CubeModel;
    model SphereModel;

    game_player Player;

    animation_graph AnimationGraph;

    // todo: continue
    animation_blend_space_1d LocomotionBlendSpace;
    animation_blend_space_2d LocomotionBlendSpace2D;

    lerper_quat LerperQuat;

    vec2 CurrentMove;
    vec2 TargetMove;

    random_sequence RNG;

    vec3 BackgroundColor;

    u32 GridCount;
    b32 IsBackgroundHighlighted;
    b32 Advance;

    b32 ShowModel;
    b32 ShowSkeleton;
};
