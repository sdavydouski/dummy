#pragma once

#include <cmath>
#include <cstdarg>

struct quat;
inline f32 Square(f32 Value);
inline f32 Sqrt(f32 Value);
inline f32 Abs(f32 Value);
inline f32 Min(f32 a, f32 b);
inline f32 Max(f32 a, f32 b);

#define EPSILON 0.0001f
#define PI 3.14159f
#define HALF_PI (PI / 2.f)
#define EULER 2.71828f

#define RADIANS(Angle) ((Angle) * PI) / 180.f
#define DEGREES(Angle) ((Angle) * 180.f) / PI

#include "dummy_vec2.h"
#include "dummy_vec3.h"
#include "dummy_vec4.h"
#include "dummy_mat4.h"
#include "dummy_quat.h"

struct plane
{
    vec3 Normal;
    f32 d;
};

struct triangle
{
    vec2 a;
    vec2 b;
    vec2 c;
};

struct circle
{
    vec2 Center;
    f32 Radius;
};

struct ray
{
    vec3 Origin;
    vec3 Direction;
};

struct transform
{
    quat Rotation;
    vec3 Translation;
    vec3 Scale;
};

struct quat_lerp
{
    f32 Duration;
    f32 Time;

    quat From;
    quat To;
};

struct vec3_lerp
{
    f32 Duration;
    f32 Time;

    vec3 From;
    vec3 To;
};

inline void
SetQuatLerp(quat_lerp *Lerp, f32 Time, f32 Duration, quat From, quat To)
{
    Lerp->Time = Time;
    Lerp->Duration = Duration;
    Lerp->From = From;
    Lerp->To = To;
}

inline void
SetVec3Lerp(vec3_lerp *Lerp, f32 Time, f32 Duration, vec3 From, vec3 To)
{
    Lerp->Time = Time;
    Lerp->Duration = Duration;
    Lerp->From = From;
    Lerp->To = To;
}

inline transform
CreateTransform(vec3 Translation, vec3 Scale, quat Rotation)
{
    transform Result;

    Result.Translation = Translation;
    Result.Scale = Scale;
    Result.Rotation = Rotation;

    return Result;
}

inline f32
Square(f32 Value)
{
    f32 Result = Value * Value;
    return Result;
}

inline u32
Square(u32 Value)
{
    u32 Result = Value * Value;
    return Result;
}

inline f32
Power(f32 Value, f32 Power)
{
    f32 Result = powf(Value, Power);
    return Result;
}

inline f32
Sqrt(f32 Value)
{
    f32 Result = sqrtf(Value);
    return Result;
}

inline f32
Sin(f32 Value)
{
    f32 Result = sinf(Value);
    return Result;
}

inline f32
Cos(f32 Value)
{
    f32 Result = cosf(Value);
    return Result;
}

inline f32
Tan(f32 Value)
{
    f32 Result = tanf(Value);
    return Result;
}

inline f32
Acos(f32 Value)
{
    f32 Result = acosf(Value);
    return Result;
}

inline f32
Atan2(f32 y, f32 x)
{
    f32 Result = atan2f(y, x);
    return Result;
}

inline i32
Abs(i32 Value)
{
    i32 Result = abs(Value);
    return Result;
}

inline f32
Abs(f32 Value)
{
    f32 Result = abs(Value);
    return Result;
}

inline vec3
Abs(vec3 Value)
{
    vec3 Result = vec3(Abs(Value.x), Abs(Value.y), Abs(Value.z));
    return Result;
}

inline i32
Ceil(f32 Value)
{
    i32 Result = (i32)ceil(Value);
    return Result;
}

inline i32 
Floor(f32 Value)
{
    i32 Result = (i32)floor(Value);
    return Result;
}

inline f32
Clamp(f32 Value, f32 Min, f32 Max)
{
    if (Value < Min) return Min;
    if (Value > Max) return Max;

    return Value;
}

inline void
Clamp(f32 *Value, f32 Min, f32 Max)
{
    if (*Value < Min) *Value = Min;
    if (*Value > Max) *Value = Max;
}

inline f32
SafeDivide(f32 Numerator, f32 Denominator)
{
    f32 Result = 0.f;

    if (Denominator != 0.f)
    {
        Result = Numerator / Denominator;
    }

    return Result;
}

inline f32
LogisticFunction(f32 L, f32 k, f32 x0, f32 x)
{
    f32 Result = L / (1.f + Power(EULER, -k * (x - x0)));

    return Result;
}

inline f32
Mod(f32 x, f32 y)
{
    f32 Result = fmod(x, y);
    return Result;
}

inline b32
InRange(f32 Value, f32 Min, f32 Max)
{
    b32 Result = Value >= Min && Value <= Max;
    return Result;
}

inline f32
Min(f32 a, f32 b)
{
    f32 Result = fmin(a, b);
    return Result;
}

inline f32
Min(i32 ArgCount, ...)
{
    std::va_list Args;
    va_start(Args, ArgCount);

    f32 Result = (f32)va_arg(Args, f64);

    for (i32 Index = 2; Index < ArgCount; ++Index)
    {
        f32 Value = (f32)va_arg(Args, f64);
        if (Value < Result)
        {
            Result = Value;
        }
    }

    va_end(Args);

    return Result;
}

inline f32
Max(f32 a, f32 b)
{
    f32 Result = fmax(a, b);
    return Result;
}

inline f32
Max(i32 ArgCount, ...)
{
    va_list Args;
    va_start(Args, ArgCount);

    f32 Result = (f32)va_arg(Args, f64);

    for (i32 Index = 2; Index < ArgCount; ++Index)
    {
        f32 Value = (f32)va_arg(Args, f64);
        if (Value > Result)
        {
            Result = Value;
        }
    }

    va_end(Args);

    return Result;
}

inline mat4
Scale(f32 Value)
{
    mat4 Result = mat4(
        Value, 0.f, 0.f, 0.f,
        0.f, Value, 0.f, 0.f,
        0.f, 0.f, Value, 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    return Result;
}

inline mat4
Scale(vec3 Value)
{
    mat4 Result = mat4(
        Value.x, 0.f, 0.f, 0.f,
        0.f, Value.y, 0.f, 0.f,
        0.f, 0.f, Value.z, 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    return Result;
}

inline mat4
Translate(vec3 Value)
{
    mat4 Result = mat4(
        1.f, 0.f, 0.f, Value.x,
        0.f, 1.f, 0.f, Value.y,
        0.f, 0.f, 1.f, Value.z,
        0.f, 0.f, 0.f, 1.f
    );

    return Result;
}

inline mat4
RotateX(f32 Angle)
{
    mat4 Result = mat4(
        1.f, 0.f, 0.f, 0.f,
        0.f, Cos(Angle), -Sin(Angle), 0.f,
        0.f, Sin(Angle), Cos(Angle), 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    return Result;
}

inline mat4
RotateY(f32 Angle)
{
    mat4 Result = mat4(
        Cos(Angle), 0.f, Sin(Angle), 0.f,
        0.f, 1.f, 0.f, 0.f,
        -Sin(Angle), 0.f, Cos(Angle), 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    return Result;
}

inline mat4
RotateZ(f32 Angle)
{
    mat4 Result = mat4(
        Cos(Angle), -Sin(Angle), 0.f, 0.f,
        Sin(Angle), Cos(Angle), 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    return Result;
}

inline mat4
Orthographic(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far)
{
    mat4 Result = mat4(
        2.f / (Right - Left), 0.f, 0.f, -(Right + Left) / (Right - Left),
        0.f, 2.f / (Top - Bottom), 0.f, -(Top + Bottom) / (Top - Bottom),
        0.f, 0.f, -2.f / (Far - Near), -(Far + Near) / (Far - Near),
        0.f, 0.f, 0.f, 1.f
    );

    return Result;
}

/**
* Projects view frustrum into the canonical view volume,
* where points inside view frustrum have normalized device
* coordinates in the range [-1, 1] along x, y and z axis
* after the perspective divide occurs.
*/
inline mat4
Perspective(f32 FovY, f32 AspectRatio, f32 Near, f32 Far)
{
    f32 FocalLength = 1.f / Tan(FovY * 0.5f);

    mat4 Result = mat4(
        FocalLength / AspectRatio, 0.f, 0.f, 0.f,
        0.f, FocalLength, 0.f, 0.f,
        0.f, 0.f, (-Near - Far) / (Far - Near), 2.f * (Near * Far) / (Near - Far),
        0.f, 0.f, -1.f, 0.f
    );

    return Result;
}

inline mat4
LookAt(vec3 Eye, vec3 Target, vec3 WorldUp)
{
    vec3 zAxis = Normalize(Eye - Target);
    vec3 xAxis = Normalize(Cross(WorldUp, zAxis));
    vec3 yAxis = Normalize(Cross(zAxis, xAxis));

    mat4 Result = mat4(
        vec4(xAxis, -Dot(xAxis, Eye)),
        vec4(yAxis, -Dot(yAxis, Eye)),
        vec4(zAxis, -Dot(zAxis, Eye)),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

inline vec3
CalculateDirectionFromEulerAngles(f32 Pitch, f32 Yaw)
{
    vec3 Result = vec3(1.f);

    Result.x = Cos(Yaw) * Cos(Pitch);
    Result.y = Sin(Pitch);
    Result.z = Sin(Yaw) * Cos(Pitch);

    Result = Normalize(Result);

    return Result;
}

inline mat4
Transform(transform Transform)
{
    mat4 T = Translate(Transform.Translation);
    mat4 R = GetRotationMatrix(Transform.Rotation);
    mat4 S = Scale(Transform.Scale);

    mat4 Result = T * R * S;

    return Result;
}

inline vec3
GetTranslation(mat4 M)
{
    vec3 Result = M.Column(3).xyz;

    return Result;
}

inline quat
AxisAngle2Quat(vec4 AxisAngle)
{
    quat Result;

    f32 Angle = AxisAngle.w;

    Result.x = AxisAngle.x * Sin(Angle / 2);
    Result.y = AxisAngle.y * Sin(Angle / 2);
    Result.z = AxisAngle.z * Sin(Angle / 2);
    Result.w = Cos(Angle / 2);

    return Result;
}

inline f32
Lerp(f32 A, f32 t, f32 B)
{
    f32 Result = A * (1 - t) + B * t;

    return Result;
}

inline vec2
Lerp(vec2 A, f32 t, vec2 B)
{
    vec2 Result;

    Result.x = Lerp(A.x, t, B.x);
    Result.y = Lerp(A.y, t, B.y);

    return Result;
}

inline vec3
Lerp(vec3 A, f32 t, vec3 B)
{
    vec3 Result;

    Result.x = Lerp(A.x, t, B.x);
    Result.y = Lerp(A.y, t, B.y);
    Result.z = Lerp(A.z, t, B.z);

    return Result;
}

inline quat
Lerp(quat A, f32 t, quat B)
{
    quat Result;

    Result.x = Lerp(A.x, t, B.x);
    Result.y = Lerp(A.y, t, B.y);
    Result.z = Lerp(A.z, t, B.z);
    Result.w = Lerp(A.w, t, B.w);

    Result = Normalize(Result);

    return Result;
}

inline quat
Slerp(quat A, f32 t, quat B)
{
    quat Result;

    quat NormalizedA = Normalize(A);
    quat NormalizedB = Normalize(B);

    f32 d = Dot(NormalizedA, NormalizedB);

    if (d < 0.f)
    {
        NormalizedA = -NormalizedA;
        d = -d;
    }

    f32 Threshold = 0.9995f;

    if (d < Threshold)
    {
        f32 Theta0 = Acos(d);
        f32 Theta = Theta0 * t;
        f32 SinTheta = Sin(Theta);
        f32 SinTheta0 = Sin(Theta0);

        f32 s0 = Cos(Theta) - d * SinTheta / SinTheta0;
        f32 s1 = SinTheta / SinTheta0;

        Result = (s0 * NormalizedA) + (s1 * NormalizedB);
    }
    else
    {
        Result = Lerp(NormalizedA, t, NormalizedB);
    }

    Assert(Abs(Magnitude(Result) - 1.f) < EPSILON);

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

/**
* Computes barycentric coordinates (u, v, w) for
* point p with respect to triangle (a, b, c)
*/
inline void
Barycentric(vec2 p, vec2 a, vec2 b, vec2 c, f32 &u, f32 &v, f32 &w)
{
    vec2 v0 = b - a;
    vec2 v1 = c - a;
    vec2 v2 = p - a;

    f32 d00 = Dot(v0, v0);
    f32 d01 = Dot(v0, v1);
    f32 d11 = Dot(v1, v1);
    f32 d20 = Dot(v2, v0);
    f32 d21 = Dot(v2, v1);
    
    f32 d = d00 * d11 - d01 * d01;

    v = (d11 * d20 - d01 * d21) / d;
    w = (d00 * d21 - d01 * d20) / d;
    u = 1.f - v - w;
}

inline vec3
UnprojectPoint(vec3 p, mat4 View, mat4 Projection)
{
    mat4 InvView = Inverse(View);
    mat4 InvProjection = Inverse(Projection);
    vec4 Point = vec4(p, 1.f);

    vec4 UnprojectedPoint = InvView * InvProjection * Point;
    vec3 Result = UnprojectedPoint.xyz / UnprojectedPoint.w;

    return Result;
}

inline void
Swap(f32 &a, f32 &b)
{
    f32 Temp = a;
    a = b;
    b = Temp;
}

// djb2 by Dan Bernstein
inline u64
Hash(char *String)
{
    u64 Hash = 5381;
    char Character;

    while (Character = *String++)
    {
        // Hash * 33 + Character
        Hash = ((Hash << 5) + Hash) + Character;
    }

    return Hash;
}

inline plane
ComputePlane(vec3 a, vec3 b, vec3 c)
{
    plane Result;
    Result.Normal = Normalize(Cross(b - a, c - a));
    Result.d = Dot(Result.Normal, a);

    return Result;
}

inline f32
Dot(plane Plane, vec3 Point)
{
    f32 Result = Dot(Plane.Normal, Point) + Plane.d;
    return Result;
}

inline vec3
Project(vec3 a, vec3 b)
{
    vec3 Result = b * (Dot(a, b) / Dot(b, b));
    return Result;
}
