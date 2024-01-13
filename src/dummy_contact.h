#pragma once

struct contact
{
    vec3 Point;
    vec3 Normal;
    f32 Penetration;

    f32 Friction;
    f32 Restitution;

    mat3 ContactToWorld;
    mat3 WorldToContact;

    // Closing velocity at the point of contact
    vec3 ContactVelocity;

    // Required change in velocity for this contact to be resolved
    f32 DesiredDeltaVelocity;

    // Bodies that are involved in the contact
    rigid_body *Bodies[2];

    // World space position of the contact point relative to center of each body
    vec3 RelativeContactPositions[2];
};

struct contact_manifold
{
    rigid_body *Bodies[2];
    contact Contacts[4];
    u32 ContactCount;
};

struct contact_params
{
    f32 Friction;
    f32 Restitution;
};

struct contact_resolver
{
    u32 ContactCount;
    u32 MaxContactCount;
    contact *Contacts;

    f32 PositionEpsilon;
    f32 VelocityEpsilon;
};
