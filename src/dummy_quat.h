#pragma once

struct quat
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
            f32 Elements[4];
        };
    };

    explicit quat() = default;
    explicit quat(f32 Value) : x(Value), y(Value), z(Value), w(Value) {}
    explicit quat(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    explicit quat(vec3 v, f32 s) : x(v.x), y(v.y), z(v.z), w(s) {}

    inline quat operator *(quat q)
    {
        quat Result = quat(
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        );
        return Result;
    }

    inline quat operator *(f32 Scalar)
    {
        quat Result = quat(Scalar * x, Scalar * y, Scalar * z, Scalar * w);
        return Result;
    }

    inline quat operator /(f32 Scalar)
    {
        quat Result = quat(x, y, z, w) * (1.f / Scalar);
        return Result;
    }

    inline quat operator +(quat q)
    {
        quat Result = quat(x + q.x, y + q.y, z + q.z, w + q.w);
        return Result;
    }

    inline quat operator -(quat q)
    {
        quat Result = quat(x - q.x, y - q.y, z - q.z, w - q.w);
        return Result;
    }

    inline quat operator -()
    {
        quat Result = quat(-x, -y, -z, -w);
        return Result;
    }
};

inline quat &operator +=(quat &Dest, quat q)
{
    Dest = Dest + q;
    return Dest;
}

inline quat &operator -=(quat &Dest, quat q)
{
    Dest = Dest - q;
    return Dest;
}

inline quat operator *(f32 Value, quat q)
{
    quat Result = q * Value;
    return Result;
}

inline f32
Dot(quat a, quat b)
{
    f32 Result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return Result;
}

inline f32
Magnitude(quat q)
{
    f32 Result = Sqrt(Square(q.x) + Square(q.y) + Square(q.z) + Square(q.w));
    return Result;
}

inline quat
Normalize(quat q)
{
    f32 Length = Magnitude(q);

    Assert(Length > 0.f);

    quat Result = q / Length;

    return Result;
}

inline mat4
GetRotationMatrix(quat q)
{
    f32 x2 = Square(q.x);
    f32 y2 = Square(q.y);
    f32 z2 = Square(q.z);
    f32 xy = q.x * q.y;
    f32 xz = q.x * q.z;
    f32 yz = q.y * q.z;
    f32 wx = q.w * q.x;
    f32 wy = q.w * q.y;
    f32 wz = q.w * q.z;

    mat4 Result = mat4(
        vec4(1.f - 2.f * (y2 + z2), 2.f * (xy - wz), 2.f * (xz + wy), 0.f),
        vec4(2.f * (xy + wz), 1.f - 2.f * (x2 + z2), 2.f * (yz - wx), 0.f),
        vec4(2.f * (xz - wy), 2.f * (yz + wx), 1.f - 2.f * (x2 + y2), 0.f),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

inline vec3
Rotate(vec3 v, quat q)
{
    vec3 b = vec3(q.x, q.y, q.z);
    f32 b2 = Square(b.x) + Square(b.y) + Square(b.z);

    vec3 Result = (v * (Square(q.w) - b2) + b * (Dot(v, b) * 2.f) + Cross(b, v) * q.w * 2.f);

    return Result;
}