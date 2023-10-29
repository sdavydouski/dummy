#pragma once

struct box_collider
{
    vec3 Center;
    vec3 Size;
};

struct sphere_collider
{
    vec3 Center;
    f32 Radius;
};

enum collider_type
{
    Collider_Box,
    Collider_Sphere
};

struct collider
{
    collider_type Type;

    union
    {
        box_collider BoxCollider;
        sphere_collider SphereCollider;
    };
};

// todo(continue): contact generation
struct contact
{
    vec3 Point;
    vec3 Normal;
    f32 Penetration;
};

struct collision
{
    u32 ContactCount;
    contact *Contacts;
};
