#pragma once

enum collider_type
{
    Collider_Box
    // Sphere, Capsule, etc.
};

struct collider_box
{
    vec3 HalfSize;
    vec3 Offset;

    mat4 Transform;
};

struct collider
{
    collider_type Type;

    union
    {
        collider_box Box;
    };

    // todo: deprecated (or calculate on the fly instead of storing it?)
    aabb BoundsLocal;
    aabb BoundsWorld;
};

struct contact
{
    vec3 Point;
    vec3 Normal;
    f32 Penetration;
};

struct collision
{
    u32 ContactCount;
    u32 MaxContactCount;
    contact *Contacts;
};
