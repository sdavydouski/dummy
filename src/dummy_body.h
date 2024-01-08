#pragma once

// todo:
constexpr f32 SleepEpsilon = 0.4f;

struct rigid_body
{
    vec3 Position;
    vec3 Velocity;
    f32 LinearDamping;

    quat Orientation;
    vec3 AngularVelocity;
    f32 AngularDamping;

    f32 InverseMass;

    mat3 InverseInertiaTensorLocal;
    mat3 InverseInertiaTensorWorld;

    vec3 ForceAccumulator;
    vec3 TorqueAccumulator;

    vec3 Acceleration;
    vec3 AngularAcceleration;

    vec3 CenterOfMassLocal;
    vec3 CenterOfMassWorld;
    
    mat4 LocalToWorldTransform;

    vec3 PrevPosition;
    vec3 PrevVelocity;
    vec3 PrevAcceleration;

    f32 Motion;
    bool32 IsAwake;
    bool32 CanSleep;
};
