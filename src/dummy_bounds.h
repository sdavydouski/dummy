#pragma once

// Axis-aligned bounding box
struct aabb
{
    vec3 Center;
    vec3 HalfExtent;

    inline vec3 Min()
    {
        vec3 Min = Center - HalfExtent;
        return Min;
    }

    inline vec3 Max()
    {
        vec3 Max = Center + HalfExtent;
        return Max;
    }
};

// Oriented bounding box
struct obb
{
    vec3 Center;
    vec3 HalfExtent;

    union
    {
        struct
        {
            vec3 AxisX;
            vec3 AxisY;
            vec3 AxisZ;
        };

        vec3 Axis[3];
    };
};
