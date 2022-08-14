#pragma once

struct rigid_body
{
    vec3 HalfSize;
    
    vec3 PrevPosition;
    vec3 Position;

    vec3 Velocity;
    vec3 Acceleration;

    quat Orientation;
    quat_lerp OrientationLerp;

    vec3 ForceAccumulator;

    f32 InverseMass;
    f32 Damping;
    b32 RootMotionEnabled;
};
