#include "dummy.h"

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
ChaseCameraPerFrameUpdate(game_camera *Camera, game_input *Input, game_state *State, vec3 TargetPosition, f32 Delta)
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
ChaseCameraSceneCollisions(game_camera *Camera, spatial_hash_grid *Grid, game_entity *Player, memory_arena *Arena)
{
    if (Player)
    {
        scoped_memory ScopedMemory(Arena);

        ray Ray;
        Ray.Origin = Camera->TargetPosition;
        Ray.Direction = -Camera->Direction;

        u32 MaxNearbyEntityCount = 100;
        game_entity **NearbyEntities = PushArray(ScopedMemory.Arena, MaxNearbyEntityCount, game_entity *);
        aabb Bounds = { vec3(-Camera->RadialDistance), vec3(Camera->RadialDistance) };

        u32 NearbyEntityCount = FindNearbyEntities(Grid, Player, Bounds, NearbyEntities, MaxNearbyEntityCount);

        f32 MinDistance = F32_MAX;
        vec3 MinIntersectionPoint = vec3(0.f);

        for (u32 EntityIndex = 0; EntityIndex < NearbyEntityCount; ++EntityIndex)
        {
            game_entity *Entity = NearbyEntities[EntityIndex];

            if (Entity->Collider)
            {
                vec3 IntersectionPoint;
                if (IntersectRayAABB(Ray, GetEntityBounds(Entity), IntersectionPoint))
                {
                    f32 Distance = Magnitude(IntersectionPoint - Camera->TargetPosition);

                    if (Distance < MinDistance)
                    {
                        MinDistance = Distance;
                        MinIntersectionPoint = IntersectionPoint;
                    }
                }
            }
        }

        if (MinDistance <= Camera->RadialDistance)
        {
            Camera->Position = MinIntersectionPoint + Camera->Direction * 0.5f;
        }
    }
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
