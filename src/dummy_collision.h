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
    if (a.Max.x < b.Min.x || a.Min.x > b.Max.x)
    {
        return false;
    }
    if (a.Max.y < b.Min.y || a.Min.y > b.Max.y)
    {
        return false;
    }
    if (a.Max.z < b.Min.z || a.Min.z > b.Max.z)
    {
        return false;
    }

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

inline void
Swap(f32 &a, f32 &b)
{
    f32 Temp = a;
    a = b;
    b = Temp;
}

internal b32
IntersectRayAABB(ray Ray, aabb Box, f32 *tMin, vec3 *IntersectionPoint)
{
    *tMin = 0.f;
    f32 tMax = F32_MAX;

    vec3 p = Ray.Origin;
    vec3 d = Ray.Direction;

    // For all three slabs
    for (u32 i = 0; i < 3; ++i)
    {
        if (Abs(d[i]) < EPSILON)
        {
            // Ray is parallel to slab. No hit if origin not within slab
            if (p[i] < Box.Min[i] || p[i] > Box.Max[i])
            {
                return false;
            }
        } 
        else
        {
            // Compute intersection t value of ray with near and far plane of slab
            f32 ood = 1.f / d[i];
            f32 t1 = (Box.Min[i] - p[i]) * ood;
            f32 t2 = (Box.Max[i] - p[i]) * ood;

            // Make t1 be intersection with near plane, t2 with far plane
            if (t1 > t2)
            {
                Swap(t1, t2);
            }

            // Compute the intersection of slab intersection intervals
            if (t1 > *tMin)
            {
                *tMin = t1;
            }

            if (t2 > tMax)
            {
                tMax = t2;
            }

            // Exit with no collision as soon as slab intersection becomes empty
            if (*tMin > tMax)
            {
                return false;
            }
        }
    }

    // Ray intersects all 3 slabs. Return point and intersection t value
    *IntersectionPoint = p + d * (*tMin);

    return true;
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
