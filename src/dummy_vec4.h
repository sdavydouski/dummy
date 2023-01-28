#pragma once

#include <xmmintrin.h>

// vec4
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

        struct
        {
            __m128 Row_4x;
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
#if 1
    f32 Result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
#else
    __m128 r1 = _mm_mul_ps(a.Row_4x, b.Row_4x);

    __m128 shuf = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(r1, shuf);

    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);

    f32 Result = _mm_cvtss_f32(sums);
#endif
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

// ivec4
struct ivec4
{
    union
    {
        struct
        {
            i32 x;
            i32 y;
            i32 z;
            i32 w;
        };

        struct
        {
            i32 Elements[4];
        };
    };

    explicit ivec4() = default;
    explicit ivec4(i32 Value) : x(Value), y(Value), z(Value), w(Value) {}
    explicit ivec4(i32 x, i32 y, i32 z, i32 w) : x(x), y(y), z(z), w(w) {}
};

inline bool32 operator ==(ivec4 a, ivec4 b)
{
    bool32 Result = (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
    return Result;
}

inline bool32 operator !=(ivec4 a, ivec4 b)
{
    bool32 Result = !(a == b);
    return Result;
}
