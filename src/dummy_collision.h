#pragma once

enum collider_type
{
    Collider_Box
    // Sphere, Capsule, etc.
};

struct collider
{
    collider_type Type;

    union
    {
        struct
        {
            aabb BoxLocal;
            aabb BoxWorld;
        };
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
