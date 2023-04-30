#pragma once

// vec3
struct vec3
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
        };

        struct
        {
            vec2 xy;
            f32 z;
        };

        struct
        {
            f32 Elements[3];
        };
    };

    explicit vec3() = default;
    explicit vec3(f32 Value) : x(Value), y(Value), z(Value) {}
    explicit vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
    explicit vec3(vec2 xy, f32 z) : x(xy.x), y(xy.y), z(z) {}

    inline f32 &operator [](u32 Index)
    {
        f32 &Result = Elements[Index];
        return Result;
    }

    inline vec3 operator -()
    {
        vec3 Result = vec3(-x, -y, -z);
        return Result;
    }

    inline vec3 operator *(f32 Scalar)
    {
        vec3 Result = vec3(Scalar * x, Scalar * y, Scalar * z);
        return Result;
    }

    inline vec3 operator /(f32 Scalar)
    {
        vec3 Result = vec3(x, y, z) * (1.f / Scalar);
        return Result;
    }

    inline vec3 operator *(vec3 Vector)
    {
        vec3 Result = vec3(x * Vector.x, y * Vector.y, z * Vector.z);
        return Result;
    }

    inline vec3 operator /(vec3 Vector)
    {
        vec3 Result = vec3(x / Vector.x, y / Vector.y, z / Vector.z);
        return Result;
    }

    inline vec3 operator +(vec3 Vector)
    {
        vec3 Result = vec3(x + Vector.x, y + Vector.y, z + Vector.z);
        return Result;
    }

    inline vec3 operator -(vec3 Vector)
    {
        vec3 Result = vec3(x - Vector.x, y - Vector.y, z - Vector.z);
        return Result;
    }

    inline bool32 operator <(vec3 Vector)
    {
        bool32 Result = x < Vector.x && y < Vector.y && z < Vector.z;
        return Result;
    }

    inline bool32 operator >(vec3 Vector)
    {
        bool32 Result = x > Vector.x &&y > Vector.y &&z > Vector.z;
        return Result;
    }
};

inline bool32 operator ==(vec3 a, vec3 b)
{
    bool32 Result = a.x == b.x && a.y == b.y && a.z == b.z;
    return Result;
}

inline bool32 operator !=(vec3 a, vec3 b)
{
    bool32 Result = !(a == b);
    return Result;
}

inline vec3 &operator +=(vec3 &Dest, vec3 Vector)
{
    Dest = Dest + Vector;
    return Dest;
}

inline vec3 &operator -=(vec3 &Dest, vec3 Vector)
{
    Dest = Dest - Vector;
    return Dest;
}

inline vec3 operator *(f32 Value, vec3 Vector)
{
    vec3 Result = Vector * Value;
    return Result;
}

inline vec3 &operator *=(vec3 &Dest, f32 Value)
{
    Dest = Dest * Value;
    return Dest;
}

inline vec3 &operator *=(vec3 &Dest, vec3 Vector)
{
    Dest = Dest * Vector;
    return Dest;
}

inline bool32 operator <=(vec3 a, vec3 b)
{
    bool32 Result = a < b || a == b;
    return Result;
}

inline bool32 operator >=(vec3 a, vec3 b)
{
    bool32 Result = a > b || a == b;
    return Result;
}

inline f32
Dot(vec3 a, vec3 b)
{
    f32 Result = a.x * b.x + a.y * b.y + a.z * b.z;
    return Result;
}

inline vec3
Cross(vec3 a, vec3 b)
{
    vec3 Result = vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
    return Result;
}

inline f32
Magnitude(vec3 Vector)
{
    f32 Result = Sqrt(Square(Vector.x) + Square(Vector.y) + Square(Vector.z));
    return Result;
}

inline f32
SquaredMagnitude(vec3 Vector)
{
    f32 Result = Square(Vector.x) + Square(Vector.y) + Square(Vector.z);
    return Result;
}

inline vec3
Normalize(vec3 Vector)
{
    f32 Length = Magnitude(Vector);

    Assert(Length > 0.f);

    vec3 Result = Vector / Length;

    return Result;
}

inline vec3
Min(vec3 a, vec3 b)
{
    vec3 Result = vec3(Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z));
    return Result;
}

inline vec3
Max(vec3 a, vec3 b)
{
    vec3 Result = vec3(Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z));
    return Result;
}

inline bool32
IsFinite(vec3 v)
{
    bool32 Result = IsFinite(v.x) && IsFinite(v.y) && IsFinite(v.z);
    return Result;
}

inline vec3
SafeReciprocal(vec3 v)
{
    vec3 Result = vec3(0.f);

    if (!NearlyEqual(v.x, 0.f))
    {
        Result.x = 1.f / v.x;
    }

    if (!NearlyEqual(v.y, 0.f))
    {
        Result.y = 1.f / v.y;
    }

    if (!NearlyEqual(v.z, 0.f))
    {
        Result.z = 1.f / v.z;
    }

    return Result;
}

inline vec3
Perpendicular(vec3 v)
{
    vec3 Result;

    f32 x = Abs(v.x);
    f32 y = Abs(v.y);
    f32 z = Abs(v.z);

    if (z < Min(x, y))
    {
        Result = vec3(v.y, -v.x, 0.f);
    }
    else if (y < x)
    {
        Result = vec3(-v.z, 0.f, v.x);
    }
    else
    {
        Result = vec3(0.f, v.z, -v.y);
    }

    return Result;
}


// ivec3
struct ivec3
{
    union
    {
        struct
        {
            i32 x;
            i32 y;
            i32 z;
        };

        struct
        {
            i32 Elements[3];
        };
    };

    explicit ivec3() = default;
    explicit ivec3(i32 Value) : x(Value), y(Value), z(Value) {}
    explicit ivec3(i32 x, i32 y, i32 z) : x(x), y(y), z(z) {}
};

inline bool32 operator ==(ivec3 a, ivec3 b)
{
    bool32 Result = (a.x == b.x && a.y == b.y && a.z == b.z);
    return Result;
}

inline bool32 operator !=(ivec3 a, ivec3 b)
{
    bool32 Result = !(a == b);
    return Result;
}
