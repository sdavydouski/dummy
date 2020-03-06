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
	};

	vec2(f32 x, f32 y) : x(x), y(y) {}
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
	};

	vec4(f32 x, f32 y, f32 z, f32 w): x(x), y(y), z(z), w(w) {}
};
