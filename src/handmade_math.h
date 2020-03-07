#pragma once

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
			f32 Elements[4];
		};
	};

	vec2(f32 Value) : x(Value), y(Value) {}
	vec2(f32 x, f32 y) : x(x), y(y) {}

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
};

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
			f32 Elements[3];
		};
	};

	vec3(f32 Value) : x(Value), y(Value), z(Value) {}
	vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
	vec3(vec2 xy, f32 z) : x(xy.x), y(xy.y), z(z) {}

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
};


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
			f32 Elements[4];
		};
	};

	vec4(f32 Value): x(Value), y(Value), z(Value), w(Value) {}
	vec4(f32 x, f32 y, f32 z, f32 w): x(x), y(y), z(z), w(w) {}

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
};

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

	mat4(f32 Value)
	{
		Rows[0] = vec4(Value, 0.f, 0.f, 0.f);
		Rows[1] = vec4(0.f, Value, 0.f, 0.f);
		Rows[2] = vec4(0.f, 0.f, Value, 0.f);
		Rows[3] = vec4(0.f, 0.f, 0.f, Value);
	}

	mat4(const mat4 &Value)
	{
		Rows[0] = Value.Rows[0];
		Rows[1] = Value.Rows[1];
		Rows[2] = Value.Rows[2];
		Rows[3] = Value.Rows[3];
	}

	inline vec4 &operator [](u32 RowIndex)
	{
		vec4 &Result = Rows[RowIndex];
		return Result;
	}
};

inline mat4
Scale(mat4 M, f32 Value)
{
	mat4 Result = mat4(M);

	Result[0][0] *= Value;
	Result[1][1] *= Value;
	Result[2][2] *= Value;

	return Result;
}

inline mat4
Translate(mat4 M, vec3 Value)
{
	mat4 Result = mat4(M);

	Result[0][3] += Value.x;
	Result[1][3] += Value.y;
	Result[2][3] += Value.z;

	return Result;
}
