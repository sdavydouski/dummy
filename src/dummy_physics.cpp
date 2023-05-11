#include "dummy.h"

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

inline bool32
HasFiniteMass(rigid_body *Body)
{
    bool32 Result = Body->InverseMass != 0.f;
    return Result;
}

inline void
BuildRigidBody(rigid_body *Body, vec3 Position, quat Orientation)
{
    Body->PrevPosition = Position;
    Body->Position = Position;
    Body->Orientation = Orientation;

    // todo:
    Body->Damping = 0.0001f;
    Body->InverseMass = 1.f / 50.f;
}

inline void
Integrate(rigid_body *Body, f32 dt)
{
    Assert(dt > 0.f);

    Body->Acceleration += Body->ForceAccumulator * Body->InverseMass;
    Body->Velocity += Body->Acceleration * dt;
    Body->Velocity *= Power(Body->Damping, dt);
    Body->Position += Body->Velocity * dt + Body->Acceleration * Square(dt) * 0.5f;
    Body->ForceAccumulator = vec3(0.f);
}

inline void
AddGravityForce(rigid_body *Body, vec3 Gravity)
{
    if (HasFiniteMass(Body))
    {
        Body->ForceAccumulator += Gravity * GetRigidBodyMass(Body);
    }
}
