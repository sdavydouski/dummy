#pragma once

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

    mat4(const mat4 &Value)
    {
        Rows[0] = Value.Rows[0];
        Rows[1] = Value.Rows[1];
        Rows[2] = Value.Rows[2];
        Rows[3] = Value.Rows[3];
    }

    explicit mat4() = default;

    explicit mat4(f32 Value)
    {
        Rows[0] = vec4(Value, 0.f, 0.f, 0.f);
        Rows[1] = vec4(0.f, Value, 0.f, 0.f);
        Rows[2] = vec4(0.f, 0.f, Value, 0.f);
        Rows[3] = vec4(0.f, 0.f, 0.f, Value);
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
Inverse(mat4 M)
{
    vec3 a = M.Column(0).xyz;
    vec3 b = M.Column(1).xyz;
    vec3 c = M.Column(2).xyz;
    vec3 d = M.Column(3).xyz;

    f32 x = M[3][0];
    f32 y = M[3][1];
    f32 z = M[3][2];
    f32 w = M[3][3];

    vec3 s = Cross(a, b);
    vec3 t = Cross(c, d);
    vec3 u = a * y - b * x;
    vec3 v = c * w - d * z;

    f32 Determinant = Dot(s, v) + Dot(t, u);

    if (Determinant == 0.f)
    {
        Assert(!"Inverse matrix doesn't exist");
    }

    f32 InverseDeterminant = 1.f / Determinant;

    s *= InverseDeterminant;
    t *= InverseDeterminant;
    u *= InverseDeterminant;
    v *= InverseDeterminant;

    vec3 r0 = Cross(b, v) + t * y;
    vec3 r1 = Cross(v, a) - t * x;
    vec3 r2 = Cross(d, u) + s * w;
    vec3 r3 = Cross(u, c) - s * z;

    mat4 Result = mat4(
        vec4(r0, -Dot(b, t)),
        vec4(r1, Dot(a, t)),
        vec4(r2, -Dot(d, s)),
        vec4(r3, Dot(c, s))
    );

    return Result;
}

inline mat4
Transpose(mat4 M)
{
    mat4 Result = mat4(
        M.Column(0),
        M.Column(1),
        M.Column(2),
        M.Column(3)
    );

    return Result;
}

// todo: implement
inline void
Decompose(mat4 M, vec3 *Scale, quat *Rotation, vec3 *Translation)
{

}
