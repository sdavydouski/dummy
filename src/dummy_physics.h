#pragma once

struct rigid_body
{
    vec3 HalfSize;
    
    // todo: ?
    vec3 PrevPosition;
    vec3 Position;

    vec3 Velocity;
    vec3 Acceleration;

    quat Orientation;
    quat_lerp OrientationLerp;

    vec3 ForceAccumulator;

    f32 InverseMass;
    f32 Damping;
};

inline void
SetRigidBodyMass(rigid_body *Body, f32 Mass)
{
    Assert(Mass > 0.f);

    Body->InverseMass = 1.f / Mass;
}

inline void
SetInfiniteRigidBodyMass(rigid_body *Body)
{
    Body->InverseMass = 0.f;
}

inline f32
GetRigidBodyMass(rigid_body *Body)
{
    f32 Result = 1.f / Body->InverseMass;
    return Result;
}

inline b32
HasFiniteMass(rigid_body *Body)
{
    b32 Result = Body->InverseMass != 0.f;
    return Result;
}

inline aabb
GetRigidBodyAABB(rigid_body *Body)
{
    aabb Result = {};

    Result.Min = vec3(Body->Position - Body->HalfSize);
    Result.Max = vec3(Body->Position + Body->HalfSize);

    return Result;
}

inline void
BuildRigidBody(rigid_body *Body, vec3 Position, quat Orientation, vec3 HalfSize)
{
    Body->PrevPosition = Position;
    Body->Position = Position;
    Body->Orientation = Orientation;
    Body->HalfSize = HalfSize;
    
    // todo: don't use at the moment? (except in Integrate)
    Body->Damping = 0.0001f;
    Body->InverseMass = 1.f / 100.f;
}