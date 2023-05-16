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
SetInertiaTensor(rigid_body *Body, mat3 InertiaTensor)
{
    Body->InverseInertiaTensorLocal = Inverse(InertiaTensor);
}

inline mat4
LocalToWorldTransform(rigid_body *Body)
{
    transform T = CreateTransform(Body->Position, vec3(1.f), Body->Orientation);
    mat4 Result = Transform(T);

    return Result;
}

inline mat3
GetInertiaWorld(rigid_body *Body)
{
    mat4 LocalToWorld = LocalToWorldTransform(Body);
    mat4 WorldToLocal = Inverse(LocalToWorld);

    mat4 Foo = mat4(
        vec4(Body->InverseInertiaTensorLocal[0], 0.f),
        vec4(Body->InverseInertiaTensorLocal[1], 0.f),
        vec4(Body->InverseInertiaTensorLocal[2], 0.f),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    mat4 Bar = WorldToLocal * Inverse(Foo) * Transpose(WorldToLocal);

    mat3 Result = mat3(
        Bar[0].xyz,
        Bar[1].xyz,
        Bar[2].xyz
    );

    return Result;
}

inline void
BuildRigidBody(rigid_body *Body, vec3 Position, quat Orientation)
{
    Body->PrevPosition = Position;
    Body->Position = Position;
    Body->Orientation = Orientation;

    // todo:
    f32 Mass = 50.f;
    vec3 Size = vec3(1.f);
    mat3 InertiaTensorLocal = GetCuboidInertiaTensor(Mass, Size);

    Body->Damping = 0.0001f;
    Body->AngularDamping = 0.00001f;
    Body->InverseMass = 1.f / Mass;
    Body->InverseInertiaTensorLocal = Inverse(InertiaTensorLocal);
    Body->InverseInertiaTensorWorld = Inverse(GetInertiaWorld(Body));
}

inline void
Integrate(rigid_body *Body, f32 dt)
{
    Assert(dt > 0.f);

    Body->Acceleration += Body->ForceAccumulator * Body->InverseMass;
    Body->Velocity += Body->Acceleration * dt;
    Body->Velocity *= Power(Body->Damping, dt);
    Body->Position += Body->Velocity * dt; // + Body->Acceleration * Square(dt) * 0.5f;

    Body->AngularAcceleration = Body->InverseInertiaTensorWorld * Body->TorqueAccumulator;
    Body->AngularVelocity += Body->AngularAcceleration * dt;
    Body->AngularVelocity *= Power(Body->AngularDamping, dt);
    Body->Orientation += Body->AngularVelocity * dt;

    Body->ForceAccumulator = vec3(0.f);
    Body->TorqueAccumulator = vec3(0.f);

    Body->InverseInertiaTensorWorld = Inverse(GetInertiaWorld(Body));
}

inline void
AddForce(rigid_body *Body, vec3 Force)
{
    Body->ForceAccumulator += Force;
}

inline void
AddTorque(rigid_body *Body, vec3 Torque)
{
    Body->TorqueAccumulator += Torque;
}

inline void
AddForceAtPoint(rigid_body *Body, vec3 Force, vec3 PointWorld)
{
    // Convert to coordinates relative to center of mass
    vec3 Point = PointWorld - Body->Position;
    vec3 Torque = Cross(Point, Force);

    AddForce(Body, Force);
    AddTorque(Body, Torque);
}

inline void
AddForceAtBodyPoint(rigid_body *Body, vec3 Force, vec3 PointLocal)
{
    mat4 LocalToWorld = LocalToWorldTransform(Body);
    vec4 PointWorld = LocalToWorld * vec4(PointLocal, 1.f);

    AddForceAtPoint(Body, Force, PointWorld.xyz);
}
