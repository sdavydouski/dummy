#include "dummy.h"

dummy_internal void
CalculateColliderInternalState(game_entity *Entity)
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
CalculateVertices(collider_box Box, vec3 *Vertices)
{
    f32 Signs[] = { -1.f, 1.f };
    vec3 BoxHalfSize = Box.HalfSize * GetScale(Box.Transform);

    for (u32 x = 0; x < 2; ++x)
    {
        for (u32 y = 0; y < 2; ++y)
        {
            for (u32 z = 0; z < 2; ++z)
            {
                *Vertices++ =
                    GetTranslation(Box.Transform) +
                    Signs[x] * GetAxis(Box.Transform, 0) * BoxHalfSize.x +
                    Signs[y] * GetAxis(Box.Transform, 1) * BoxHalfSize.y +
                    Signs[z] * GetAxis(Box.Transform, 2) * BoxHalfSize.z;
            }
        }
    }
}

dummy_internal bool32 
TestBoxPlane(collider_box Box, plane Plane)
{
    vec3 BoxHalfSize = Box.HalfSize * GetScale(Box.Transform);

    // Compute the projected radius of the box onto the plane direction
    f32 Radius =
        BoxHalfSize.x * Abs(Dot(Plane.Normal, GetAxis(Box.Transform, 0))) +
        BoxHalfSize.y * Abs(Dot(Plane.Normal, GetAxis(Box.Transform, 1))) +
        BoxHalfSize.z * Abs(Dot(Plane.Normal, GetAxis(Box.Transform, 2)));

    // Compute how far the box is from the origin
    f32 Distance = Dot(Plane.Normal, GetTranslation(Box.Transform)) - Radius;

    // Check for the intersection
    return Distance <= Plane.Distance;
}

inline f32
TransformToAxis(collider_box Box, vec3 Axis)
{
    vec3 BoxHalfSize = Box.HalfSize * GetScale(Box.Transform);

    f32 Result =
        BoxHalfSize.x * Abs(Dot(Axis, GetAxis(Box.Transform, 0))) +
        BoxHalfSize.y * Abs(Dot(Axis, GetAxis(Box.Transform, 1))) +
        BoxHalfSize.z * Abs(Dot(Axis, GetAxis(Box.Transform, 2)));

    return Result;
}

inline bool32
OverlapOnAxis(collider_box a, collider_box b, vec3 Axis)
{
    // Make sure we have a normalized axis, and don't check almost parallel axes
    if (SquaredMagnitude(Axis) < EPSILON)
    {
        return true;
    }

    vec3 ToCenter = GetTranslation(b.Transform) - GetTranslation(a.Transform);

    // Project the half-sizes of a and b onto Axis
    f32 ProjectionA = TransformToAxis(a, Axis);
    f32 ProjectionB = TransformToAxis(b, Axis);

    // Project this onto the axis
    f32 Distance = Abs(Dot(ToCenter, Axis));

    // Check for overlap
    bool32 Result = (Distance < ProjectionA + ProjectionB);
    return Result;
}

inline f32
PenetrationOnAxis(collider_box a, collider_box b, vec3 Axis)
{
    vec3 ToCenter = GetTranslation(b.Transform) - GetTranslation(a.Transform);

    // Project the half-sizes of a and b onto Axis
    f32 ProjectionA = TransformToAxis(a, Axis);
    f32 ProjectionB = TransformToAxis(b, Axis);

    // Project this onto the axis
    f32 Distance = Abs(Dot(ToCenter, Axis));

    // Return the overlap (i.e. positive indicates
    // overlap, negative indicates separation)
    f32 Result = ProjectionA + ProjectionB - Distance;
    return Result;
}

inline bool32
TryAxis(collider_box a, collider_box b, vec3 Axis, u32 Index, f32 *SmallestPenetration, u32 *SmallestCase)
{
    // Make sure we have a normalized axis, and don't check almost parallel axes
    if (SquaredMagnitude(Axis) < EPSILON)
    {
        return true;
    }

    Axis = Normalize(Axis);

    f32 Penetration = PenetrationOnAxis(a, b, Axis);

    if (Penetration < 0)
    {
        return false;
    }

    if (Penetration < *SmallestPenetration)
    {
        *SmallestPenetration = Penetration;
        *SmallestCase = Index;
    }

    return true;
}

dummy_internal bool32
TestBoxBox(collider_box a, collider_box b)
{
    vec3 xAxisA = GetAxis(a.Transform, 0);
    vec3 yAxisA = GetAxis(a.Transform, 1);
    vec3 zAxisA = GetAxis(a.Transform, 2);

    vec3 xAxisB = GetAxis(b.Transform, 0);
    vec3 yAxisB = GetAxis(b.Transform, 1);
    vec3 zAxisB = GetAxis(b.Transform, 2);

    bool32 Result = (
        OverlapOnAxis(a, b, xAxisA) &&
        OverlapOnAxis(a, b, yAxisA) &&
        OverlapOnAxis(a, b, zAxisA) &&

        OverlapOnAxis(a, b, xAxisB) &&
        OverlapOnAxis(a, b, yAxisB) &&
        OverlapOnAxis(a, b, zAxisB) &&

        OverlapOnAxis(a, b, Cross(xAxisA, xAxisB)) &&
        OverlapOnAxis(a, b, Cross(xAxisA, yAxisB)) &&
        OverlapOnAxis(a, b, Cross(xAxisA, zAxisB)) &&
        OverlapOnAxis(a, b, Cross(yAxisA, xAxisB)) &&
        OverlapOnAxis(a, b, Cross(yAxisA, yAxisB)) &&
        OverlapOnAxis(a, b, Cross(yAxisA, zAxisB)) &&
        OverlapOnAxis(a, b, Cross(zAxisA, xAxisB)) &&
        OverlapOnAxis(a, b, Cross(zAxisA, yAxisB)) &&
        OverlapOnAxis(a, b, Cross(zAxisA, zAxisB))
    );

    return Result;
}

dummy_internal u32
CalculateSpherePlaneContacts(collider_sphere *Sphere, plane Plane, contact *Contacts)
{
    vec3 Position = GetTranslation(Sphere->Transform);

    // Calculate the distance from the plane
    f32 Distance = Dot(Position, Plane.Normal) - Sphere->Radius - Plane.Distance;

    if (Distance >= 0)
    {
        return 0;
    }

    contact *Contact = Contacts;

    Contact->Point = Position - Plane.Normal * (Distance + Sphere->Radius);
    Contact->Normal = Plane.Normal;
    Contact->Penetration = -Distance;

    return 1;
}

dummy_internal u32
CalculateBoxPlaneContacts(collider_box Box, plane Plane, contact *Contacts)
{
    // Check for intersection
    if (!TestBoxPlane(Box, Plane))
    {
        return 0;
    }

    vec3 Vertices[8];
    CalculateVertices(Box, Vertices);

    u32 ContactCount = 0;

    for (u32 VertexIndex = 0; VertexIndex < ArrayCount(Vertices); ++VertexIndex)
    {
        vec3 VertexPosition = Vertices[VertexIndex];

        // Calculate the distance from the plane
        f32 VertexDistance = Dot(VertexPosition, Plane.Normal);

        // Compare this to the plane's distance
        if (VertexDistance <= Plane.Distance)
        {
            contact *Contact = Contacts + ContactCount++;

            // The contact point is halfway between the vertex and the plane
            Contact->Point = VertexPosition + Plane.Normal * (VertexDistance - Plane.Distance);
            Contact->Normal = Plane.Normal;
            Contact->Penetration = Plane.Distance - VertexDistance;
        }
    }

    return ContactCount;
}

dummy_internal u32
CalculateSphereSphereContacts(collider_sphere a, collider_sphere b, contact *Contacts)
{
    vec3 PositionA = GetTranslation(a.Transform);
    vec3 PositionB = GetTranslation(b.Transform);

    // Find the vector between the objects
    vec3 MidLine = PositionA - PositionB;
    f32 Distance = Magnitude(MidLine);

    // See if it is large enough
    if (Distance <= 0.f || Distance >= a.Radius + b.Radius)
    {
        return 0;
    }

    contact *Contact = Contacts;

    Contact->Point = PositionA + MidLine * 0.5f;
    Contact->Normal = Normalize(MidLine);
    Contact->Penetration = a.Radius + b.Radius - Distance;

    return 1;
}

dummy_internal u32
CalculateBoxSphereContacts(collider_box Box, collider_sphere Sphere, contact *Contacts)
{
    // Transform the center of the sphere into box coordinates
    vec3 SphereCenterWorld = GetTranslation(Sphere.Transform);
    vec3 SphereCenterBox = (Inverse(Box.Transform) * vec4(SphereCenterWorld, 1.f)).xyz;

    vec3 BoxHalfSize = Box.HalfSize * GetScale(Box.Transform);

    // Early out check to see if we can exclude the contact
    if (
        Abs(SphereCenterBox.x) - Sphere.Radius > BoxHalfSize.x ||
        Abs(SphereCenterBox.y) - Sphere.Radius > BoxHalfSize.y ||
        Abs(SphereCenterBox.z) - Sphere.Radius > BoxHalfSize.z
    )
    {
        return 0;
    }

    vec3 ClosestPointBox = vec3(0.f);

    // Clamp each coordinate to the box
    for (u32 Axis = 0; Axis < 3; ++Axis)
    {
        f32 Distance = SphereCenterBox[Axis];

        if (Distance > BoxHalfSize[Axis])
        {
            Distance = BoxHalfSize[Axis];
        }

        if (Distance < -BoxHalfSize[Axis])
        {
            Distance = -BoxHalfSize[Axis];
        }

        ClosestPointBox[Axis] = Distance;
    }

    // Check we're in contact
    f32 Distance = Magnitude(ClosestPointBox - SphereCenterBox);

    if (Distance > Sphere.Radius)
    {
        return 0;
    }

    vec3 ClosestPointWorld = (Box.Transform * vec4(ClosestPointBox, 1.f)).xyz;

    contact *Contact = Contacts;

    Contact->Point = ClosestPointWorld;
    Contact->Normal = Normalize(ClosestPointWorld - SphereCenterWorld);
    Contact->Penetration = Sphere.Radius - Distance;

    return 1;
}

dummy_internal void
FillPointFaceBoxBox(collider_box a, collider_box b, contact *Contacts, f32 Penetration, u32 Best)
{
    // This method is called when we know that a vertex from
    // box two is in contact with box one
    contact *Contact = Contacts;

    vec3 ToCenter = GetTranslation(b.Transform) - GetTranslation(a.Transform);

    // We know which axis the collision is on (i.e. best),
    // but we need to work out which of the two faces on
    // this axis
    vec3 Normal = GetAxis(a.Transform, Best);
    if (Dot(Normal, ToCenter) > 0.f)
    {
        Normal *= -1.f;
    }

    // Work out which vertex of box two we're colliding with
    vec3 Vertex = b.HalfSize;

    for (u32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
    {
        if (Dot(GetAxis(b.Transform, AxisIndex), Normal) < 0)
        {
            Vertex[AxisIndex] *= -1.f;
        }
    }

    Contact->Point = b.Transform * Vertex;
    Contact->Normal = Normal;
    Contact->Penetration = Penetration;
}

// If useOne is true, and the contact point is outside
// the edge (in the case of an edge-face contact) then
// we use one's midpoint, otherwise we use two's
dummy_internal vec3
ContactPoint(vec3 pOne, vec3 dOne, f32 oneSize, vec3 pTwo, vec3 dTwo, f32 twoSize, bool32 useOne)
{
    vec3 toSt, cOne, cTwo;
    f32 dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
    f32 denom, mua, mub;

    smOne = SquaredMagnitude(dOne);
    smTwo = SquaredMagnitude(dTwo);
    dpOneTwo = Dot(dTwo, dOne);

    toSt = pOne - pTwo;
    dpStaOne = Dot(dOne, toSt);
    dpStaTwo = Dot(dTwo, toSt);

    denom = smOne * smTwo - dpOneTwo * dpOneTwo;

    // Zero denominator indicates parrallel lines
    if (NearlyEqual(denom, 0.f))
    {
        return useOne ? pOne : pTwo;
    }

    mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
    mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

    // If either of the edges has the nearest point out
    // of bounds, then the edges aren't crossed, we have
    // an edge-face contact. Our point is on the edge, which
    // we know from the useOne parameter
    if (
        mua > oneSize ||
        mua < -oneSize ||
        mub > twoSize ||
        mub < -twoSize)
    {
        return useOne ? pOne : pTwo;
    }
    else
    {
        cOne = pOne + dOne * mua;
        cTwo = pTwo + dTwo * mub;

        return cOne * 0.5f + cTwo * 0.5f;
    }
}

dummy_internal u32
CalculateBoxBoxContacts(collider_box a, collider_box b, contact *Contacts)
{
    // We start assuming there is no contact
    f32 Penetration = F32_MAX;
    u32 Best = 0xffffff;

    // Now we check each axes, returning if it gives us
    // a separating axis, and keeping track of the axis with
    // the smallest penetration otherwise
    vec3 xAxisA = GetAxis(a.Transform, 0);
    vec3 yAxisA = GetAxis(a.Transform, 1);
    vec3 zAxisA = GetAxis(a.Transform, 2);

    vec3 xAxisB = GetAxis(b.Transform, 0);
    vec3 yAxisB = GetAxis(b.Transform, 1);
    vec3 zAxisB = GetAxis(b.Transform, 2);

    if (!TryAxis(a, b, xAxisA, 0, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, yAxisA, 1, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, zAxisA, 2, &Penetration, &Best)) return 0;

    if (!TryAxis(a, b, xAxisB, 3, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, yAxisB, 4, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, zAxisB, 5, &Penetration, &Best)) return 0;

    // Store the best axis-major, in case we run into almost parallel edge collisions later
    u32 BestSingleAxis = Best;

    if (!TryAxis(a, b, Cross(xAxisA, xAxisB), 6, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, Cross(xAxisA, yAxisB), 7, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, Cross(xAxisA, zAxisB), 8, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, Cross(yAxisA, xAxisB), 9, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, Cross(yAxisA, yAxisB), 10, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, Cross(yAxisA, zAxisB), 11, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, Cross(zAxisA, xAxisB), 12, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, Cross(zAxisA, yAxisB), 13, &Penetration, &Best)) return 0;
    if (!TryAxis(a, b, Cross(zAxisA, zAxisB), 14, &Penetration, &Best)) return 0;

    // Make sure we've got a result
    Assert(Best != 0xffffff);

    // We now know there's a collision, and we know which
    // of the axes gave the smallest penetration. We now
    // can deal with it in different ways depending on
    // the case
    if (Best < 3)
    {
        // We've got a vertex of box two on a face of box one
        FillPointFaceBoxBox(a, b, Contacts, Penetration, Best);

        return 1;
    }
    else if (Best < 6)
    {
        // We've got a vertex of box one on a face of box two.
        // We use the same algorithm as above, but swap around
        // one and two (and therefore also the vector between their
        // centers)
        FillPointFaceBoxBox(b, a, Contacts, Penetration, Best - 3);

        return 1;
    }
    else
    {
        // We've got an edge-edge contact. Find out which axes
        Best -= 6;

        u32 AxisIndexA = Best / 3;
        u32 AxisIndexB = Best % 3;

        vec3 AxisA = GetAxis(a.Transform, AxisIndexA);
        vec3 AxisB = GetAxis(b.Transform, AxisIndexB);

        vec3 Axis = Cross(AxisA, AxisB);
        Axis = Normalize(Axis);

        vec3 ToCenter = GetTranslation(b.Transform) - GetTranslation(a.Transform);

        // The axis should point from box one to box two
        if (Dot(Axis, ToCenter) > 0.f)
        {
            Axis *= -1.f;
        }

        // We have the axes, but not the edges: each axis has 4 edges parallel
        // to it, we need to find which of the 4 for each object. We do
        // that by finding the point in the center of the edge. We know
        // its component in the direction of the box's collision axis is zero
        // (its a mid-point) and we determine which of the extremes in each
        // of the other axes is closest
        vec3 PointOnOneEdge = a.HalfSize;
        vec3 PointOnTwoEdge = b.HalfSize;

        for (u32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
        {
            if (AxisIndex == AxisIndexA)
            {
                PointOnOneEdge[AxisIndex] = 0;
            }
            else if (Dot(GetAxis(a.Transform, AxisIndex), Axis) > 0)
            {
                PointOnOneEdge[AxisIndex] *= -1.f;
            }

            if (AxisIndex == AxisIndexB)
            {
                PointOnTwoEdge[AxisIndex] = 0;
            }
            else if (Dot(GetAxis(b.Transform, AxisIndex), Axis) < 0)
            {
                PointOnTwoEdge[AxisIndex] *= -1.f;
            }
        }

        // Move them into world coordinates
        PointOnOneEdge = a.Transform * PointOnOneEdge;
        PointOnTwoEdge = b.Transform * PointOnTwoEdge;

        // So we have a point and a direction for the colliding edges.
        // We need to find out point of closest approach of the two
        // line-segments
        vec3 HalfSizeA = a.HalfSize * GetScale(a.Transform);
        vec3 HalfSizeB = b.HalfSize * GetScale(b.Transform);

        vec3 Vertex = ContactPoint(
            PointOnOneEdge, AxisA, HalfSizeA[AxisIndexA], 
            PointOnTwoEdge, AxisB, HalfSizeB[AxisIndexB], 
            BestSingleAxis > 2
        );

        contact *Contact = Contacts;

        Contact->Point = Vertex;
        Contact->Normal = Axis;
        Contact->Penetration = Penetration;

        return 1;
    }

    return 0;
}

struct contact_resolver
{
    contact *Contact;
    rigid_body *Body;

    mat3 ContactToWorld;
    mat3 WorldToContact;

    vec3 ContactVelocity;
};

// Constructs an arbitrary orthonormal basis for the contact
dummy_internal mat3
ContactToWorldTransform(contact *Contact)
{
    vec3 xAxis = Contact->Normal;
    vec3 yAxis;
    vec3 zAxis;

    // Check whether the Z-axis is nearer to the X or Y axis
    if (Abs(Contact->Normal.x) > Abs(Contact->Normal.y))
    {
        // Scaling factor to ensure the results are normalised
        f32 s = 1.f / Sqrt(Square(Contact->Normal.z) + Square(Contact->Normal.x));

        // The new Y-axis is at right angles to the world Y-axis
        yAxis.x = Contact->Normal.z * s;
        yAxis.y = 0.f;
        yAxis.z = -Contact->Normal.x * s;

        // The new Z-axis is at right angles to the new X and Y axes
        zAxis.x = Contact->Normal.y * yAxis.x;
        zAxis.y = Contact->Normal.z * yAxis.x - Contact->Normal.x * yAxis.z;
        zAxis.z = -Contact->Normal.y * yAxis.x;
    }
    else
    {
        // Scaling factor to ensure the results are normalised
        f32 s = 1.f / Sqrt(Square(Contact->Normal.z) + Square(Contact->Normal.y));

        // The new Y-axis is at right angles to the world X-axis
        yAxis.x = 0.f;
        yAxis.y = -Contact->Normal.z * s;
        yAxis.z = Contact->Normal.y * s;

        // The new Z-axis is at right angles to the new X and Y axes
        zAxis.x = Contact->Normal.y * yAxis.z - Contact->Normal.z * yAxis.y;
        zAxis.y = -Contact->Normal.x * yAxis.z;
        zAxis.z = Contact->Normal.x * yAxis.y;
    }

    mat3 Result = mat3(
        vec3(xAxis.x, yAxis.x, zAxis.x),
        vec3(xAxis.y, yAxis.y, zAxis.y),
        vec3(xAxis.z, yAxis.z, zAxis.z)
    );

    return Result;
}

dummy_internal vec3
CalculateLocalVelocity(contact_resolver *Resolver, f32 dt)
{
    contact *Contact = Resolver->Contact;
    rigid_body *Body = Resolver->Body;

    vec3 RelativeContactPosition = Contact->Point - Body->Position;

    // Calculate the velocity of the contact point
    vec3 VelocityWorld = Cross(Body->AngularVelocity, RelativeContactPosition);
    VelocityWorld += Body->Velocity;

    // Turn the velocity into contact-coordinates
    vec3 VelocityContact = Resolver->WorldToContact * VelocityWorld;

    // Calculate the ammount of velocity that is due to forces without reactions
    vec3 AccelerationVelocityWorld = Body->PrevAcceleration * dt;

    // Calculate the velocity in contact-coordinates
    vec3 AccelerationVelocityContact = Resolver->WorldToContact * AccelerationVelocityWorld;

    // We ignore any component of acceleration in the contact normal direction, we are only interested in planar acceleration
    AccelerationVelocityContact.x = 0.f;

    // Add the planar velocities - if there's enough friction they will be removed during velocity resolution
    VelocityContact += AccelerationVelocityContact;

    return VelocityContact;
}

dummy_internal void
ResolveContactsOrSomething()
{
    //
    contact Contact = 
    {
        .Point = vec3(0.f),
        .Normal = vec3(0.f, 1.f, 0.f),
        .Penetration = 0.1f
    };
    rigid_body Body = {};
    f32 dt = 0.01f;

    contact_resolver Resolver = {};
    Resolver.Contact = &Contact;
    Resolver.Body = &Body;
    //

    //
    Resolver.ContactToWorld = ContactToWorldTransform(Resolver.Contact);
    Resolver.WorldToContact = Transpose(Resolver.ContactToWorld);

    Resolver.ContactVelocity = CalculateLocalVelocity(&Resolver, dt);



    // todo(continue): contact resolution!!!
    //
}
