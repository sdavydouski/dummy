#pragma once

struct contact
{
    vec3 Point;
    vec3 Normal;
    f32 Penetration;

    f32 Friction;
    f32 Restitution;

    rigid_body *Bodies[2];
    vec3 RelativeContactPositions[2];

    mat3 ContactToWorld;
    mat3 WorldToContact;

    vec3 ContactVelocity;
    f32 DesiredDeltaVelocity;
};

struct contact_resolver
{
    u32 ContactCount;
    u32 MaxContactCount;
    contact *Contacts;
};
