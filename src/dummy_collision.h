#pragma once

enum collider_type
{
    Collider_Box,
    Collider_Sphere
};

struct collider_box
{
    vec3 HalfSize;

    mat4 Offset;
    mat4 Transform;
};

struct collider_sphere
{
    f32 Radius;

    mat4 Offset;
    mat4 Transform;
};

struct collider
{
    collider_type Type;

    union
    {
        collider_box Box;
        collider_sphere Sphere;
    };

    aabb Bounds;
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
