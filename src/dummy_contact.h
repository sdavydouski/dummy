#pragma once

struct contact_params
{
    f32 Friction;
    f32 Restitution;
};

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

    // Features that are involved in the contact (point, edge or face)
    u32 Features[2];
};

struct contact_resolver
{
    f32 PositionEpsilon;
    f32 VelocityEpsilon;

    u32 ContactCount;
    u32 MaxContactCount;
    contact *Contacts;
};
