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

/*
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/

#define NUMDIM	3
#define RIGHT	0
#define LEFT	1
#define MIDDLE	2

b32 HitBoundingBox(ray Ray, aabb Box, vec3 &Coord)
{
    vec3 origin = Ray.Origin;
    vec3 dir = Ray.Direction;
    vec3 minB = Box.Min;
    vec3 maxB = Box.Max;

    b32 inside = 1;
    i32 whichPlane;
    f32 maxT[NUMDIM];
    f32 candidatePlane[NUMDIM];
    char quadrant[NUMDIM];

    /* Find candidate planes; this loop can be avoided if
    rays cast all from the eye(assume perpsective view) */
    for (u32 i = 0; i < NUMDIM; i++)
    {
        if (origin[i] < minB[i])
        {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = 0;
        }
        else if (origin[i] > maxB[i])
        {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = 0;
        }
        else
        {
            quadrant[i] = MIDDLE;
        }
    }

    /* Ray origin inside bounding box */
    if (inside)
    {
        Coord = origin;
        return (1);
    }


    /* Calculate T distances to candidate planes */
    for (u32 i = 0; i < NUMDIM; i++)
    {
        if (quadrant[i] != MIDDLE && dir[i] != 0.)
        {
            maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
        }
        else
        {
            maxT[i] = -1.;
        }
    }

    /* Get largest of the maxT's for final choice of intersection */
    whichPlane = 0;
    for (u32 i = 1; i < NUMDIM; i++)
    {
        if (maxT[whichPlane] < maxT[i])
        {
            whichPlane = i;
        }
    }

    /* Check final candidate actually inside box */
    if (maxT[whichPlane] < 0.)
    {
        return (0);
    }

    for (u32 i = 0; i < NUMDIM; i++)
    {
        if (whichPlane != i) 
        {
            Coord[i] = origin[i] + maxT[whichPlane] * dir[i];

            if (Coord[i] < minB[i] || Coord[i] > maxB[i])
            {
                return (0);
            }
        }
        else 
        {
            Coord[i] = candidatePlane[i];
        }
    }

    return (1);				/* ray hits box */
}

// todo: wrong!
internal b32
IntersectRayAABB(ray Ray, aabb Box, vec3 *IntersectionPoint)
{
    f32 tMin = 0.f;
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
            if (t1 > tMin)
            {
                tMin = t1;
            }

            if (t2 > tMax)
            {
                tMax = t2;
            }

            // Exit with no collision as soon as slab intersection becomes empty
            if (tMin > tMax)
            {
                return false;
            }
        }
    }

    // Ray intersects all 3 slabs
    *IntersectionPoint = p + d * tMin;

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
