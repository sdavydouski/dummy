#pragma once

struct bot_animator_params
{
    vec3 PrevVelocity;
    vec3 Velocity;

    f32 TargetMoveMagnitude;
    f32 CurrentMoveMagnitude;

    bool32 IsGrounded;
    bool32 IsPlayer;
    bool32 IsDanceMode;

    f32 MaxActionIdleTime;
    f32 ActionIdleRandomTransition;
};

struct paladin_animator_params
{
    f32 TargetMoveMagnitude;
    f32 CurrentMoveMagnitude;

    bool32 IsDanceMode;

    f32 MaxActionIdleTime;
    f32 ActionIdleRandomTransition;

    bool32 LightAttack;
    bool32 StrongAttack;
};

struct monstar_animator_params
{
    f32 TargetMoveMagnitude;
    f32 CurrentMoveMagnitude;

    bool32 IsDanceMode;
    bool32 Attack;
};
