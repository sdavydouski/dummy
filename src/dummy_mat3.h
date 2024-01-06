#pragma once

struct mat3
{
    union
    {
        vec3 Rows[3];
        f32 Elements[3][3];
    };

    mat3(const mat3 &Value)
    {
        Rows[0] = Value.Rows[0];
        Rows[1] = Value.Rows[1];
        Rows[2] = Value.Rows[2];
    }

    explicit mat3() = default;

    explicit mat3(f32 Value)
    {
        Rows[0] = vec3(Value, 0.f, 0.f);
        Rows[1] = vec3(0.f, Value, 0.f);
        Rows[2] = vec3(0.f, 0.f, Value);
    }

    explicit mat3(vec3 Row0, vec3 Row1, vec3 Row2)
    {
        Rows[0] = Row0;
        Rows[1] = Row1;
        Rows[2] = Row2;
    }

    explicit mat3(
        f32 e00, f32 e01, f32 e02,
        f32 e10, f32 e11, f32 e12,
        f32 e20, f32 e21, f32 e22
    )
    {
        Elements[0][0] = e00;
        Elements[0][1] = e01;
        Elements[0][2] = e02;

        Elements[1][0] = e10;
        Elements[1][1] = e11;
        Elements[1][2] = e12;

        Elements[2][0] = e20;
        Elements[2][1] = e21;
        Elements[2][2] = e22;
    }

    inline vec3 &operator [](u32 RowIndex)
    {
        vec3 &Result = Rows[RowIndex];
        return Result;
    }

    inline vec3 Column(u32 ColumnIndex)
    {
        vec3 Result = vec3(Elements[0][ColumnIndex], Elements[1][ColumnIndex], Elements[2][ColumnIndex]);
        return Result;
    }

    inline vec3 operator *(const vec3 &Vector)
    {
        vec3 Row0 = Rows[0];
        vec3 Row1 = Rows[1];
        vec3 Row2 = Rows[2];

        vec3 Result = vec3(Dot(Row0, Vector), Dot(Row1, Vector), Dot(Row2, Vector));

        return Result;
    }
};

inline mat3 operator +(mat3 a, mat3 b)
{
    mat3 Result = mat3(
        a.Rows[0] + b.Rows[0],
        a.Rows[1] + b.Rows[1],
        a.Rows[2] + b.Rows[2]
    );

    return Result;
}

inline mat3 operator *(mat3 a, mat3 b)
{
    vec3 Row0 = a.Rows[0];
    vec3 Row1 = a.Rows[1];
    vec3 Row2 = a.Rows[2];

    vec3 Column0 = b.Column(0);
    vec3 Column1 = b.Column(1);
    vec3 Column2 = b.Column(2);

    mat3 Result = mat3(
        Dot(Row0, Column0), Dot(Row0, Column1), Dot(Row0, Column2),
        Dot(Row1, Column0), Dot(Row1, Column1), Dot(Row1, Column2),
        Dot(Row2, Column0), Dot(Row2, Column1), Dot(Row2, Column2)
    );

    return Result;
}

inline mat3 operator *(mat3 M, f32 Scalar)
{
    mat3 Result = mat3(
        M[0] * Scalar,
        M[1] * Scalar,
        M[2] * Scalar
    );

    return Result;
}

inline mat3
Inverse(mat3 M)
{
    vec3 a = M.Column(0);
    vec3 b = M.Column(1);
    vec3 c = M.Column(2);

    vec3 r0 = Cross(b, c);
    vec3 r1 = Cross(c, a);
    vec3 r2 = Cross(a, b);

    f32 Determinant = Dot(r2, c);

    if (Abs(Determinant) < EPSILON)
    {
        Assert(!"Inverse matrix doesn't exist");
    }

    f32 InverseDeterminant = 1.f / Determinant;

    mat3 Result = mat3(
        r0 * InverseDeterminant,
        r1 * InverseDeterminant,
        r2 * InverseDeterminant
    );

    return Result;
}

inline mat3
Transpose(mat3 M)
{
    mat3 Result = mat3(
        M.Column(0),
        M.Column(1),
        M.Column(2)
    );

    return Result;
}

inline mat3
SkewSymmetric(vec3 Vector)
{
    mat3 Result = mat3(
        0.f, -Vector.z, Vector.y,
        Vector.z, 0.f, -Vector.x,
        -Vector.y, Vector.x, 0.f
    );

    return Result;
}
