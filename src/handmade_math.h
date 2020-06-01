#pragma once

#include <cmath>
#include <cstdarg>

struct quat;
inline f32 Square(f32 Value);
inline f32 Sqrt(f32 Value);

#include "handmade_vec2.h"
#include "handmade_vec3.h"
#include "handmade_vec4.h"
#include "handmade_mat4.h"
#include "handmade_quat.h"

#define PI 3.14159265359f
#define HALF_PI (PI / 2.f)

#define RADIANS(Angle) ((Angle) * PI) / 180.f

// todo: SIMD

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
	mat4 Result = mat4(1.f);

	Result[0][0] = Value;
	Result[1][1] = Value;
	Result[2][2] = Value;

	return Result;
}

inline mat4
Scale(vec3 Vector)
{
	mat4 Result = mat4(1.f);

	Result[0][0] = Vector.x;
	Result[1][1] = Vector.y;
	Result[2][2] = Vector.z;

	return Result;
}

inline mat4
Translate(vec3 Value)
{
	mat4 Result = mat4(1.f);

	Result[0][3] = Value.x;
	Result[1][3] = Value.y;
	Result[2][3] = Value.z;

	return Result;
}

inline mat4
RotateX(f32 Angle)
{
	mat4 Result = mat4(1.f);

	Result[1][1] = Cos(Angle);
	Result[1][2] = -Sin(Angle);
	Result[2][1] = Sin(Angle);
	Result[2][2] = Cos(Angle);

	return Result;
}

inline mat4
RotateY(f32 Angle)
{
	mat4 Result = mat4(1.f);

	Result[0][0] = Cos(Angle);
	Result[0][2] = Sin(Angle);
	Result[2][0] = -Sin(Angle);
	Result[2][2] = Cos(Angle);

	return Result;
}

inline mat4
RotateZ(f32 Angle)
{
	mat4 Result = mat4(1.f);

	Result[0][0] = Cos(Angle);
	Result[0][1] = -Sin(Angle);
	Result[1][0] = Sin(Angle);
	Result[1][1] = Cos(Angle);

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
#if 0
	mat4 Result = mat4(
		vec4(1.f / (Aspect * Tan(FovY * 0.5f)), 0.f, 0.f, 0.f),
		vec4(0.f, 1.f / Tan(FovY / 2), 0.f, 0.f),
		vec4(0.f, 0.f, (-Near - Far) / (Near - Far), 2.f * (Near * Far) / (Near - Far)),
		vec4(0.f, 0.f, 1.f, 0.f)
	);
#else
	mat4 Result = mat4(
		vec4(1.f / (Aspect * Tan(FovY * 0.5f)), 0.f, 0.f, 0.f),
		vec4(0.f, 1.f / Tan(FovY / 2), 0.f, 0.f),
		vec4(0.f, 0.f, (-Near - Far) / (Far - Near), 2.f * (Near * Far) / (Near - Far)),
		vec4(0.f, 0.f, -1.f, 0.f)
	);
#endif


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

inline mat4
LookAtRH(vec3 Eye, vec3 Target, vec3 WorldUp)
{
	vec3 Forward = Normalize(Eye - Target);
	vec3 Right = Normalize(Cross(WorldUp, Forward));
	vec3 Up = Cross(Forward, Right);

	mat4 Result = mat4(
		vec4(Right, Dot(Right, Eye)),
		vec4(Up, Dot(Up, Eye)),
		vec4(Forward, Dot(Forward, Eye)),
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
CalculateModelMatrix(vec3 Position, vec3 Size, vec4 Rotation)
{
	mat4 T = Translate(Position);
	mat4 R = mat4(1.f);

	f32 Angle = Rotation.x;
	vec3 RotationAxis = Rotation.yzw;

	if (IsXAxis(RotationAxis))
	{
		R = RotateX(Angle);
	}
	else if (IsYAxis(RotationAxis))
	{
		R = RotateY(Angle);
	}
	else if (IsZAxis(RotationAxis))
	{
		R = RotateZ(Angle);
	}
	else
	{
		// todo: rotation around arbitrary axis
	}
	mat4 S = Scale(Size);

	mat4 Result = T * R * S;

	return Result;
}