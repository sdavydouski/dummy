#pragma once

// Axis-Aligned Bounding Box
struct bounds
{
    vec3 Min;
    vec3 Max;
};

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
