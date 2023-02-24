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

inline quat operator *(quat q1, quat q2)
{
    quat Result = quat(
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
        q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
    );
    return Result;
}

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
MagnitudeSquared(quat q)
{
    f32 Result = Square(q.x) + Square(q.y) + Square(q.z) + Square(q.w);
    return Result;
}

inline f32
Magnitude(quat q)
{
    f32 Result = Sqrt(MagnitudeSquared(q));
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

inline bool32
IsNormalized(quat q)
{
    f32 Length = Magnitude(q);
    bool32 Result = NearlyEqual(Length, 1.f);

    return Result;
}

inline bool32
IsFinite(quat q)
{
    bool32 Result = IsFinite(q.x) && IsFinite(q.y) && IsFinite(q.z) && IsFinite(q.w);
    return Result;
}

inline quat
Inverse(quat q)
{
    // todo:
#if 0
    quat Result = quat(-q.x, -q.y, -q.z, q.w);
#else
    quat Result = quat(-q.x, -q.y, -q.z, q.w) / MagnitudeSquared(q);
#endif
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

inline quat
LookRotation(mat4 M)
{
    quat Result;

    f32 m00 = M[0][0];
    f32 m11 = M[1][1];
    f32 m22 = M[2][2];
    f32 sum = m00 + m11 + m22;

    if (sum > 0.f)
    {
        f32 w = Sqrt(sum + 1.f) + 0.5f;
        f32 f = 0.25f / w;

        Result.x = (M[2][1] - M[1][2]) * f;
        Result.y = (M[0][2] - M[2][0]) * f;
        Result.z = (M[1][0] - M[0][1]) * f;
        Result.w = w;
    }
    else if ((m00 > m11) && (m00 > m22))
    {
        f32 x = Sqrt(m00 - m11 - m22 + 1.f) * 0.5f;
        f32 f = 0.25f / x;

        Result.x = x;
        Result.y = (M[1][0] + M[0][1]) * f;
        Result.z = (M[0][2] + M[2][0]) * f;
        Result.w = (M[2][1] - M[1][2]) * f;
    }
    else if (m11 > m22)
    {
        f32 y = Sqrt(m11 - m00 - m22 + 1.f) * 0.5f;
        f32 f = 0.25f / y;

        Result.x = (M[1][0] + M[0][1]) * f;
        Result.y = y;
        Result.z = (M[2][1] + M[1][2]) * f;
        Result.w = (M[0][2] - M[2][0]) * f;
    }
    else
    {
        f32 z = Sqrt(m22 - m00 - m11 + 1.f) * 0.5f;
        f32 f = 0.25f / z;

        Result.x = (M[0][2] + M[2][0]) * f;
        Result.y = (M[2][1] + M[1][2]) * f;
        Result.z = z;
        Result.w = (M[1][0] - M[0][1]) * f;
    }

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