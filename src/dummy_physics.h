#pragma once

struct rigid_body
{
    // todo: should be the position of body's center of mass
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
    
    mat4 LocalToWorldTransform;

    // ?
    vec3 PrevPosition;
    vec3 PrevVelocity;
    vec3 PrevAcceleration;

    quat PrevOrientation;
    vec3 PrevAngularVelocity;
    vec3 PrevAngularAcceleration;
    //
};
