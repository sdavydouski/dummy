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

inline mat4
GetCameraTransform(game_camera *Camera)
{
    mat4 Result = LookAt(Camera->Position, Camera->Position + Camera->Direction, Camera->Up);
    return Result;
}

dummy_internal void
ChaseCameraPerFrameUpdate(game_camera *Camera, game_input *Input, vec3 TargetPosition, f32 Delta)
{
    Camera->RadialDistance -= Input->ZoomDelta;
    Camera->RadialDistance = Clamp(Camera->RadialDistance, 4.f, 16.f);

    Camera->Azimuth += Input->Camera.Range.x;
    Camera->Azimuth = Mod(Camera->Azimuth, 2 * PI);

    Camera->Altitude -= Input->Camera.Range.y;
    Camera->Altitude = Clamp(Camera->Altitude, RADIANS(0.f), RADIANS(80.f));

    Camera->TargetPosition = TargetPosition;

    // todo:
#if 0
    vec3 IdealPosition = Camera->TargetPosition + Spherical2Cartesian(Camera->SphericalCoords);
#else
    f32 CameraHeight = Max(0.1f, Camera->SphericalCoords.x * Sin(Camera->SphericalCoords.z));
    vec3 Coords = vec3
    (
        Sqrt(Square(Camera->SphericalCoords.x) - Square(CameraHeight)) * Sin(Camera->SphericalCoords.y),
        CameraHeight,
        -Sqrt(Square(Camera->SphericalCoords.x) - Square(CameraHeight)) * Cos(Camera->SphericalCoords.y)
    );

    vec3 IdealPosition = Camera->TargetPosition + Coords;
#endif
    vec3 Displace = Camera->Position - IdealPosition;

    // Damped spring system
    f32 SpringC = 256.f;
    f32 DampingC = 32.f;

    // Ensure we have critically damped system
    f32 DampingRatio = DampingC / (2 * Sqrt(SpringC));
    Assert(DampingRatio == 1.f);

    vec3 SpringAcceleration = -(SpringC * Displace) - (DampingC * Camera->Velocity);

    Camera->Velocity += SpringAcceleration * Delta;
    Camera->Position += Camera->Velocity * Delta;
    Camera->Direction = Normalize(Camera->TargetPosition - Camera->Position);
}

dummy_internal void
FreeCameraPerFrameUpdate(game_camera *Camera, game_input *Input, f32 Delta)
{
    f32 CameraSpeed = 10.f;

    Camera->Azimuth += Input->Camera.Range.x;
    Camera->Azimuth = Mod(Camera->Azimuth, 2 * PI);

    Camera->Altitude += Input->Camera.Range.y;
    Camera->Altitude = Clamp(Camera->Altitude, RADIANS(-89.f), RADIANS(89.f));

    Camera->Direction = Euler2Direction(Camera->Azimuth, Camera->Altitude);

    Camera->Position += (
        Input->Move.Range.x * (Normalize(Cross(Camera->Direction, Camera->Up))) +
        Input->Move.Range.y * Camera->Direction) * CameraSpeed * Delta;
}
