#pragma once

struct plane
{
    union
    {
        struct
        {
            vec3 Normal;
            f32 Distance;
        };

        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 d;
        };
    };

    explicit plane() = default;
    explicit plane(vec3 Normal, f32 Distance) : Normal(Normal), Distance(Distance) {}
    explicit plane(f32 x, f32 y, f32 z, f32 d) : x(x), y(y), z(z), d(d) {}
};

inline plane
ComputePlane(vec3 a, vec3 b, vec3 c)
{
    plane Result;
    Result.Normal = Normalize(Cross(b - a, c - a));
    Result.Distance = Dot(Result.Normal, a);

    return Result;
}

inline f32
DotVector(plane Plane, vec3 Vector)
{
    f32 Result = Dot(Plane.Normal, Vector);
    return Result;
}

inline f32
DotPoint(plane Plane, vec3 Point)
{
    f32 Result = Dot(Plane.Normal, Point) + Plane.Distance;
    return Result;
}

inline f32
Dot(plane Plane, vec4 Vector)
{
    f32 Result = Dot(vec4(Plane.Normal, Plane.Distance), Vector);
    return Result;
}

inline vec3
Projection(vec3 Vector, plane Plane)
{
    vec3 Result = Vector - Dot(Vector, Plane.Normal) * Plane.Normal;
    return Result;
}

inline vec3
Orthogonal(vec3 Vector, plane Plane)
{
    vec3 Result = Cross(Vector, Plane.Normal);
    return Result;
}

inline bool32
IntersectLinePlane(vec3 LineOrigin, vec3 LineDirection, plane Plane, vec3 *IntersectionPoint)
{
    f32 fv = DotVector(Plane, LineDirection);
    if (Abs(fv) > F32_MIN)
    {
        *IntersectionPoint = LineOrigin - LineDirection * (DotPoint(Plane, LineOrigin) / fv);
        return true;
    }

    return false;
}

/**
* Input matrix needs to be inverted before being passed to this function
* in order to calculate the correct transformation
*/
inline plane
Transform(plane Plane, mat4 Inverted)
{
    plane Result;

    Result.Normal.x = Dot(Plane.Normal, Inverted.Column(0).xyz);
    Result.Normal.y = Dot(Plane.Normal, Inverted.Column(1).xyz);
    Result.Normal.z = Dot(Plane.Normal, Inverted.Column(2).xyz);
    Result.Distance = Dot(Plane.Normal, Inverted.Column(3).xyz) + Plane.d;

    return Result;
}
