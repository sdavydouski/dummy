inline bounds
CreateAABBMinMax(vec3 Min, vec3 Max)
{
    bounds Result = {};

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

inline bounds
CreateAABBCenterHalfSize(vec3 Center, vec3 HalfSize)
{
    bounds Result = {};

    Result.Min = Center - HalfSize;
    Result.Max = Center + HalfSize;

    return Result;
}

inline vec3
GetAABBHalfSize(bounds Box)
{
    vec3 Result = (Box.Max - Box.Min) * 0.5f;
    return Result;
}

inline vec3
GetAABBCenter(bounds Box)
{
    vec3 Result = Box.Min + GetAABBHalfSize(Box);
    return Result;
}

inline bounds
Union(bounds a, bounds b)
{
    bounds Result;

    Result.Min = Min(a.Min, b.Min);
    Result.Max = Max(a.Max, b.Max);

    return Result;
}

internal bounds
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

    bounds Result = {};

    Result.Min = vMin;
    Result.Max = vMax;

    return Result;
}

inline bounds
ScaleBounds(bounds Bounds, vec3 Scale)
{
    bounds Result = {};

    vec3 ScaledHalfSize = (Bounds.Max - Bounds.Min) * Scale / 2.f;
    // todo:
    vec3 Center = vec3(0.f, ScaledHalfSize.y, 0.f);

    Result.Min = Center - ScaledHalfSize;
    Result.Max = Center + ScaledHalfSize;

    return Result;
}

inline bounds
TranslateBounds(bounds Bounds, vec3 Translation)
{
    bounds Result = {};

    Result.Min = Bounds.Min + Translation;
    Result.Max = Bounds.Max + Translation;

    return Result;
}

inline bounds
UpdateBounds(bounds Bounds, transform T)
{
    bounds Result = {};

    vec3 Translation = T.Translation;
    mat4 M = Transform(T);

    for (u32 Axis = 0; Axis < 3; ++Axis)
    {
        Result.Min[Axis] = Result.Max[Axis] = Translation[Axis];

        for (u32 Element = 0; Element < 3; ++Element)
        {
            f32 e = M[Axis][Element] * Bounds.Min[Element];
            f32 f = M[Axis][Element] * Bounds.Max[Element];

            if (e < f)
            {
                Result.Min[Axis] += e;
                Result.Max[Axis] += f;
            }
            else
            {
                Result.Min[Axis] += f;
                Result.Max[Axis] += e;
            }
        }
    }

    return Result;
}

inline bounds
GetColliderBounds(collider *Collider)
{
    bounds Result = {};

    switch (Collider->Type)
    {
        case Collider_Box:
        {
            box_collider Box = Collider->BoxCollider;
            vec3 HalfSize = Box.Size / 2;

            Result.Min = Box.Center - HalfSize;
            Result.Max = Box.Center + HalfSize;

            break;
        }
        case Collider_Sphere:
        {
            sphere_collider Sphere = Collider->SphereCollider;

            Result.Min = Sphere.Center - vec3(Sphere.Radius);
            Result.Max = Sphere.Center + vec3(Sphere.Radius);

            break;
        }
    }

    return Result;
}

inline void
UpdateColliderPosition(collider *Collider, vec3 BasePosition)
{
    switch (Collider->Type)
    {
        case Collider_Box:
        {
            box_collider *BoxCollider = &Collider->BoxCollider;
            BoxCollider->Center = BasePosition + vec3(0.f, BoxCollider->Size.y / 2.f, 0.f);

            break;
        }
        case Collider_Sphere:
        {
            sphere_collider *SphereCollider = &Collider->SphereCollider;
            SphereCollider->Center = BasePosition + vec3(0.f, SphereCollider->Radius, 0.f);

            break;
        }
    }
}

internal bounds
GetEntityBounds(game_entity *Entity)
{
    bounds Result = {};

    if (Entity->Collider)
    {
        Result = GetColliderBounds(Entity->Collider);
    }
    else if (Entity->Model)
    {
        Result = UpdateBounds(Entity->Model->Bounds, Entity->Transform);
    }
    else
    {
        bounds DefaultBounds =
        {
            .Min = vec3(-0.1f),
            .Max = vec3(0.1f)
        };
        Result = UpdateBounds(DefaultBounds, Entity->Transform);

        //Result.Min = Entity->Transform.Translation;
        //Result.Max = Entity->Transform.Translation;
    }

    return Result;
}

internal bool32
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

internal bool32
TestAABBAABB(bounds a, bounds b, vec3 *mtv)
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

inline bool32
TestAABBAABB(bounds a, bounds b)
{
    // Exit with no intersection if separated along an axis
    if (a.Max.x < b.Min.x || a.Min.x > b.Max.x) return false;
    if (a.Max.y < b.Min.y || a.Min.y > b.Max.y) return false;
    if (a.Max.z < b.Min.z || a.Min.z > b.Max.z) return false;

    // Overlapping on all axes means AABBs are intersecting
    return true;
}

internal bool32
TestAABBPlane(bounds Box, plane Plane)
{
    vec3 BoxCenter = (Box.Min + Box.Max) * 0.5f;
    vec3 BoxExtents = Box.Max - BoxCenter;

    // Compute the projection interval radius of AABB onto L(t) = BoxCenter + t * Plane.Normal
    f32 Radius = Dot(BoxExtents, Abs(Plane.Normal));
    // Compute distance of AABB center from plane
    f32 Distance = Dot(Plane.Normal, BoxCenter) - Plane.Distance;
    
    // Intersection occurs when distance falls within [-Radius, +Radius] interval
    bool32 Result = Abs(Distance) <= Radius;
    
    return Result;
}

/*
    Fast Ray-Box Intersection
    by Andrew Woo
    from "Graphics Gems", Academic Press, 1990
*/
bool32 IntersectRayAABB(ray Ray, bounds Box, vec3 &Coord)
{
    vec3 RayOrigin = Ray.Origin;
    vec3 RayDirection = Ray.Direction;
    vec3 BoxMin = Box.Min;
    vec3 BoxMax = Box.Max;

    constexpr i32 Right = 0;
    constexpr i32 Left = 1;
    constexpr i32 Middle = 2;

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

internal f32
GetAABBPlaneMinDistance(bounds Box, plane Plane)
{
    vec3 BoxCenter = (Box.Min + Box.Max) * 0.5f;
    vec3 BoxExtents = Box.Max - BoxCenter;

    f32 Radius = Dot(BoxExtents, Abs(Plane.Normal));
    f32 Distance = Dot(Plane.Normal, BoxCenter) - Plane.Distance;

    f32 Result = Distance - Radius;

    return Result;
}

internal bool32
TestColliders(collider *a, collider *b, vec3 *mtv)
{
    if (a->Type == Collider_Box && b->Type == Collider_Box)
    {
        return TestAABBAABB(GetColliderBounds(a), GetColliderBounds(b), mtv);
    }
    else
    {
        Assert(!"Not implemented");
    }

    return false;
}
