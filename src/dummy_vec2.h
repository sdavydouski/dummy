#pragma once

struct vec2
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
        };

        struct
        {
            f32 r;
            f32 g;
        };

        struct
        {
            f32 Elements[2];
        };
    };

    explicit vec2() = default;
    explicit vec2(f32 Value) : x(Value), y(Value) {}
    explicit vec2(f32 x, f32 y) : x(x), y(y) {}

    inline f32 operator [](u32 Index)
    {
        f32 Result = Elements[Index];
        return Result;
    }

    inline vec2 operator *(f32 Scalar)
    {
        vec2 Result = vec2(Scalar * x, Scalar * y);
        return Result;
    }

    inline vec2 operator /(f32 Scalar)
    {
        vec2 Result = vec2(x, y) * (1.f / Scalar);
        return Result;
    }

    inline vec2 operator *(vec2 Vector)
    {
        vec2 Result = vec2(x * Vector.x, y * Vector.y);
        return Result;
    }

    inline vec2 operator +(vec2 Vector)
    {
        vec2 Result = vec2(x + Vector.x, y + Vector.y);
        return Result;
    }

    inline vec2 operator -(vec2 Vector)
    {
        vec2 Result = vec2(x - Vector.x, y - Vector.y);
        return Result;
    }

    inline vec2 operator -()
    {
        vec2 Result = vec2(-x, -y);
        return Result;
    }
};

inline vec2 &operator +=(vec2 &Dest, vec2 Vector)
{
    Dest = Dest + Vector;
    return Dest;
}

inline vec2 &operator -=(vec2 &Dest, vec2 Vector)
{
    Dest = Dest - Vector;
    return Dest;
}

inline f32
Dot(vec2 a, vec2 b)
{
    f32 Result = a.x * b.x + a.y * b.y;
    return Result;
}

inline f32
Magnitude(vec2 Vector)
{
    f32 Result = Sqrt(Square(Vector.x) + Square(Vector.y));
    return Result;
}

inline vec2
Normalize(vec2 Vector)
{
    vec2 Result = Vector / Magnitude(Vector);
    return Result;
}

inline vec2
PerpendicularCW(vec2 Vector)
{
    vec2 Result = vec2(Vector.y, -Vector.x);
    return Result;
}

inline vec2
PerpendicularCCW(vec2 Vector)
{
    vec2 Result = vec2(-Vector.y, Vector.x);
    return Result;
}