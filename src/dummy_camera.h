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

inline void
InitCamera(game_camera *Camera, f32 FieldOfView, f32 AspectRatio, f32 NearClipPlane, f32 FarClipPlane, vec3 Position, vec3 SphericalCoords, vec3 Up = vec3(0.f, 1.f, 0.f))
{
    Camera->Position = Position;
    Camera->SphericalCoords = SphericalCoords;
    Camera->Up = Up;
    Camera->Direction = EulerToDirection(SphericalCoords.y, SphericalCoords.z);

    Camera->FieldOfView = FieldOfView;
    Camera->FocalLength = 1.f / Tan(FieldOfView * 0.5f);
    Camera->AspectRatio = AspectRatio;
    Camera->NearClipPlane = NearClipPlane;
    Camera->FarClipPlane = FarClipPlane;
}

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

    Camera->Azimuth -= Input->Camera.Range.x;
    Camera->Azimuth = Mod(Camera->Azimuth, 2 * PI);

    Camera->Altitude -= Input->Camera.Range.y;
    Camera->Altitude = Clamp(Camera->Altitude, RADIANS(0.f), RADIANS(80.f));

    Camera->TargetPosition = TargetPosition;

    vec3 IdealPosition = Camera->TargetPosition + SphericalToCartesian(Camera->SphericalCoords);
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

    Camera->Direction = EulerToDirection(Camera->Azimuth, Camera->Altitude);

    Camera->Position += (
        Input->Move.Range.x * (Normalize(Cross(Camera->Direction, Camera->Up))) +
        Input->Move.Range.y * Camera->Direction) * CameraSpeed * Delta;
}
