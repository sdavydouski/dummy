#pragma once

// Axis-aligned bounding box
struct aabb
{
    vec3 Min;
    vec3 Max;
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
