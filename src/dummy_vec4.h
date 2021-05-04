#pragma once

struct vec4
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };

        struct
        {
            f32 x;
            vec3 yzw;
        };

        struct
        {
            vec3 xyz;
            f32 w;
        };

        struct
        {
            vec2 xy;
            f32 z;
            f32 w;
        };

        struct
        {
            vec3 rgb;
            f32 a;
        };

        struct
        {
            f32 Elements[4];
        };
    };

    explicit vec4() = default;
    explicit vec4(f32 Value) : x(Value), y(Value), z(Value), w(Value) {}
    explicit vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    explicit vec4(f32 Value, vec3 Vector) : x(Value), y(Vector.x), z(Vector.y), w(Vector.z) {}
    explicit vec4(vec3 Vector, f32 Value) : x(Vector.x), y(Vector.y), z(Vector.z), w(Value) {}
    explicit vec4(vec2 xy, f32 z, f32 w) : x(xy.x), y(xy.y), z(z), w(w) {}

    inline f32 &operator [](u32 Index)
    {
        f32 &Result = Elements[Index];
        return Result;
    }

    inline vec4 operator *(f32 Scalar)
    {
        vec4 Result = vec4(Scalar * x, Scalar * y, Scalar * z, Scalar * w);
        return Result;
    }

    inline vec4 operator /(f32 Scalar)
    {
        vec4 Result = vec4(x, y, z, w) * (1.f / Scalar);
        return Result;
    }

    inline vec4 operator *(vec4 Vector)
    {
        vec4 Result = vec4(x * Vector.x, y * Vector.y, z * Vector.z, w * Vector.w);
        return Result;
    }

    inline vec4 operator +(vec4 Vector)
    {
        vec4 Result = vec4(x + Vector.x, y + Vector.y, z + Vector.z, w + Vector.w);
        return Result;
    }

    inline vec4 operator -(vec4 Vector)
    {
        vec4 Result = vec4(x - Vector.x, y - Vector.y, z - Vector.z, w - Vector.w);
        return Result;
    }

    inline vec4 operator -()
    {
        vec4 Result = vec4(-x, -y, -z, -w);
        return Result;
    }
};

inline vec4 &operator +=(vec4 &Dest, vec4 Vector)
{
    Dest = Dest + Vector;
    return Dest;
}

inline vec4 &operator -=(vec4 &Dest, vec4 Vector)
{
    Dest = Dest - Vector;
    return Dest;
}

inline f32
Dot(vec4 a, vec4 b)
{
    f32 Result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return Result;
}

inline f32
Magnitude(vec4 Vector)
{
    f32 Result = Sqrt(Square(Vector.x) + Square(Vector.y) + Square(Vector.z) + Square(Vector.w));
    return Result;
}

inline vec4
Normalize(vec4 Vector)
{
    f32 Length = Magnitude(Vector);

    Assert(Length > 0.f);

    vec4 Result = Vector / Length;

    return Result;
}
