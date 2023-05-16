#pragma once

struct rigid_body
{
    vec3 Position;
    vec3 Velocity;
    vec3 Acceleration;
    f32 Damping;

    quat Orientation;
    vec3 AngularVelocity;
    vec3 AngularAcceleration;
    f32 AngularDamping;

    f32 InverseMass;

    mat3 InverseInertiaTensorLocal;
    mat3 InverseInertiaTensorWorld;

    vec3 ForceAccumulator;
    vec3 TorqueAccumulator;
    
    // ?
    vec3_lerp PositionLerp;
    quat_lerp OrientationLerp;

    vec3 PrevPosition;
    vec3 PrevVelocity;
    vec3 PrevAcceleration;
};
