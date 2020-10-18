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

enum player_state
{
    PlayerState_Idle,
    PlayerState_Walking,
    // todo: ?
    PlayerState_IdleToWalking,
    PlayerState_WalkingToIdle
};

struct game_player
{
    vec3 Direction;
    vec3 Offset;
    player_state State;

    model *Model;
    rigid_body *RigidBody;
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
    model CubeModel;
    model LightModel;

    game_player Player;
    animation_state_set PlayerAnimationStateSet;

    // todo: continue
    animation_blend_space_1d LocomotionBlendSpace;
    animation_blend_space_2d LocomotionBlendSpace2D;

    vec2 Blend;

    f32 IdleToWalkingClock;
    f32 WalkingToIdleClock;
    // todo: temp
    //skeleton_pose PoseA;
    //skeleton_pose PoseB;
    //animation_clip *BlendAnimation;
    //f32 CurrentTimeB;
    //f32 BlendFactor;

    f32 Angle;

    vec3 BackgroundColor;

    u32 GridCount;
    b32 IsBackgroundHighlighted;
    b32 Advance;
};
