#include "dummy.h"

inline void
SetPosition(rigid_body *Body, vec3 Position)
{
    Body->Position = Position;
    Body->PrevPosition = Body->Position;
}

inline void
SetOrientation(rigid_body *Body, quat Orientation)
{
    Body->Orientation = Orientation;
}

inline void
SetVelocity(rigid_body *Body, vec3 Velocity)
{
    Body->Velocity = Velocity;
}

inline void
SetAngularVelocity(rigid_body *Body, vec3 AngularVelocity)
{
    Body->AngularVelocity = AngularVelocity;
}

inline f32
GetMass(rigid_body *Body)
{
    Assert(Body->InverseMass != 0.f);

    f32 Mass = 1.f / Body->InverseMass;
    return Mass;
}

inline void
SetMass(rigid_body *Body, f32 Mass)
{
    Assert(Mass != 0.f);

    Body->InverseMass = 1.f / Mass;
}

inline void
SetInfiniteMass(rigid_body *Body)
{
    Body->InverseMass = 0.f;
}

inline void
SetInertiaTensor(rigid_body *Body, mat3 Tensor)
{
    Body->InverseInertiaTensorLocal = Inverse(Tensor);
}

inline void
SetLinearDamping(rigid_body *Body, f32 LinearDamping)
{
    Body->LinearDamping = LinearDamping;
}

inline void
SetAngularDamping(rigid_body *Body, f32 AngularDamping)
{
    Body->AngularDamping = AngularDamping;
}

inline void
SetCenterOfMass(rigid_body *Body, vec3 CenterOfMass)
{
    Body->CenterOfMassLocal = CenterOfMass;
}

inline mat3
CalculateInverseInertiaWorld(rigid_body *Body)
{
    mat4 InverseInertiaTensorLocalM4 = mat4(Body->InverseInertiaTensorLocal);
    mat4 InertiaTensorLocalM4 = Inverse(InverseInertiaTensorLocalM4);
    mat4 WorldToLocalTransform = Inverse(Body->LocalToWorldTransform);
    mat4 InertiaTensorWorldM4 = Transpose(WorldToLocalTransform) * InertiaTensorLocalM4 * WorldToLocalTransform;

    mat3 Result = mat3(
        InertiaTensorWorldM4[0].xyz,
        InertiaTensorWorldM4[1].xyz,
        InertiaTensorWorldM4[2].xyz
    );

    return Inverse(Result);
}

inline void
CalculateRigidBodyState(rigid_body *Body)
{
    mat4 RotationM = GetRotationMatrix(Body->Orientation);

    Body->CenterOfMassWorld = Body->Position + RotationM * Body->CenterOfMassLocal;
    Body->LocalToWorldTransform = Translate(RotationM, Body->CenterOfMassWorld);
    Body->InverseInertiaTensorWorld = CalculateInverseInertiaWorld(Body);
}

dummy_internal void
Integrate(rigid_body *Body, f32 dt)
{
    Assert(dt > 0.f);

#if 0
    Body->PrevPosition = Body->Position;
    Body->PrevVelocity = Body->Velocity;
    Body->PrevAcceleration = Body->Acceleration;
#endif

    vec3 ForceAcceleration = Body->ForceAccumulator * Body->InverseMass;
    vec3 TotalAcceleration = Body->Acceleration + ForceAcceleration;
    Body->Velocity += TotalAcceleration * dt;
    Body->Velocity *= Power(Body->LinearDamping, dt);
    Body->Position += Body->Velocity * dt;

    Body->AngularAcceleration = Body->InverseInertiaTensorWorld * Body->TorqueAccumulator;
    Body->AngularVelocity += Body->AngularAcceleration * dt;
    Body->AngularVelocity *= Power(Body->AngularDamping, dt);
    Body->Orientation += Body->AngularVelocity * dt;
    Body->Orientation = Normalize(Body->Orientation);

    Body->ForceAccumulator = vec3(0.f);
    Body->TorqueAccumulator = vec3(0.f);
    Body->Acceleration = vec3(0.f);

    CalculateRigidBodyState(Body);
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
    vec3 Point = PointWorld - Body->CenterOfMassWorld;
    vec3 Torque = Cross(Point, Force);

    AddForce(Body, Force);
    AddTorque(Body, Torque);
}

inline void
AddForceAtBodyPoint(rigid_body *Body, vec3 Force, vec3 PointLocal)
{
    vec4 PointWorld = Body->LocalToWorldTransform * vec4(PointLocal, 1.f);
    AddForceAtPoint(Body, Force, PointWorld.xyz);
}
