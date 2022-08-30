inline aabb
CreateAABBMinMax(vec3 Min, vec3 Max)
{
    aabb Result = {};

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

inline aabb
CreateAABBCenterHalfSize(vec3 Center, vec3 HalfSize)
{
    aabb Result = {};

    Result.Min = Center - HalfSize;
    Result.Max = Center + HalfSize;

    return Result;
}

inline vec3
GetAABBHalfSize(aabb Box)
{
    vec3 Result = (Box.Max - Box.Min) * 0.5f;
    return Result;
}

inline vec3
GetAABBCenter(aabb Box)
{
    vec3 Result = Box.Min + GetAABBHalfSize(Box);
    return Result;
}

internal b32
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

internal b32
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
    if (!TestAxis(xAxis, a.Min.x, a.Max.x, b.Min.x, b.Max.x, &mtvAxis, &mtvDistance))
    {
        return false;
    }

    // [Y Axis]
    if (!TestAxis(yAxis, a.Min.y, a.Max.y, b.Min.y, b.Max.y, &mtvAxis, &mtvDistance))
    {
        return false;
    }

    // [Z Axis]
    if (!TestAxis(zAxis, a.Min.z, a.Max.z, b.Min.z, b.Max.z, &mtvAxis, &mtvDistance))
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

internal b32
TestAABBPlane(aabb Box, plane Plane)
{
    vec3 BoxCenter = (Box.Min + Box.Max) * 0.5f;
    vec3 BoxExtents = Box.Max - BoxCenter;

    // Compute the projection interval radius of AABB onto L(t) = BoxCenter + t * Plane.Normal
    f32 Radius = Dot(BoxExtents, Abs(Plane.Normal));
    // Compute distance of AABB center from plane
    f32 Distance = Dot(Plane.Normal, BoxCenter) - Plane.Distance;
    
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

    return (1);     /* ray hits box */
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
    f32 Distance = Dot(Plane.Normal, BoxCenter) - Plane.Distance;

    f32 Result = Distance - Radius;

    return Result;
}

internal aabb
CalculateAxisAlignedBoundingBox(u32 VertexCount, vec3 *Vertices)
{
    vec3 vMin = Vertices[0];
    vec3 vMax = Vertices[0];

    for (u32 VertexIndex = 1; VertexIndex < VertexCount; ++VertexIndex)
    {
        vec3 *Vertex = Vertices + VertexIndex;

        vMin = Min(vMin, *Vertex);
        vMax = Max(vMax, *Vertex);
    }

    aabb Result = {};

    Result.Min = vMin;
    Result.Max = vMax;

    return Result;
}

internal aabb
CalculateAxisAlignedBoundingBox(model *Model)
{
    aabb Result = {};

    if (Model->MeshCount > 0)
    {
        mesh *FirstMesh = First(Model->Meshes);
        aabb Box = CalculateAxisAlignedBoundingBox(FirstMesh->VertexCount, FirstMesh->Positions);

        vec3 vMin = Box.Min;
        vec3 vMax = Box.Max;

        for (u32 MeshIndex = 1; MeshIndex < Model->MeshCount; ++MeshIndex)
        {
            mesh *Mesh = Model->Meshes + MeshIndex;
            aabb Box = CalculateAxisAlignedBoundingBox(Mesh->VertexCount, Mesh->Positions);

            vMin = Min(vMin, Box.Min);
            vMax = Max(vMax, Box.Max);
        }

        Result.Min = vMin;
        Result.Max = vMax;
    }

    return Result;
}
