#pragma once

struct game_camera
{
    vec3 Position;
    vec3 Velocity;
    vec3 TargetPosition;
    vec3 Direction;
    vec3 Up;

    union
    {
        vec3 SphericalCoords;
        struct
        {
            f32 RadialDistance;
            f32 Azimuth;
            f32 Altitude;
        };
    };

    f32 FieldOfView;
    f32 FocalLength;
    f32 AspectRatio;
    f32 NearClipPlane;
    f32 FarClipPlane;
};
