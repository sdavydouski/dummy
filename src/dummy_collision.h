#pragma once

struct aabb
{
    vec3 Min;
    vec3 Max;
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

internal b32
TestAABBPlane(aabb Box, plane Plane)
{
    vec3 BoxCenter = (Box.Min + Box.Max) * 0.5f;
    vec3 BoxExtents = Box.Max - BoxCenter;

    // Compute the projection interval radius of AABB onto L(t) = BoxCenter + t * Plane.Normal
    f32 Radius = Dot(BoxExtents, Abs(Plane.Normal));
    // Compute distance of AABB center from plane
    f32 Distance = Dot(Plane.Normal, BoxCenter) - Plane.d;
    
    // Intersection occurs when distance falls within [-Radius, +Radius] interval
    b32 Result = Abs(Distance) <= Radius;
    
    return Result;
}

internal f32
GetAABBPlaneMinDistance(aabb Box, plane Plane)
{
    vec3 BoxCenter = (Box.Min + Box.Max) * 0.5f;
    vec3 BoxExtents = Box.Max - BoxCenter;

    f32 Radius = Dot(BoxExtents, Abs(Plane.Normal));
    f32 Distance = Dot(Plane.Normal, BoxCenter) - Plane.d;

    f32 Result = Distance - Radius;

    return Result;
}
