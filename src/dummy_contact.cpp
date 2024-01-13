#include "dummy.h"

inline f32
PenetrationOnAxis(collider_box *One, collider_box *Two, vec3 Axis)
{
    vec3 ToCenter = GetTranslation(Two->Transform) - GetTranslation(One->Transform);

    // Project the half-sizes of One and Two onto Axis
    f32 ProjectionOne = TransformToAxis(One, Axis);
    f32 ProjectionTwo = TransformToAxis(Two, Axis);

    // Project this onto the axis
    f32 Distance = Abs(Dot(ToCenter, Axis));

    // Return the overlap (i.e. positive indicates overlap, negative indicates separation)
    f32 Result = ProjectionOne + ProjectionTwo - Distance;

    return Result;
}

inline bool32
TryAxis(collider_box *One, collider_box *Two, vec3 Axis, u32 Index, f32 *SmallestPenetration, u32 *SmallestCase)
{
    // Make sure we have a normalized axis, and don't check almost parallel axes
    if (SquaredMagnitude(Axis) < EPSILON)
    {
        return true;
    }

    Axis = Normalize(Axis);

    f32 Penetration = PenetrationOnAxis(One, Two, Axis);

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

dummy_internal u32
CalculateSpherePlaneContacts(collider_sphere *Sphere, plane Plane, contact *Contacts, contact_params Params)
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
    Contact->Bodies[0] = Sphere->Body;
    Contact->Bodies[1] = 0;
    Contact->Friction = Params.Friction;
    Contact->Restitution = Params.Restitution;

    return 1;
}

dummy_internal u32
CalculateBoxPlaneContacts(collider_box *Box, plane Plane, contact *Contacts, contact_params Params)
{
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

        f32 VertexDistance = Dot(VertexPosition, Plane.Normal);

        if (VertexDistance <= Plane.Distance)
        {
            contact *Contact = Contacts + ContactCount++;

            // The contact point is halfway between the vertex and the plane
            Contact->Point = VertexPosition + Plane.Normal * (VertexDistance - Plane.Distance);
            Contact->Normal = Plane.Normal;
            Contact->Penetration = Plane.Distance - VertexDistance;
            Contact->Bodies[0] = Box->Body;
            Contact->Bodies[1] = 0;
            Contact->Friction = Params.Friction;
            Contact->Restitution = Params.Restitution;
        }
    }

    return ContactCount;
}

dummy_internal u32
CalculateSphereSphereContacts(collider_sphere *One, collider_sphere *Two, contact *Contacts, contact_params Params)
{
    vec3 PositionOne = GetTranslation(One->Transform);
    vec3 PositionTwo = GetTranslation(Two->Transform);

    // Find the vector between the objects
    vec3 MidLine = PositionOne - PositionTwo;
    f32 Distance = Magnitude(MidLine);

    // See if it is large enough
    if (Distance <= 0.f || Distance >= One->Radius + Two->Radius)
    {
        return 0;
    }

    contact *Contact = Contacts;

    Contact->Point = PositionOne + MidLine * 0.5f;
    Contact->Normal = Normalize(MidLine);
    Contact->Penetration = One->Radius + Two->Radius - Distance;
    Contact->Bodies[0] = One->Body;
    Contact->Bodies[1] = Two->Body;
    Contact->Friction = Params.Friction;
    Contact->Restitution = Params.Restitution;

    return 1;
}

dummy_internal u32
CalculateBoxSphereContacts(collider_box *Box, collider_sphere *Sphere, contact *Contacts)
{
    // Transform the center of the sphere into box coordinates
    vec3 SphereCenterWorld = GetTranslation(Sphere->Transform);
    vec3 SphereCenterBox = Inverse(Box->Transform) * SphereCenterWorld;

    vec3 BoxHalfSize = Box->HalfSize * GetScale(Box->Transform);

    // Early out check to see if we can exclude the contact
    if (
        Abs(SphereCenterBox.x) - Sphere->Radius > BoxHalfSize.x ||
        Abs(SphereCenterBox.y) - Sphere->Radius > BoxHalfSize.y ||
        Abs(SphereCenterBox.z) - Sphere->Radius > BoxHalfSize.z
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

    if (Distance > Sphere->Radius)
    {
        return 0;
    }

    vec3 ClosestPointWorld = Box->Transform * ClosestPointBox;

    contact *Contact = Contacts;

    Contact->Point = ClosestPointWorld;
    Contact->Normal = Normalize(ClosestPointWorld - SphereCenterWorld);
    Contact->Penetration = Sphere->Radius - Distance;

    return 1;
}

dummy_internal void
FillPointFaceBoxBox(collider_box *One, collider_box *Two, contact *Contacts, contact_params Params, f32 Penetration, u32 Best)
{
    // This method is called when we know that a vertex from box two is in contact with box one
    contact *Contact = Contacts;

    vec3 ToCenter = GetTranslation(Two->Transform) - GetTranslation(One->Transform);

    // We know which axis the collision is on (i.e. best), but we need to work out which of the two faces on this axis
    vec3 Normal = GetAxis(One->Transform, Best);
    if (Dot(Normal, ToCenter) > 0.f)
    {
        Normal *= -1.f;
    }

    // Work out which vertex of box two we're colliding with
    vec3 Vertex = Two->HalfSize;

    for (u32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
    {
        if (Dot(GetAxis(Two->Transform, AxisIndex), Normal) < 0)
        {
            Vertex[AxisIndex] *= -1.f;
        }
    }

    Contact->Point = Two->Transform * Vertex;
    Contact->Normal = Normal;
    Contact->Penetration = Penetration;
    Contact->Bodies[0] = One->Body;
    Contact->Bodies[1] = Two->Body;
    Contact->Friction = Params.Friction;
    Contact->Restitution = Params.Restitution;
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
CalculateBoxBoxContacts(collider_box *One, collider_box *Two, contact *Contacts, contact_params Params)
{
    // We start assuming there is no contact
    f32 Penetration = F32_MAX;
    u32 Best = 0xffffff;

    // Now we check each axes, returning if it gives us
    // a separating axis, and keeping track of the axis with
    // the smallest penetration otherwise
    vec3 xAxisOne = GetAxis(One->Transform, 0);
    vec3 yAxisOne = GetAxis(One->Transform, 1);
    vec3 zAxisOne = GetAxis(One->Transform, 2);

    vec3 xAxisTwo = GetAxis(Two->Transform, 0);
    vec3 yAxisTwo = GetAxis(Two->Transform, 1);
    vec3 zAxisTwo = GetAxis(Two->Transform, 2);

    if (!TryAxis(One, Two, xAxisOne, 0, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, yAxisOne, 1, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, zAxisOne, 2, &Penetration, &Best)) return 0;

    if (!TryAxis(One, Two, xAxisTwo, 3, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, yAxisTwo, 4, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, zAxisTwo, 5, &Penetration, &Best)) return 0;

    // Store the best axis-major, in case we run into almost parallel edge collisions later
    u32 BestSingleAxis = Best;

    if (!TryAxis(One, Two, Cross(xAxisOne, xAxisTwo), 6, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, Cross(xAxisOne, yAxisTwo), 7, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, Cross(xAxisOne, zAxisTwo), 8, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, Cross(yAxisOne, xAxisTwo), 9, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, Cross(yAxisOne, yAxisTwo), 10, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, Cross(yAxisOne, zAxisTwo), 11, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, Cross(zAxisOne, xAxisTwo), 12, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, Cross(zAxisOne, yAxisTwo), 13, &Penetration, &Best)) return 0;
    if (!TryAxis(One, Two, Cross(zAxisOne, zAxisTwo), 14, &Penetration, &Best)) return 0;

    // Make sure we've got a result
    Assert(Best != 0xffffff);

    // We now know there's a collision, and we know which
    // of the axes gave the smallest penetration. We now
    // can deal with it in different ways depending on
    // the case
    if (Best < 3)
    {
        // We've got a vertex of box two on a face of box one
        FillPointFaceBoxBox(One, Two, Contacts, Params, Penetration, Best);

        return 1;
    }
    else if (Best < 6)
    {
        // We've got a vertex of box one on a face of box two.
        // We use the same algorithm as above, but swap around
        // one and two (and therefore also the vector between their
        // centers)
        FillPointFaceBoxBox(Two, One, Contacts, Params, Penetration, Best - 3);

        return 1;
    }
    else
    {
        // We've got an edge-edge contact. Find out which axes
        Best -= 6;

        u32 AxisIndexOne = Best / 3;
        u32 AxisIndexTwo = Best % 3;

        vec3 AxisOne = GetAxis(One->Transform, AxisIndexOne);
        vec3 AxisTwo = GetAxis(Two->Transform, AxisIndexTwo);

        vec3 Axis = Cross(AxisOne, AxisTwo);
        Axis = Normalize(Axis);

        vec3 ToCenter = GetTranslation(Two->Transform) - GetTranslation(One->Transform);

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
        vec3 PointOnOneEdge = One->HalfSize;
        vec3 PointOnTwoEdge = Two->HalfSize;

        for (u32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
        {
            if (AxisIndex == AxisIndexOne)
            {
                PointOnOneEdge[AxisIndex] = 0;
            }
            else if (Dot(GetAxis(One->Transform, AxisIndex), Axis) > 0)
            {
                PointOnOneEdge[AxisIndex] *= -1.f;
            }

            if (AxisIndex == AxisIndexTwo)
            {
                PointOnTwoEdge[AxisIndex] = 0;
            }
            else if (Dot(GetAxis(Two->Transform, AxisIndex), Axis) < 0)
            {
                PointOnTwoEdge[AxisIndex] *= -1.f;
            }
        }

        // Move them into world coordinates
        PointOnOneEdge = One->Transform * PointOnOneEdge;
        PointOnTwoEdge = Two->Transform * PointOnTwoEdge;

        // So we have a point and a direction for the colliding edges.
        // We need to find out point of closest approach of the two
        // line-segments
        vec3 HalfSizeOne = One->HalfSize * GetScale(One->Transform);
        vec3 HalfSizeTwo = Two->HalfSize * GetScale(Two->Transform);

        vec3 Vertex = ContactPoint(
            PointOnOneEdge, AxisOne, HalfSizeOne[AxisIndexOne],
            PointOnTwoEdge, AxisTwo, HalfSizeTwo[AxisIndexTwo],
            BestSingleAxis > 2
        );

        contact *Contact = Contacts;

        Contact->Point = Vertex;
        Contact->Normal = Axis;
        Contact->Penetration = Penetration;
        Contact->Bodies[0] = One->Body;
        Contact->Bodies[1] = Two->Body;
        Contact->Friction = Params.Friction;
        Contact->Restitution = Params.Restitution;

        return 1;
    }

    return 0;
}

inline void
SwapContactBodies(contact *Contact)
{
    rigid_body *Temp = Contact->Bodies[0];
    Contact->Bodies[0] = Contact->Bodies[1];
    Contact->Bodies[1] = Temp;

    Contact->Normal *= -1.f;
}

// Constructs an arbitrary orthonormal basis for the contact
dummy_internal void
CalculateContactBasis(contact *Contact)
{
#if 1
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
#else
    vec3 xAxis = Contact->Normal;
    vec3 yAxis = Normalize(Cross(vec3(1.f, 0.f, 0.f), xAxis));
    vec3 zAxis = Normalize(Cross(xAxis, yAxis));
#endif

    mat3 ContactToWorld = mat3(
        vec3(xAxis.x, yAxis.x, zAxis.x),
        vec3(xAxis.y, yAxis.y, zAxis.y),
        vec3(xAxis.z, yAxis.z, zAxis.z)
    );

    mat3 WorldToContact = Transpose(ContactToWorld);

    Contact->ContactToWorld = ContactToWorld;
    Contact->WorldToContact = WorldToContact;
}

dummy_internal vec3
CalculateLocalVelocity(contact *Contact, u32 BodyIndex, f32 dt)
{
    rigid_body *Body = Contact->Bodies[BodyIndex];

    // Calculate the velocity of the contact point
    vec3 VelocityWorld = Cross(Body->AngularVelocity, Contact->RelativeContactPositions[BodyIndex]);
    VelocityWorld += Body->Velocity;

    // Turn the velocity into contact-coordinates
    vec3 VelocityContact = Contact->WorldToContact * VelocityWorld;

    // Calculate the ammount of velocity that is due to forces without reactions
    // todo: PrevAcceleration?
    vec3 AccelerationVelocityWorld = Body->Acceleration * dt;

    // Calculate the velocity in contact-coordinates
    vec3 AccelerationVelocityContact = Contact->WorldToContact * AccelerationVelocityWorld;

    // We ignore any component of acceleration in the contact normal direction, we are only interested in planar acceleration
    AccelerationVelocityContact.x = 0.f;

    // Add the planar velocities - if there's enough friction they will be removed during velocity resolution
    VelocityContact += AccelerationVelocityContact;

    return VelocityContact;
}

dummy_internal void
CalculateDesiredDeltaVelocity(contact *Contact, f32 dt)
{
    f32 VelocityLimit = 0.25f;

    // Calculate the acceleration induced velocity accumulated this frame
    f32 VelocityFromAcceleration = 0.f;
    
    if (Contact->Bodies[0]->IsAwake)
    {
        // todo: PrevAcceleration?
        VelocityFromAcceleration += Dot(Contact->Bodies[0]->Acceleration * dt, Contact->Normal);
    }

    if (Contact->Bodies[1] && Contact->Bodies[1]->IsAwake)
    {
        // todo: PrevAcceleration?
        VelocityFromAcceleration -= Dot(Contact->Bodies[1]->Acceleration * dt, Contact->Normal);
    }

    // If the velocity is very slow, limit the restitution
    f32 Restituion = Contact->Restitution;
    if (Abs(Contact->ContactVelocity.x) < VelocityLimit)
    {
        Restituion = 0.f;
    }

    // Combine the bounce velocity with the removed acceleration velocity
    Contact->DesiredDeltaVelocity = -Contact->ContactVelocity.x - Restituion * (Contact->ContactVelocity.x - VelocityFromAcceleration);
}

dummy_internal void
MatchAwakeState(contact *Contact)
{
    // Collisions with the world never cause a body to wake up
    if (Contact->Bodies[1])
    {
        bool32 Body0Awake = Contact->Bodies[0]->IsAwake;
        bool32 Body1Awake = Contact->Bodies[1]->IsAwake;

        // Wake up only the sleeping one
        if (Body0Awake ^ Body1Awake)
        {
            if (Body0Awake)
            {
                SetIsAwake(Contact->Bodies[1], true);
            }
            else
            {
                SetIsAwake(Contact->Bodies[0], true);
            }
        }
    }
}

dummy_internal void
PrepareContacts(contact_resolver *Resolver, f32 dt)
{
    for (u32 ContactIndex = 0; ContactIndex < Resolver->ContactCount; ++ContactIndex)
    {
        contact *Contact = Resolver->Contacts + ContactIndex;

        if (!Contact->Bodies[0])
        {
            SwapContactBodies(Contact);
        }

        Assert(Contact->Bodies[0]);

        CalculateContactBasis(Contact);

        // Store the relative position of the contact relative to each body
        Contact->RelativeContactPositions[0] = Contact->Point - Contact->Bodies[0]->CenterOfMassWorld;
        if (Contact->Bodies[1])
        {
            Contact->RelativeContactPositions[1] = Contact->Point - Contact->Bodies[1]->CenterOfMassWorld;
        }

        // Find the relative velocity of the bodies at the contact point
        Contact->ContactVelocity = CalculateLocalVelocity(Contact, 0, dt);
        if (Contact->Bodies[1])
        {
            Contact->ContactVelocity -= CalculateLocalVelocity(Contact, 1, dt);
        }

        // Calculate the desired change in velocity for resolution
        CalculateDesiredDeltaVelocity(Contact, dt);
    }
}

dummy_internal void
ApplyPositionChange(contact *Contact, vec3 *LinearChange, vec3 *AngularChange, f32 Penetration)
{
    f32 AngularLimit = 0.2f;
    f32 AngularMove[2] = {};
    f32 LinearMove[2] = {};

    f32 TotalInertia = 0.f;
    f32 LinearInertia[2] = {};
    f32 AngularInertia[2] = {};

    // We need to work out the inertia of each object in the direction
    // of the contact normal, due to angular inertia only
    for (u32 BodyIndex = 0; BodyIndex < 2; ++BodyIndex)
    {
        rigid_body *Body = Contact->Bodies[BodyIndex];

        if (Body)
        {
            // Use the same procedure as for calculating frictionless
            // velocity change to work out the angular inertia
            vec3 AngularInertiaWorld = Cross(Contact->RelativeContactPositions[BodyIndex], Contact->Normal);
            AngularInertiaWorld = Body->InverseInertiaTensorWorld * AngularInertiaWorld;
            AngularInertiaWorld = Cross(AngularInertiaWorld, Contact->RelativeContactPositions[BodyIndex]);

            AngularInertia[BodyIndex] = Dot(AngularInertiaWorld, Contact->Normal);

            // The linear component is simply the inverse mass
            LinearInertia[BodyIndex] = Body->InverseMass;

            // Keep track of the total inertia from all components
            TotalInertia += LinearInertia[BodyIndex] + AngularInertia[BodyIndex];
        }

        // We break the loop here so that the TotalInertia value is
        // completely calculated (by both iterations) before
        // continuing
    }

    // Loop through again calculating and applying the changes
    for (u32 BodyIndex = 0; BodyIndex < 2; ++BodyIndex)
    {
        rigid_body *Body = Contact->Bodies[BodyIndex];

        if (Body)
        {
            // The linear and angular movements required are in proportion to the two inverse inertias
            f32 Sign = (BodyIndex == 0) ? 1.f : -1.f;
            AngularMove[BodyIndex] = Sign * Penetration * (AngularInertia[BodyIndex] / TotalInertia);
            LinearMove[BodyIndex] = Sign * Penetration * (LinearInertia[BodyIndex] / TotalInertia);

            // To avoid angular projections that are too great (when mass is large
            // but inertia tensor is small) limit the angular move
            vec3 Projection =
                Contact->RelativeContactPositions[BodyIndex] +
                Contact->Normal * (-Dot(Contact->RelativeContactPositions[BodyIndex], Contact->Normal));

            // Use the small angle approximation for the sine of the angle (i.e.
            // the magnitude would be sine(angularLimit) * projection.magnitude
            // but we approximate sine(angularLimit) to angularLimit)
            f32 MaxMagnitude = AngularLimit * Magnitude(Projection);

            if (AngularMove[BodyIndex] < -MaxMagnitude)
            {
                f32 TotalMove = AngularMove[BodyIndex] + LinearMove[BodyIndex];
                AngularMove[BodyIndex] = -MaxMagnitude;
                LinearMove[BodyIndex] = TotalMove - AngularMove[BodyIndex];
            }
            else if (AngularMove[BodyIndex] > MaxMagnitude)
            {
                f32 TotalMove = AngularMove[BodyIndex] + LinearMove[BodyIndex];
                AngularMove[BodyIndex] = MaxMagnitude;
                LinearMove[BodyIndex] = TotalMove - AngularMove[BodyIndex];
            }

            // We have the linear amount of movement required by turning
            // the rigid body (in angularMove[i]). We now need to
            // calculate the desired rotation to achieve that
            if (AngularMove[BodyIndex] == 0)
            {
                // Easy case - no angular movement means no rotation
                AngularChange[BodyIndex] = vec3(0.f);
            }
            else
            {
                // Work out the direction we'd like to rotate in
                vec3 TargetAngularDirection = Cross(Contact->RelativeContactPositions[BodyIndex], Contact->Normal);

                // Work out the direction we'd need to rotate to achieve that
                AngularChange[BodyIndex] =
                    Body->InverseInertiaTensorWorld * TargetAngularDirection *
                    (AngularMove[BodyIndex] / AngularInertia[BodyIndex]);
            }

            // Velocity change is easier - it is just the linear movement along the contact normal
            LinearChange[BodyIndex] = Contact->Normal * LinearMove[BodyIndex];

            // Now we can start to apply the values we've calculated
            // Apply the linear movement
            Body->Position += Contact->Normal * LinearMove[BodyIndex];

            // And the change in orientation
            Body->Orientation += AngularChange[BodyIndex];
            Body->Orientation = Normalize(Body->Orientation);

            Assert(IsFinite(Body->Position));
            Assert(IsFinite(Body->Orientation));

            // We need to calculate the derived data for any body that is
            // asleep, so that the changes are reflected in the object's
            // data. Otherwise the resolution will not change the position
            // of the object, and the next collision detection round will
            // have the same penetration
            if (!Body->IsAwake)
            {
                CalculateRigidBodyState(Body);
            }
        }
    }
}

dummy_internal vec3
CalculateFrictionlessImpulse(contact *Contact)
{
    // Build a vector that shows the change in velocity in
    // world space for a unit impulse in the direction of the contact normal
    vec3 DeltaVelocityWorld = Cross(Contact->RelativeContactPositions[0], Contact->Normal);
    DeltaVelocityWorld = Contact->Bodies[0]->InverseInertiaTensorWorld * DeltaVelocityWorld;
    DeltaVelocityWorld = Cross(DeltaVelocityWorld, Contact->RelativeContactPositions[0]);

    // Work out the change in velocity in contact coordiantes
    f32 DeltaVelocity = Dot(DeltaVelocityWorld, Contact->Normal);

    // Add the linear component of velocity change
    DeltaVelocity += Contact->Bodies[0]->InverseMass;

    // Check if we need to the second body's data
    if (Contact->Bodies[1])
    {
        // Go through the same transformation sequence again
        vec3 DeltaVelocityWorld = Cross(Contact->RelativeContactPositions[1], Contact->Normal);
        DeltaVelocityWorld = Contact->Bodies[1]->InverseInertiaTensorWorld * DeltaVelocityWorld;
        DeltaVelocityWorld = Cross(DeltaVelocityWorld, Contact->RelativeContactPositions[1]);

        // Add the change in velocity due to rotation
        DeltaVelocity += Dot(DeltaVelocityWorld, Contact->Normal);

        // Add the change in velocity due to linear motion
        DeltaVelocity += Contact->Bodies[1]->InverseMass;
    }

    // Calculate the required size of the impulse
    vec3 Result = vec3(Contact->DesiredDeltaVelocity / DeltaVelocity, 0.f, 0.f);

    return Result;
}

dummy_internal vec3
CalculateFrictionImpulse(contact *Contact)
{
    f32 InverseMass = Contact->Bodies[0]->InverseMass;

    // The equivalent of a cross product in matrices is multiplication
    // by a skew symmetric matrix - we build the matrix for converting
    // between linear and angular quantities
    mat3 ImpulseToTorque = SkewSymmetric(Contact->RelativeContactPositions[0]);

    // Build the matrix to convert contact impulse to change in velocity
    // in world coordinates
    mat3 DeltaVelocityWorld = ImpulseToTorque;
    DeltaVelocityWorld = DeltaVelocityWorld * Contact->Bodies[0]->InverseInertiaTensorWorld;
    DeltaVelocityWorld = DeltaVelocityWorld * ImpulseToTorque;
    DeltaVelocityWorld = DeltaVelocityWorld * -1.f;

    // Check if we need to add body two's data
    if (Contact->Bodies[1])
    {
        // Set the cross product matrix
        mat3 ImpulseToTorque = SkewSymmetric(Contact->RelativeContactPositions[1]);

        // Calculate the velocity change matrix
        mat3 DeltaVelocityWorld2 = ImpulseToTorque;
        DeltaVelocityWorld2 = DeltaVelocityWorld2 * Contact->Bodies[1]->InverseInertiaTensorWorld;
        DeltaVelocityWorld2 = DeltaVelocityWorld2 * ImpulseToTorque;
        DeltaVelocityWorld2 = DeltaVelocityWorld2 * -1.f;

        // Add to the total delta velocity
        DeltaVelocityWorld = DeltaVelocityWorld + DeltaVelocityWorld2;

        // Add to the inverse mass
        InverseMass += Contact->Bodies[1]->InverseMass;
    }

    // Do a change of basis to convert into contact coordinates
    mat3 DeltaVelocity = Contact->WorldToContact;
    DeltaVelocity = DeltaVelocity * DeltaVelocityWorld;
    DeltaVelocity = DeltaVelocity * Contact->ContactToWorld;

    // Add in the linear velocity change
    DeltaVelocity[0][0] += InverseMass;
    DeltaVelocity[1][1] += InverseMass;
    DeltaVelocity[2][2] += InverseMass;

    // Invert to get the impulse needed per unit velocity
    mat3 ImpulseMatrix = Inverse(DeltaVelocity);

    // Find the target velocities to kill
    vec3 VelocityKill = vec3(Contact->DesiredDeltaVelocity, -Contact->ContactVelocity.y, -Contact->ContactVelocity.z);

    // Find the impulse to kill target velocities
    vec3 ImpulseContact = ImpulseMatrix * VelocityKill;

    // Check for exceeding friction
    f32 PlanarImpulse = Sqrt(Square(ImpulseContact.y) + Square(ImpulseContact.z));

    if (PlanarImpulse > ImpulseContact.x * Contact->Friction)
    {
        // We need to use dynamic friction
        ImpulseContact.y /= PlanarImpulse;
        ImpulseContact.z /= PlanarImpulse;

        ImpulseContact.x =
            DeltaVelocity[0][0] +
            DeltaVelocity[0][1] * Contact->Friction * ImpulseContact.y +
            DeltaVelocity[0][2] * Contact->Friction * ImpulseContact.z;
        ImpulseContact.x = Contact->DesiredDeltaVelocity / ImpulseContact.x;
        ImpulseContact.y *= Contact->Friction * ImpulseContact.x;
        ImpulseContact.z *= Contact->Friction * ImpulseContact.x;
    }

    return ImpulseContact;
}

dummy_internal void
ApplyVelocityChange(contact *Contact, vec3 *VelocityChange, vec3 *RotationChange)
{
    // We will calculate the impulse for each contact axis
    vec3 ImpulseContact;

    if (Contact->Friction == 0.f)
    {
        // Use the short format for frictionless contacts
        ImpulseContact = CalculateFrictionlessImpulse(Contact);
    }
    else
    {
        // Otherwise we may have impulses that aren't in the direction of the
        // contact, so we need the more complex version
        ImpulseContact = CalculateFrictionImpulse(Contact);
    }

    // Convert impulse to world coordinates
    vec3 Impulse = Contact->ContactToWorld * ImpulseContact;

    // Split in the impulse into linear and rotational components
    vec3 ImpulsiveTorque = Cross(Contact->RelativeContactPositions[0], Impulse);
    RotationChange[0] = Contact->Bodies[0]->InverseInertiaTensorWorld * ImpulsiveTorque;
    VelocityChange[0] = Impulse * Contact->Bodies[0]->InverseMass;

    // Apply the changes
    Contact->Bodies[0]->Velocity += VelocityChange[0];
    Contact->Bodies[0]->AngularVelocity += RotationChange[0];

    Assert(IsFinite(Contact->Bodies[0]->Velocity));
    Assert(IsFinite(Contact->Bodies[0]->AngularVelocity));

    if (Contact->Bodies[1])
    {
        // Work out body one's linear and angular changes
        vec3 ImpulsiveTorque = Cross(Impulse, Contact->RelativeContactPositions[1]);
        RotationChange[1] = Contact->Bodies[1]->InverseInertiaTensorWorld * ImpulsiveTorque;
        VelocityChange[1] = Impulse * -Contact->Bodies[1]->InverseMass;

        // And apply them
        Contact->Bodies[1]->Velocity += VelocityChange[1];
        Contact->Bodies[1]->AngularVelocity += RotationChange[1];

        Assert(IsFinite(Contact->Bodies[1]->Velocity));
        Assert(IsFinite(Contact->Bodies[1]->AngularVelocity));
    }
}

dummy_internal void
AdjustPositions(contact_resolver *Resolver, f32 dt)
{
    vec3 LinearChange[2];
    vec3 AngularChange[2];

    f32 MaxPenetration;
    vec3 DeltaPosition;

    u32 MaxPenetrationIndex = Resolver->ContactCount;
    contact *MaxPenetrationContact = 0;

    u32 PositionIterationCount = 0;
    u32 PositionIterationMaxCount = Resolver->ContactCount * 4;

    // Iteratively resolve interpenetrations in order of severity
    while (PositionIterationCount < PositionIterationMaxCount)
    {
        // Find biggest penetration
        MaxPenetration = Resolver->PositionEpsilon;
        MaxPenetrationIndex = Resolver->ContactCount;

        for (u32 ContactIndex = 0; ContactIndex < Resolver->ContactCount; ++ContactIndex)
        {
            contact *Contact = Resolver->Contacts + ContactIndex;

            if (Contact->Penetration > MaxPenetration)
            {
                MaxPenetration = Contact->Penetration;
                MaxPenetrationIndex = ContactIndex;
                MaxPenetrationContact = Contact;
            }
        }

        if (MaxPenetrationIndex == Resolver->ContactCount)
        {
            break;
        }

        // Match the awake state at the contact
        MatchAwakeState(MaxPenetrationContact);

        // Resolve the penetration
        ApplyPositionChange(MaxPenetrationContact, LinearChange, AngularChange, MaxPenetration);

        // Again this action may have changed the penetration of other bodies, so we update contacts
        for (u32 ContactIndex = 0; ContactIndex < Resolver->ContactCount; ++ContactIndex)
        {
            contact *Contact = Resolver->Contacts + ContactIndex;

            // Check each body in the contact
            for (u32 BodyIndex = 0; BodyIndex < 2; ++BodyIndex)
            {
                rigid_body *Body = Contact->Bodies[BodyIndex];

                if (Body)
                {
                    // Check for a match with each body in the newly resolved contact
                    for (u32 OtherBodyIndex = 0; OtherBodyIndex < 2; ++OtherBodyIndex)
                    {
                        if (Body == MaxPenetrationContact->Bodies[OtherBodyIndex])
                        {
                            DeltaPosition =
                                LinearChange[OtherBodyIndex] +
                                Cross(AngularChange[OtherBodyIndex], Contact->RelativeContactPositions[BodyIndex]);

                            // The sign of the change is positive if we're
                            // dealing with the second body in a contact
                            // and negative otherwise (because we're
                            // subtracting the resolution)..
                            Contact->Penetration += Dot(DeltaPosition, Contact->Normal) * (BodyIndex ? 1.f : -1.f);
                        }
                    }
                }
            }
        }

        PositionIterationCount += 1;
    }
}

dummy_internal void
AdjustVelocities(contact_resolver *Resolver, f32 dt)
{
    vec3 VelocityChange[2];
    vec3 RotationChange[2];

    vec3 DeltaVelocity;

    u32 VelocityIterationCount = 0;
    u32 VelocityIterationMaxCount = Resolver->ContactCount * 4;

    // Iteratively handle impacts in order of severity
    while (VelocityIterationCount < VelocityIterationMaxCount)
    {
        // Find contact with maximum magnitude of probable velocity change
        f32 MaxVelocity = Resolver->VelocityEpsilon;
        u32 MaxVelocityIndex = Resolver->ContactCount;
        contact *MaxVelocityContact = 0;

        for (u32 ContactIndex = 0; ContactIndex < Resolver->ContactCount; ++ContactIndex)
        {
            contact *Contact = Resolver->Contacts + ContactIndex;

            if (Contact->DesiredDeltaVelocity > MaxVelocity)
            {
                MaxVelocity = Contact->DesiredDeltaVelocity;
                MaxVelocityIndex = ContactIndex;
                MaxVelocityContact = Contact;
            }
        }

        if (MaxVelocityIndex == Resolver->ContactCount)
        {
            break;
        }

        // Match the awake state at the contact
        MatchAwakeState(MaxVelocityContact);

        // Do the resolution on the contact that came out top
        ApplyVelocityChange(MaxVelocityContact, VelocityChange, RotationChange);

        // With the change in velocity of the two bodies, the update of
        // contact velocities means that some of the relative closing
        // velocities need recomputing
        for (u32 ContactIndex = 0; ContactIndex < Resolver->ContactCount; ++ContactIndex)
        {
            contact *Contact = Resolver->Contacts + ContactIndex;

            // Check each body in the contact
            for (u32 BodyIndex = 0; BodyIndex < 2; ++BodyIndex)
            {
                rigid_body *Body = Contact->Bodies[BodyIndex];

                if (Body)
                {
                    // Check for a match with each body in the newly resolved contact
                    for (u32 OtherBodyIndex = 0; OtherBodyIndex < 2; ++OtherBodyIndex)
                    {
                        if (Body == MaxVelocityContact->Bodies[OtherBodyIndex])
                        {
                            DeltaVelocity =
                                VelocityChange[OtherBodyIndex] +
                                Cross(RotationChange[OtherBodyIndex], Contact->RelativeContactPositions[BodyIndex]);

                            // The sign of the change is negative if we're dealing
                            // with the second body in a contact
                            Contact->ContactVelocity += Contact->WorldToContact * DeltaVelocity * (BodyIndex ? -1.f : 1.f);
                            CalculateDesiredDeltaVelocity(Contact, dt);
                        }
                    }
                }
            }
        }

        VelocityIterationCount += 1;
    }
}

dummy_internal void
ResolveContacts(contact_resolver *Resolver, f32 dt)
{
    Assert(dt > 0.f);

    if (Resolver->ContactCount > 0)
    {
        // Prepare the contacts for processing
        PrepareContacts(Resolver, dt);

        // Resolve the interpenetration problems with the contacts
        AdjustPositions(Resolver, dt);

        // Resolve the velocity problems with the contacts
        AdjustVelocities(Resolver, dt);
    }
}
