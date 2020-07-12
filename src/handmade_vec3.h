#pragma once

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

	inline b32 operator <(vec3 Vector)
	{
		b32 Result = x < Vector.x && y < Vector.y && z < Vector.z;
		return Result;
	}

	inline b32 operator >(vec3 Vector)
	{
		b32 Result = x > Vector.x &&y > Vector.y &&z > Vector.z;
		return Result;
	}

	inline b32 operator ==(vec3 Vector)
	{
		b32 Result = x == Vector.x && y == Vector.y && z == Vector.z;
		return Result;
	}
};

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

inline b32 operator <=(vec3 a, vec3 b)
{
	b32 Result = a < b || a == b;
	return Result;
}

inline b32 operator >=(vec3 a, vec3 b)
{
	b32 Result = a > b || a == b;
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

inline vec3
Normalize(vec3 Vector)
{
	vec3 Result = Vector / Magnitude(Vector);
	return Result;
}

inline b32
IsXAxis(vec3 Vector)
{
	b32 Result = Vector.x == 1.f && Vector.y == 0.f && Vector.z == 0.f;
	return Result;
}

inline b32
IsYAxis(vec3 Vector)
{
	b32 Result = Vector.x == 0.f && Vector.y == 1.f && Vector.z == 0.f;
	return Result;
}

inline b32
IsZAxis(vec3 Vector)
{
	b32 Result = Vector.x == 0.f && Vector.y == 0.f && Vector.z == 1.f;
	return Result;
}
