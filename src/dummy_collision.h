#pragma once

struct aabb
{
    vec3 Min;
    vec3 Max;
};

struct obb
{
    vec3 Center;
    vec3 HalfSize;
    vec3 Axes[3];
};
