#pragma once

enum collider_type
{
    Collider_Box,
    Collider_Sphere
};

struct collider_box
{
    vec3 HalfSize;

    // todo: move this to collider struct?
    mat4 Offset;
    mat4 Transform;
    rigid_body *Body;
};

struct collider_sphere
{
    f32 Radius;

    // todo: move this to collider struct?
    mat4 Offset;
    mat4 Transform;
    rigid_body *Body;
};

struct collider
{
    collider_type Type;

    union
    {
        collider_box Box;
        collider_sphere Sphere;
    };

    // todo: do I need to store it?
    aabb Bounds;
};
