#pragma once

#include <cmath>
#include <cstdarg>

#define EPSILON 0.00001f
#define PI 3.14159f
#define HALF_PI (PI / 2.f)
#define EULER 2.71828f

inline f32 Square(f32 Value);
inline f32 Sqrt(f32 Value);
inline f32 Abs(f32 Value);
inline f32 Min(f32 a, f32 b);
inline f32 Max(f32 a, f32 b);
inline bool32 NearlyEqual(f32 a, f32 b, f32 Epsilon = EPSILON);
inline bool32 IsFinite(f32 n);

#define RADIANS(Angle) ((Angle) * PI) / 180.f
#define DEGREES(Angle) ((Angle) * 180.f) / PI

#include "dummy_vec2.h"
#include "dummy_vec3.h"
#include "dummy_vec4.h"
#include "dummy_mat3.h"
#include "dummy_mat4.h"
#include "dummy_quat.h"
#include "dummy_plane.h"

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
    u32 ProcessId;

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
    f32 Result = fabs(Value);
    return Result;
}

inline vec3
Abs(vec3 Value)
{
    vec3 Result = vec3(Abs(Value.x), Abs(Value.y), Abs(Value.z));
    return Result;
}

inline i32
Sign(f32 Value)
{
    i32 Result = 0;

    if (Value > 0.f)
    {
        Result = 1;
    }
    else if (Value < 0.f)
    {
        Result = -1;
    }

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
NormalizeRange(f32 Value, f32 RangeMin, f32 RangeMax)
{
    f32 Result = (Value - RangeMin) / (RangeMax - RangeMin);
    return Result;
}

inline bool32
NearlyEqual(f32 a, f32 b, f32 Epsilon)
{
    bool32 Result = Abs(a - b) < Epsilon;
    return Result;
}

inline bool32
IsFinite(f32 n)
{
    bool32 Result = isfinite(n);
    return Result;
}

inline bool32
IsFinite(transform Transform)
{
    bool32 Result = IsFinite(Transform.Rotation) && IsFinite(Transform.Translation) && IsFinite(Transform.Scale);
    return Result;
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

inline bool32
InRange(f32 Value, f32 Min, f32 Max)
{
    bool32 Result = Value >= Min && Value <= Max;
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

inline i32
Min(i32 a, i32 b)
{
    i32 Result = a <= b ? a : b;
    return Result;
}

inline i32
Max(i32 a, i32 b)
{
    i32 Result = a > b ? a : b;
    return Result;
}

inline bool32
IsPrime(u32 n)
{
    if (n == 2 || n == 3)
    {
        return true;
    }

    if (n <= 1 || n % 2 == 0 || n % 3 == 0)
    {
        return false;
    }

    for (u32 i = 5; i * i <= n; i += 6)
    {
        if (n % i == 0 || n % (i + 2) == 0)
        {
            return false;
        }
    }

    return true;
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
OrthographicProjection(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far)
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
* http://www.songho.ca/opengl/gl_projectionmatrix.html <-- todo: switch from this
*/
inline mat4
FrustrumProjection(f32 FovY, f32 AspectRatio, f32 Near, f32 Far)
{
    f32 FocalLength = 1.f / Tan(FovY * 0.5f);

    mat4 Result = mat4(
        FocalLength / AspectRatio, 0.f, 0.f, 0.f,
        0.f, FocalLength, 0.f, 0.f,
        0.f, 0.f, -(Near + Far) / (Far - Near), (-2.f * Near * Far) / (Far - Near),
        0.f, 0.f, -1.f, 0.f
    );

    return Result;
}

inline mat4
LookAt(vec3 Position, vec3 Target, vec3 Up)
{
    vec3 zAxis = Normalize(Position - Target);
    vec3 xAxis = Normalize(Cross(Up, zAxis));
    vec3 yAxis = Normalize(Cross(zAxis, xAxis));

    mat4 Result = mat4(
        vec4(xAxis, -Dot(xAxis, Position)),
        vec4(yAxis, -Dot(yAxis, Position)),
        vec4(zAxis, -Dot(zAxis, Position)),
        vec4(0.f, 0.f, 0.f, 1.f)
    );

    return Result;
}

// todo: Roll?
inline vec3
EulerToDirection(f32 Yaw, f32 Pitch)
{
    vec3 Result = vec3(1.f);

    Result.x = Cos(Yaw) * Cos(Pitch);
    Result.y = Sin(Pitch);
    Result.z = Sin(Yaw) * Cos(Pitch);

    Result = Normalize(Result);

    return Result;
}

// todo: change ther order!
// Yaw (Z), Pitch (Y), Roll (X)
inline quat
EulerToQuat(f32 Yaw, f32 Pitch, f32 Roll)
{
    f32 cy = Cos(Yaw * 0.5f);
    f32 sy = Sin(Yaw * 0.5f);
    f32 cp = Cos(Pitch * 0.5f);
    f32 sp = Sin(Pitch * 0.5f);
    f32 cr = Cos(Roll * 0.5f);
    f32 sr = Sin(Roll * 0.5f);

    quat Result;
    Result.x = sr * cp * cy - cr * sp * sy;
    Result.y = cr * sp * cy + sr * cp * sy;
    Result.z = cr * cp * sy - sr * sp * cy;
    Result.w = cr * cp * cy + sr * sp * sy;

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

inline transform
Decompose(mat4 M)
{
    vec3 Column0 = M.Column(0).xyz;
    vec3 Column1 = M.Column(1).xyz;
    vec3 Column2 = M.Column(2).xyz;
    vec3 Translation = M.Column(3).xyz;

    vec3 Scale = vec3(Magnitude(Column0), Magnitude(Column1), Magnitude(Column2));

    vec3 NormalizedColumn0 = Column0 / Scale.x;
    vec3 NormalizedColumn1 = Column1 / Scale.y;
    vec3 NormalizedColumn2 = Column2 / Scale.z;

    mat4 RotationM = mat4(
        NormalizedColumn0.x, NormalizedColumn1.x, NormalizedColumn2.x, 0.f,
        NormalizedColumn0.y, NormalizedColumn1.y, NormalizedColumn2.y, 0.f,
        NormalizedColumn0.z, NormalizedColumn1.z, NormalizedColumn2.z, 0.f,
        0.f, 0.f, 0.f, 1.f
    );
    quat Rotation = LookRotation(RotationM);

    transform Result = {};

    Result.Translation = Translation;
    Result.Rotation = Rotation;
    Result.Scale = Scale;

    return Result;
}

inline transform
ParentTransform(transform Local, transform Parent)
{
    mat4 LocalM = Transform(Local);
    mat4 ParentM = Transform(Parent);
    mat4 ResultM = LocalM * ParentM;

    transform Result = Decompose(ResultM);

    return Result;
}

inline vec3
GetTranslation(mat4 M)
{
    vec3 Result = M.Column(3).xyz;

    return Result;
}

inline mat4
RemoveTranslation(mat4 M)
{
    mat4 Result = mat4(M);

    Result[0].w = 0.f;
    Result[1].w = 0.f;
    Result[2].w = 0.f;
    Result[3].w = 1.f;

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

inline quat
AxisAngle2Quat(vec3 Axis, f32 Angle)
{
    quat Result = AxisAngle2Quat(vec4(Axis.x, Axis.y, Axis.z, Angle));

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

    Assert(IsNormalized(Result));

    return Result;
}

/*
    Computes barycentric coordinates (u, v, w) for
    point p with respect to triangle (a, b, c)
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

/*
    Right-handed Cartesian coordinates
    
        y
        |  
        |
        | 
        |
        + ------------ x
       /
      /
     /
    z

    SphericalCoords.x - Radial Distance
    SphericalCoords.y - Azimuth
    SphericalCoords.z - Altitude

*/
inline vec3
SphericalToCartesian(vec3 SphericalCoords)
{
    vec3 Result;

    Result.x = SphericalCoords.x * Sin(HALF_PI - SphericalCoords.z) * Sin(SphericalCoords.y);
    Result.y = SphericalCoords.x * Cos(HALF_PI - SphericalCoords.z);
    Result.z = SphericalCoords.x * Sin(HALF_PI - SphericalCoords.z) * Cos(SphericalCoords.y);
    
    return Result;
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
constexpr u64
Hash(const char *String)
{
    u64 Hash = 5381;

    while (char Character = *String++)
    {
        Hash = ((Hash << 5) + Hash) + Character;
    }

    return Hash;
}

constexpr u32
Hash(const u32 Key)
{
    u32 Hash = Key;

#if 0
    Hash = (Hash ^ 61) ^ (Hash >> 16);
    Hash = Hash + (Hash << 3);
    Hash = Hash ^ (Hash >> 4);
    Hash = Hash * 0x27d4eb2d;
    Hash = Hash ^ (Hash >> 15);
#else
    Hash = (Hash + 0x7ed55d16) + (Hash << 12);
    Hash = (Hash ^ 0xc761c23c) ^ (Hash >> 19);
    Hash = (Hash + 0x165667b1) + (Hash << 5);
    Hash = (Hash + 0xd3a2646c) ^ (Hash << 9);
    Hash = (Hash + 0xfd7046c5) + (Hash << 3);
    Hash = (Hash ^ 0xb55a4f09) ^ (Hash >> 16);
#endif

    return Hash;
}

inline vec3
Project(vec3 a, vec3 b)
{
    vec3 Result = b * (Dot(a, b) / Dot(b, b));
    return Result;
}
