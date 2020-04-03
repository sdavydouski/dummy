#pragma once

#include <cmath>

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

inline f32
Clamp(f32 Value, f32 Min, f32 Max)
{
	if (Value < Min) return Min;
	if (Value > Max) return Max;

	return Value;
}

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

inline f32
Dot(vec3 a, vec3 b)
{
	f32 Result = a.x * b.x + a.y * b.y + a.z * b.z;
	return Result;
}

inline f32
Magnitude(vec3 Vector)
{
	f32 Result = Sqrt(Square(Vector.x) + Square(Vector.y));
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
			f32 Elements[4];
		};
	};

	explicit vec4() = default;
	explicit vec4(f32 Value) : x(Value), y(Value), z(Value), w(Value) {}
	explicit vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
	explicit vec4(f32 Value, vec3 Vector) : x(Value), y(Vector.x), z(Vector.y), w(Vector.z) {}

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
	f32 Result = Sqrt(Square(Vector.x) + Square(Vector.y));
	return Result;
}

inline vec4
Normalize(vec4 Vector)
{
	vec4 Result = Vector / Magnitude(Vector);
	return Result;
}

struct mat4
{
	union
	{
		struct
		{
			vec4 Rows[4];
		};
		struct
		{
			f32 Elements[4][4];
		};
	};

	explicit mat4(f32 Value)
	{
		Rows[0] = vec4(Value, 0.f, 0.f, 0.f);
		Rows[1] = vec4(0.f, Value, 0.f, 0.f);
		Rows[2] = vec4(0.f, 0.f, Value, 0.f);
		Rows[3] = vec4(0.f, 0.f, 0.f, Value);
	}

	// todo: explicit copy contructor?
	mat4(const mat4 &Value)
	{
		Rows[0] = Value.Rows[0];
		Rows[1] = Value.Rows[1];
		Rows[2] = Value.Rows[2];
		Rows[3] = Value.Rows[3];
	}

	explicit mat4(vec4 Row0, vec4 Row1, vec4 Row2, vec4 Row3)
	{
		Rows[0] = Row0;
		Rows[1] = Row1;
		Rows[2] = Row2;
		Rows[3] = Row3;
	}

	inline vec4 &operator [](u32 RowIndex)
	{
		vec4 &Result = Rows[RowIndex];
		return Result;
	}

	inline vec4 Row(u32 RowIndex)
	{
		vec4 Result = Rows[RowIndex];
		return Result;
	}

	inline vec4 Column(u32 ColumnIndex)
	{
		vec4 Result = vec4(Elements[0][ColumnIndex], Elements[1][ColumnIndex], Elements[2][ColumnIndex], Elements[3][ColumnIndex]);
		return Result;
	}

	inline mat4 operator *(mat4 &M)
	{
		vec4 Row0 = Rows[0];
		vec4 Row1 = Rows[1];
		vec4 Row2 = Rows[2];
		vec4 Row3 = Rows[3];

		vec4 Column0 = M.Column(0);
		vec4 Column1 = M.Column(1);
		vec4 Column2 = M.Column(2);
		vec4 Column3 = M.Column(3);

		mat4 Result = mat4(
			vec4(Dot(Row0, Column0), Dot(Row0, Column1), Dot(Row0, Column2), Dot(Row0, Column3)),
			vec4(Dot(Row1, Column0), Dot(Row1, Column1), Dot(Row1, Column2), Dot(Row1, Column3)),
			vec4(Dot(Row2, Column0), Dot(Row2, Column1), Dot(Row2, Column2), Dot(Row2, Column3)),
			vec4(Dot(Row3, Column0), Dot(Row3, Column1), Dot(Row3, Column2), Dot(Row3, Column3))
		);

		return Result;
	}
};

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

// projects view frustrum into the canonical view volume,
// where points inside view frustrum have normalized device
// coordinates in the range [-1, 1] along x, y and z axis
// after the perspective divide occurs
inline mat4
Perspective(f32 FovY, f32 Aspect, f32 Near, f32 Far)
{
	mat4 Result = mat4(
		vec4(1.f / (Aspect * Tan(FovY * 0.5f)), 0.f, 0.f, 0.f),
		vec4(0.f, 1.f / Tan(FovY / 2), 0.f, 0.f),
		vec4(0.f, 0.f, (-Near - Far) / (Near - Far), 2.f * (Near * Far) / (Near - Far)),
		vec4(0.f, 0.f, 1.f, 0.f)
	);

	return Result;
}
