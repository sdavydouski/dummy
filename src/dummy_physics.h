#pragma once

struct rigid_body
{
    vec3 Position;
    vec3 Velocity;
    vec3 AngularVelocity;
    vec3 Acceleration;

    quat Orientation;

    f32 InverseMass;
    mat3 InverseInertiaTensor;

    f32 Damping;
    f32 AngularDamping;

    vec3 ForceAccumulator;
    
    // ?
    vec3_lerp PositionLerp;
    quat_lerp OrientationLerp;

    vec3 PrevPosition;
    vec3 PrevVelocity;
    vec3 PrevAcceleration;
};
