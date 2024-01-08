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

inline void
SetIsAwake(rigid_body *Body, bool32 IsAwake)
{
    Body->IsAwake = IsAwake;

    if (Body->IsAwake)
    {
        // Add a bit of motion to avoid it falling asleep immediately
        Body->Motion = SleepEpsilon * 2.f;
    }
    else
    {
        Body->Velocity = vec3(0.f);
        Body->AngularVelocity = vec3(0.f);
    }
}

inline void
SetCanSleep(rigid_body *Body, bool32 CanSleep)
{
    Body->CanSleep = CanSleep;

    if (!Body->CanSleep && !Body->IsAwake)
    {
        SetIsAwake(Body, true);
    }
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

    if (Body->IsAwake)
    {
        Body->PrevPosition = Body->Position;
        Body->PrevVelocity = Body->Velocity;
        Body->PrevAcceleration = Body->Acceleration;

        Body->Acceleration = Body->ForceAccumulator * Body->InverseMass;
        Body->Velocity += Body->Acceleration * dt;
        Body->Velocity *= Power(Body->LinearDamping, dt);
        Body->Position += Body->Velocity * dt;

        Body->AngularAcceleration = Body->InverseInertiaTensorWorld * Body->TorqueAccumulator;
        Body->AngularVelocity += Body->AngularAcceleration * dt;
        Body->AngularVelocity *= Power(Body->AngularDamping, dt);
        Body->Orientation += Body->AngularVelocity * dt;
        Body->Orientation = Normalize(Body->Orientation);

        Body->ForceAccumulator = vec3(0.f);
        Body->TorqueAccumulator = vec3(0.f);

        CalculateRigidBodyState(Body);

        if (Body->CanSleep)
        {
            f32 CurrentMotion = SquaredMagnitude(Body->Velocity) + SquaredMagnitude(Body->AngularVelocity);
            f32 Bias = Power(0.5f, dt);

            Body->Motion = Lerp(CurrentMotion, Bias, Body->Motion);
            Body->Motion = Min(Body->Motion, SleepEpsilon * 10.f);

            if (Body->Motion < SleepEpsilon)
            {
                SetIsAwake(Body, false);
            }
        }
    }
}

inline void
AddForce(rigid_body *Body, vec3 Force)
{
    Body->ForceAccumulator += Force;
    Body->IsAwake = true;
}

inline void
AddTorque(rigid_body *Body, vec3 Torque)
{
    Body->TorqueAccumulator += Torque;
    Body->IsAwake = true;
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
