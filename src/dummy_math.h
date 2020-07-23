#pragma once

#include <cmath>
#include <cstdarg>

struct quat;
inline f32 Square(f32 Value);
inline f32 Sqrt(f32 Value);

#include "dummy_vec2.h"
#include "dummy_vec3.h"
#include "dummy_vec4.h"
#include "dummy_mat4.h"
#include "dummy_quat.h"

#define PI 3.14159265359f
#define HALF_PI (PI / 2.f)

#define RADIANS(Angle) ((Angle) * PI) / 180.f

// todo: SIMD

struct transform
{
    quat Rotation;
    vec3 Translation;
    vec3 Scale;
};

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

inline f32
Power(f32 Value, f32 Power)
{
    f32 Result = powf(Value, Power);
    return Result;
}

inline f32
Sqrt(f32 Value)
{
    f32 Result = (f32)sqrt(Value);
    return Result;
}

inline f32
Sin(f32 Value)
{
    f32 Result = (f32)sin(Value);
    return Result;
}

inline f32
Cos(f32 Value)
{
    f32 Result = (f32)cos(Value);
    return Result;
}

inline f32
Tan(f32 Value)
{
    f32 Result = (f32)tan(Value);
    return Result;
}

inline f32
Acos(f32 Value)
{
    f32 Result = (f32)acos(Value);
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

inline f32
Clamp(f32 Value, f32 Min, f32 Max)
{
    if (Value < Min) return Min;
    if (Value > Max) return Max;

    return Value;
}

inline f32
Mod(f32 x, f32 y)
{
    f32 Result = fmod(x, y);
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
        vec4(Value, 0.f, 0.f, 0.f),
        vec4(0.f, Value, 0.f, 0.f),
        vec4(0.f, 0.f, Value, 0.f),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

inline mat4
Scale(vec3 Vector)
{
    mat4 Result = mat4(
        vec4(Vector.x, 0.f, 0.f, 0.f),
        vec4(0.f, Vector.y, 0.f, 0.f),
        vec4(0.f, 0.f, Vector.z, 0.f),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

inline mat4
Translate(vec3 Value)
{
    mat4 Result = mat4(
        vec4(1.f, 0.f, 0.f, Value.x),
        vec4(0.f, 1.f, 0.f, Value.y),
        vec4(0.f, 0.f, 1.f, Value.z),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

inline mat4
RotateX(f32 Angle)
{
    mat4 Result = mat4(
        vec4(1.f, 0.f, 0.f, 0.f),
        vec4(0.f, Cos(Angle), -Sin(Angle), 0.f),
        vec4(0.f, Sin(Angle), Cos(Angle), 0.f),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

inline mat4
RotateY(f32 Angle)
{
    mat4 Result = mat4(
        vec4(Cos(Angle), 0.f, Sin(Angle), 0.f),
        vec4(0.f, 1.f, 0.f, 0.f),
        vec4(-Sin(Angle), 0.f, Cos(Angle), 0.f),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

inline mat4
RotateZ(f32 Angle)
{
    mat4 Result = mat4(
        vec4(Cos(Angle), -Sin(Angle), 0.f, 0.f),
        vec4(Sin(Angle), Cos(Angle), 0.f, 0.f),
        vec4(0.f, 0.f, 1.f, 0.f),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

inline mat4
Orthographic(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far)
{
    mat4 Result = mat4(
        vec4(2.f / (Right - Left), 0.f, 0.f, -(Right + Left) / (Right - Left)),
        vec4(0.f, 2.f / (Top - Bottom), 0.f, -(Top + Bottom) / (Top - Bottom)),
        vec4(0.f, 0.f, -2.f / (Far - Near), -(Far + Near) / (Far - Near)),
        vec4(0.f, 0.f, 0.f, 1.f)
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
Perspective(f32 FovY, f32 Aspect, f32 Near, f32 Far)
{
    mat4 Result = mat4(
        vec4(1.f / (Aspect * Tan(FovY * 0.5f)), 0.f, 0.f, 0.f),
        vec4(0.f, 1.f / Tan(FovY / 2), 0.f, 0.f),
        vec4(0.f, 0.f, (-Near - Far) / (Far - Near), 2.f * (Near * Far) / (Near - Far)),
        vec4(0.f, 0.f, -1.f, 0.f)
    );

    return Result;
}

inline mat4
LookAtLH(vec3 Eye, vec3 Target, vec3 WorldUp)
{
    vec3 Forward = Normalize(Eye - Target);
    vec3 Right = Normalize(Cross(WorldUp, Forward));
    vec3 Up = Cross(Forward, Right);

    mat4 Result = mat4(
        vec4(Right, -Dot(Right, Eye)),
        vec4(Up, -Dot(Up, Eye)),
        vec4(Forward, -Dot(Forward, Eye)),
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

    return Result;
}