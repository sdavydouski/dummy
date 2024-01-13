#include "dummy.h"

dummy_internal void
CalculateColliderState(game_entity *Entity)
{
    Assert(Entity->Collider);

    collider *Collider = Entity->Collider;

    vec3 Position = Entity->Body
        ? Entity->Body->Position
        : Entity->Transform.Translation;

    quat Rotation = Entity->Body
        ? Entity->Body->Orientation
        : Entity->Transform.Rotation;

    vec3 Scale = Entity->Transform.Scale;

    switch (Collider->Type)
    {
        case Collider_Box:
        {
            transform T = CreateTransform(Position, Scale, Rotation);
            Collider->Box.Transform = Transform(T) * Collider->Box.Offset;

            aabb BoundsLocal = CreateAABBCenterHalfExtent(vec3(0.f), Collider->Box.HalfSize);
            Collider->Bounds = UpdateBounds(BoundsLocal, Collider->Box.Transform);

            break;
        }
        case Collider_Sphere:
        {
            NotImplemented;
        }
    }
}

dummy_internal aabb
GetEntityBounds(game_entity *Entity)
{
    aabb Result = {};

    if (Entity->Collider)
    {
        Result = Entity->Collider->Bounds;
    }
    else if (Entity->Model)
    {
        Result = UpdateBounds(Entity->Model->Bounds, Entity->Transform);
    }
    else if (Entity->PointLight || Entity->ParticleEmitter || Entity->AudioSource)
    {
        aabb Bounds =
        {
            .Center = vec3(0.f),
            .HalfExtent = vec3(0.1f)
        };

        Result = UpdateBounds(Bounds, Entity->Transform);
    }
    else
    {
        aabb Bounds =
        {
            .Center = vec3(0.f, 0.5f, 0.f),
            .HalfExtent = vec3(0.5f)
        };

        Result = UpdateBounds(Bounds, Entity->Transform);
    }

    return Result;
}

dummy_internal bool32
TestAxis(vec3 Axis, f32 MinA, f32 MaxA, f32 MinB, f32 MaxB, vec3 *mtvAxis, f32 *mtvDistance)
{
    // [Separating Axis Theorem]
    // • Two convex shapes only overlap if they overlap on all axes of separation
    // • In order to create accurate responses we need to find the collision vector (Minimum Translation Vector)
    // • Find if the two boxes intersect along a single axis
    // • Compute the intersection interval for that axis
    // • Keep the smallest intersection/penetration value

    f32 AxisLengthSquared = Dot(Axis, Axis);

    // If the axis is degenerate then ignore
    if (AxisLengthSquared < EPSILON)
    {
        return true;
    }

    // Calculate the two possible overlap ranges
    // Either we overlap on the left or the right sides
    f32 d0 = (MaxB - MinA);   // 'Left' side
    f32 d1 = (MaxA - MinB);   // 'Right' side

    // Intervals do not overlap, so no intersection
    if (d0 <= 0.0f || d1 <= 0.0f)
    {
        return false;
    }

    // Find out if we overlap on the 'right' or 'left' of the object.
    f32 Overlap = (d0 < d1) ? d0 : -d1;

    // The mtd vector for that axis
    vec3 Sep = Axis * (Overlap / AxisLengthSquared);

    // The mtd vector length squared
    f32 SepLengthSquared = Dot(Sep, Sep);

    // If that vector is smaller than our computed Minimum Translation Distance use that vector as our current MTV distance
    if (SepLengthSquared < *mtvDistance)
    {
        *mtvDistance = SepLengthSquared;
        *mtvAxis = Sep;
    }

    return true;
}

dummy_internal bool32
TestAABBAABB(aabb a, aabb b, vec3 *mtv)
{
    // [Minimum Translation Vector]
    f32 mtvDistance = F32_MAX;              // Set current minimum distance (max float value so next value is always less)
    vec3 mtvAxis;                           // Axis along which to travel with the minimum distance

    // [Axes of potential separation]
    vec3 xAxis = vec3(1.f, 0.f, 0.f);
    vec3 yAxis = vec3(0.f, 1.f, 0.f);
    vec3 zAxis = vec3(0.f, 0.f, 1.f);

    // [X Axis]
    if (!TestAxis(xAxis, a.Min().x, a.Max().x, b.Min().x, b.Max().x, &mtvAxis, &mtvDistance))
    {
        return false;
    }

    // [Y Axis]
    if (!TestAxis(yAxis, a.Min().y, a.Max().y, b.Min().y, b.Max().y, &mtvAxis, &mtvDistance))
    {
        return false;
    }

    // [Z Axis]
    if (!TestAxis(zAxis, a.Min().z, a.Max().z, b.Min().z, b.Max().z, &mtvAxis, &mtvDistance))
    {
        return false;
    }

    // Multiply the penetration depth by itself plus a small increment
    // When the penetration is resolved using the MTV, it will no longer intersect
    f32 Penetration = Sqrt(mtvDistance) * 1.001f;

    // Calculate Minimum Translation Vector (MTV) [normal * penetration]
    *mtv = Normalize(mtvAxis) * Penetration;

    return true;
}

inline bool32
TestAABBAABB(aabb a, aabb b)
{
    // Exit with no intersection if separated along an axis
    if (Abs(a.Center[0] - b.Center[0]) > (a.HalfExtent[0] + b.HalfExtent[0])) return false;
    if (Abs(a.Center[1] - b.Center[1]) > (a.HalfExtent[1] + b.HalfExtent[1])) return false;
    if (Abs(a.Center[2] - b.Center[2]) > (a.HalfExtent[2] + b.HalfExtent[2])) return false;

    // Overlapping on all axes means AABBs are intersecting
    return true;
}

dummy_internal bool32
TestAABBPlane(aabb Box, plane Plane)
{
    // Compute the projection interval radius of AABB onto L(t) = BoxCenter + t * Plane.Normal
    f32 Radius = Dot(Box.HalfExtent, Abs(Plane.Normal));
    // Compute distance of AABB center from plane
    f32 Distance = Dot(Plane.Normal, Box.Center) - Plane.Distance;
    
    // Intersection occurs when distance falls within [-Radius, +Radius] interval
    bool32 Result = Abs(Distance) <= Radius;
    
    return Result;
}

dummy_internal bool32
IntersectMovingAABBPlane(aabb Box, plane Plane, vec3 Velocity, f32 *CollisionTime, vec3 *ContactPoint)
{
    // Compute the projection interval radius of AABB onto L(t) = BoxCenter + t * Plane.Normal
    f32 Radius = Dot(Box.HalfExtent, Abs(Plane.Normal));
    // Compute distance of AABB center from plane
    f32 Distance = Dot(Plane.Normal, Box.Center) - Plane.Distance;

    if (Abs(Distance) <= Radius)
    {
        // AABB is already overlapping the plane
        *CollisionTime = 0.f;
        *ContactPoint = Box.Center - Radius * Plane.Normal;

        return true;
    }
    else
    {
        f32 Denom = Dot(Plane.Normal, Velocity);

        if (Denom >= 0.f)
        {
            // No intersection as AABB moving parallel to or away from plane
            return false;
        }
        else
        {
            // AABB is moving towards the plane
            *CollisionTime = (Radius - Distance) / Denom;
            *ContactPoint = Box.Center + Velocity * (*CollisionTime) - Radius * Plane.Normal;

            return true;
        }
    }
}

dummy_internal bool32
IntersectMovingAABBAABB(aabb BoxA, aabb BoxB, vec3 VelocityA, vec3 VelocityB, f32 *tFirst, f32 *tLast)
{
    // Exit early if BoxA and BoxB initially overlapping
    if (TestAABBAABB(BoxA, BoxB))
    {
        *tFirst = 0.f;
        *tLast = 0.f;

        return true;
    }

    // Use relative velocity; effectively treating BoxA as stationary
    vec3 RelativeVelocity = VelocityB - VelocityA;

    // Initialize times of first and last contact
    *tFirst = 0.f;
    *tLast = 1.f;

    vec3 MinA = BoxA.Min();
    vec3 MaxA = BoxA.Max();
    vec3 MinB = BoxB.Min();
    vec3 MaxB = BoxB.Max();

    // For each axis, determine times of first and last contact, if any
    for (u32 Axis = 0; Axis < 3; ++Axis)
    {
        if (RelativeVelocity[Axis] < 0.f)
        {
            if (MaxB[Axis] < MinA[Axis])
            {
                // Nonintersecting and moving apart
                return false;
            }

            if (MaxA[Axis] < MinB[Axis])
            {
                *tFirst = Max((MaxA[Axis] - MinB[Axis]) / RelativeVelocity[Axis], *tFirst);
            }

            if (MaxB[Axis] > MinA[Axis])
            {
                *tLast = Min((MinA[Axis] - MaxB[Axis]) / RelativeVelocity[Axis], *tLast);
            }
        }
        else if (RelativeVelocity[Axis] > 0.f)
        {
            if (MinB[Axis] > MaxA[Axis])
            {
                // Nonintersecting and moving apart
                return false;
            }

            if (MaxB[Axis] < MinA[Axis])
            {
                *tFirst = Max((MinA[Axis] - MaxB[Axis]) / RelativeVelocity[Axis], *tFirst);
            }

            if (MaxA[Axis] > MinB[Axis])
            {
                *tLast = Min((MaxA[Axis] - MinB[Axis]) / RelativeVelocity[Axis], *tLast);
            }
        }

        if (*tFirst > *tLast)
        {
            // No overlap possible if time of first contact occurs after time of last contact
            return false;
        }
    }

    return true;
}

/*
    Fast Ray-Box Intersection
    by Andrew Woo
    from "Graphics Gems", Academic Press, 1990
*/
bool32 IntersectRayAABB(ray Ray, aabb Box, vec3 &Coord)
{
    vec3 RayOrigin = Ray.Origin;
    vec3 RayDirection = Ray.Direction;
    vec3 BoxMin = Box.Min();
    vec3 BoxMax = Box.Max();

    i32 Right = 0;
    i32 Left = 1;
    i32 Middle = 2;

    bool32 IsInside = true;
    i32 WhichPlane;
    f32 MaxT[3];
    f32 CandidatePlanes[3];
    u8 Quadrant[3];

    /* Find candidate planes; this loop can be avoided if
       rays cast all from the eye(assume perpsective view) */
    for (u32 i = 0; i < 3; i++)
    {
        if (RayOrigin[i] < BoxMin[i])
        {
            Quadrant[i] = Left;
            CandidatePlanes[i] = BoxMin[i];
            IsInside = false;
        }
        else if (RayOrigin[i] > BoxMax[i])
        {
            Quadrant[i] = Right;
            CandidatePlanes[i] = BoxMax[i];
            IsInside = false;
        }
        else
        {
            Quadrant[i] = Middle;
        }
    }

    /* Ray origin inside bounding box */
    if (IsInside)
    {
        Coord = RayOrigin;
        return true;
    }

    /* Calculate T distances to candidate planes */
    for (u32 i = 0; i < 3; i++)
    {
        if (Quadrant[i] != Middle && RayDirection[i] != 0.f)
        {
            MaxT[i] = (CandidatePlanes[i] - RayOrigin[i]) / RayDirection[i];
        }
        else
        {
            MaxT[i] = -1.f;
        }
    }

    /* Get largest of the MaxT's for final choice of intersection */
    WhichPlane = 0;
    for (u32 i = 1; i < 3; i++)
    {
        if (MaxT[WhichPlane] < MaxT[i])
        {
            WhichPlane = i;
        }
    }

    /* Check final candidate actually inside box */
    if (MaxT[WhichPlane] < 0.f)
    {
        return false;
    }

    for (u32 i = 0; i < 3; i++)
    {
        if (WhichPlane != i)
        {
            Coord[i] = RayOrigin[i] + MaxT[WhichPlane] * RayDirection[i];

            if (Coord[i] < BoxMin[i] || Coord[i] > BoxMax[i])
            {
                return false;
            }
        }
        else 
        {
            Coord[i] = CandidatePlanes[i];
        }
    }

    /* Ray hits box */
    return true;     
}

dummy_internal f32
GetAABBPlaneMinDistance(aabb Box, plane Plane)
{
    f32 Radius = Dot(Box.HalfExtent, Abs(Plane.Normal));
    f32 Distance = Dot(Plane.Normal, Box.Center) - Plane.Distance;

    f32 Result = Distance - Radius;

    return Result;
}

dummy_internal bool32
TestColliders(game_entity *a, game_entity *b, vec3 *mtv)
{
    Assert(a->Collider);
    Assert(b->Collider);

    if (a->Collider->Type == Collider_Box && b->Collider->Type == Collider_Box)
    {
        aabb BoxA = a->Collider->Bounds;
        aabb BoxB = b->Collider->Bounds;

        return TestAABBAABB(BoxA, BoxB, mtv);
    }
    else
    {
        Assert(!"Not implemented");
    }

    return false;
}

inline void
CalculateVertices(collider_box *Box, vec3 *Vertices)
{
    f32 Signs[] = { -1.f, 1.f };
    vec3 BoxHalfSize = Box->HalfSize * GetScale(Box->Transform);

    for (u32 x = 0; x < 2; ++x)
    {
        for (u32 y = 0; y < 2; ++y)
        {
            for (u32 z = 0; z < 2; ++z)
            {
                *Vertices++ =
                    GetTranslation(Box->Transform) +
                    Signs[x] * GetAxis(Box->Transform, 0) * BoxHalfSize.x +
                    Signs[y] * GetAxis(Box->Transform, 1) * BoxHalfSize.y +
                    Signs[z] * GetAxis(Box->Transform, 2) * BoxHalfSize.z;
            }
        }
    }
}

inline f32
TransformToAxis(collider_box *Box, vec3 Axis)
{
    vec3 BoxHalfSize = Box->HalfSize * GetScale(Box->Transform);

    f32 Result =
        BoxHalfSize.x * Abs(Dot(Axis, GetAxis(Box->Transform, 0))) +
        BoxHalfSize.y * Abs(Dot(Axis, GetAxis(Box->Transform, 1))) +
        BoxHalfSize.z * Abs(Dot(Axis, GetAxis(Box->Transform, 2)));

    return Result;
}

inline bool32
OverlapOnAxis(collider_box *One, collider_box *Two, vec3 Axis)
{
    // Make sure we have a normalized axis, and don't check almost parallel axes
    if (SquaredMagnitude(Axis) < EPSILON)
    {
        return true;
    }

    vec3 ToCenter = GetTranslation(Two->Transform) - GetTranslation(One->Transform);

    // Project the half-sizes of One and Two onto Axis
    f32 ProjectionOne = TransformToAxis(One, Axis);
    f32 ProjectionTwo = TransformToAxis(Two, Axis);

    // Project this onto the axis
    f32 Distance = Abs(Dot(ToCenter, Axis));

    // Check for overlap
    bool32 Result = (Distance < ProjectionOne + ProjectionTwo);

    return Result;
}

dummy_internal bool32
TestBoxPlane(collider_box *Box, plane Plane)
{
    vec3 BoxHalfSize = Box->HalfSize * GetScale(Box->Transform);

    // Compute the projected radius of the box onto the plane direction
    f32 Radius =
        BoxHalfSize.x * Abs(Dot(Plane.Normal, GetAxis(Box->Transform, 0))) +
        BoxHalfSize.y * Abs(Dot(Plane.Normal, GetAxis(Box->Transform, 1))) +
        BoxHalfSize.z * Abs(Dot(Plane.Normal, GetAxis(Box->Transform, 2)));

    // Compute how far the box is from the origin
    f32 Distance = Dot(Plane.Normal, GetTranslation(Box->Transform)) - Radius;

    // Check for the intersection
    return Distance <= Plane.Distance;
}

dummy_internal bool32
TestBoxBox(collider_box *One, collider_box *Two)
{
    vec3 xAxisA = GetAxis(One->Transform, 0);
    vec3 yAxisA = GetAxis(One->Transform, 1);
    vec3 zAxisA = GetAxis(One->Transform, 2);

    vec3 xAxisB = GetAxis(Two->Transform, 0);
    vec3 yAxisB = GetAxis(Two->Transform, 1);
    vec3 zAxisB = GetAxis(Two->Transform, 2);

    bool32 Result = (
        OverlapOnAxis(One, Two, xAxisA) &&
        OverlapOnAxis(One, Two, yAxisA) &&
        OverlapOnAxis(One, Two, zAxisA) &&

        OverlapOnAxis(One, Two, xAxisB) &&
        OverlapOnAxis(One, Two, yAxisB) &&
        OverlapOnAxis(One, Two, zAxisB) &&

        OverlapOnAxis(One, Two, Cross(xAxisA, xAxisB)) &&
        OverlapOnAxis(One, Two, Cross(xAxisA, yAxisB)) &&
        OverlapOnAxis(One, Two, Cross(xAxisA, zAxisB)) &&
        OverlapOnAxis(One, Two, Cross(yAxisA, xAxisB)) &&
        OverlapOnAxis(One, Two, Cross(yAxisA, yAxisB)) &&
        OverlapOnAxis(One, Two, Cross(yAxisA, zAxisB)) &&
        OverlapOnAxis(One, Two, Cross(zAxisA, xAxisB)) &&
        OverlapOnAxis(One, Two, Cross(zAxisA, yAxisB)) &&
        OverlapOnAxis(One, Two, Cross(zAxisA, zAxisB))
    );

    return Result;
}
