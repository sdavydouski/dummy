#pragma once

struct aabb
{
    vec3 Min;
    vec3 Max;
};

struct plane
{
    vec3 Normal;
    f32 d;
};

inline plane
ComputePlane(vec3 a, vec3 b, vec3 c)
{
    plane Result;
    Result.Normal = Normalize(Cross(b - a, c - a));
    Result.d = Dot(Result.Normal, a);

    return Result;
}

inline aabb
CreateAABB(vec3 Min, vec3 Max)
{
    aabb Result = {};

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

inline b32
TestAABBAABB(aabb a, aabb b)
{
    // Exit with no intersection if separated along an axis
    if (a.Max.x < b.Min.x || a.Min.x > b.Max.x) return false;
    if (a.Max.y < b.Min.y || a.Min.y > b.Max.y) return false;
    if (a.Max.z < b.Min.z || a.Min.z > b.Max.z) return false;

    // Overlapping on all axes means AABBs are intersecting
    return true;
}

// Intersect AABBs ‘a’ and ‘b’ moving with constant velocities.
// On intersection, return time of first and last contact in tFirst and tLast
internal b32
IntersectMovingAABBAABB(aabb a, aabb b, vec3 VelocityA, vec3 VelocityB, f32 *tFirst, f32 *tLast)
{
    // Exit early if ‘a’ and ‘b’ initially overlapping
    if (TestAABBAABB(a, b))
    {
        *tFirst = 0.f;
        *tLast = 0.f;
        return true;
    }

    // Use relative velocity; effectively treating ’a’ as stationary
    vec3 RelativeVelocity = VelocityB - VelocityA;

    // Initialize times of first and last contact
    *tFirst = 0.f;
    *tLast = 1.f;

    // For each axis, determine times of first and last contact, if any
    for (u32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
    {
        if (RelativeVelocity[AxisIndex] < 0.f)
        {
            // Nonintersecting and moving apart
            if (b.Max[AxisIndex] < a.Min[AxisIndex])
            {
                return false;
            }

            if (a.Max[AxisIndex] < b.Min[AxisIndex])
            {
                *tFirst = Max((a.Max[AxisIndex] - b.Min[AxisIndex]) / RelativeVelocity[AxisIndex], *tFirst);
            }

            if (b.Max[AxisIndex] > a.Min[AxisIndex])
            {
                *tLast = Min((a.Min[AxisIndex] - b.Max[AxisIndex]) / RelativeVelocity[AxisIndex], *tLast);
            }
        }
        else if (RelativeVelocity[AxisIndex] > 0.f)
        {
            // Nonintersecting and moving apart
            if (b.Min[AxisIndex] > a.Max[AxisIndex])
            {
                return false;
            }

            if (b.Max[AxisIndex] < a.Min[AxisIndex])
            {
                *tFirst = Max((a.Min[AxisIndex] - b.Max[AxisIndex]) / RelativeVelocity[AxisIndex], *tFirst);
            }

            if (a.Max[AxisIndex] > b.Min[AxisIndex])
            {
                *tLast = Min((a.Max[AxisIndex] - b.Min[AxisIndex]) / RelativeVelocity[AxisIndex], *tLast);
            }
        }

        // No overlap possible if time of first contact occurs after time of last contact
        if (*tFirst > * tLast)
        {
            return false;
        }
    }

    return true;
}

internal b32
TestAABBPlane(aabb Box, plane Plane)
{
    vec3 BoxCenter = (Box.Min + Box.Max) * 0.5f;
    vec3 BoxExtents = Box.Max - BoxCenter;

    // Compute the projection interval radius of AABB onto L(t) = BoxCenter + t * Plane.Normal
    f32 Radius = BoxExtents.x * Abs(Plane.Normal.x) + BoxExtents.y * Abs(Plane.Normal.y) + BoxExtents.z * Abs(Plane.Normal.z);
    // Compute distance of AABB center from plane
    f32 Distance = Dot(Plane.Normal, BoxCenter) - Plane.d;
    
    // Intersection occurs when distance falls within [-Radius, +Radius] interval
    b32 Result = Abs(Distance) <= Radius;
    
    return Result;
}

// todo:
internal f32
GetAABBAABBMinDistance(aabb a, aabb b)
{
    vec3 Overlap;

    if (a.Min > b.Min)
    {
        Overlap = b.Max - a.Min;
    }
    else
    {
        Overlap = b.Max - a.Max;
    }

    // todo: ?
    f32 Result = Abs(Min(Min(Overlap.x, Overlap.y), Min(Overlap.y, Overlap.z)));

    //Assert(Result >= 0.f);

    return Result;
}

internal f32
GetAABBPlaneMinDistance(aabb Box, plane Plane)
{
    vec3 BoxCenter = (Box.Min + Box.Max) * 0.5f;
    vec3 BoxExtents = Box.Max - BoxCenter;

    f32 Radius = BoxExtents.x * Abs(Plane.Normal.x) + BoxExtents.y * Abs(Plane.Normal.y) + BoxExtents.z * Abs(Plane.Normal.z);
    f32 Distance = Dot(Plane.Normal, BoxCenter) - Plane.d;

    f32 Result = Distance - Radius;

    return Result;
}

inline b32
Contains(aabb Volume, aabb Box)
{
    b32 Result = Volume.Min <= Box.Min && Box.Max <= Volume.Max;
    return Result;
}
