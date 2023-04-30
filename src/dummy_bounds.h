#pragma once

// Axis-aligned bounding box
struct aabb
{
    vec3 Min;
    vec3 Max;
};

// Oriented bounding box
struct obb
{
    vec3 Center;
    vec3 HalfExtent;

    union
    {
        struct
        {
            vec3 AxisX;
            vec3 AxisY;
            vec3 AxisZ;
        };

        vec3 Axis[3];
    };
};

inline aabb
CreateAABBMinMax(vec3 Min, vec3 Max)
{
    aabb Result = {};

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

inline aabb
CreateAABBCenterHalfSize(vec3 Center, vec3 HalfSize)
{
    aabb Result = {};

    Result.Min = Center - HalfSize;
    Result.Max = Center + HalfSize;

    return Result;
}

inline vec3
GetAABBHalfSize(aabb Box)
{
    vec3 Result = (Box.Max - Box.Min) * 0.5f;
    return Result;
}

inline vec3
GetAABBCenter(aabb Box)
{
    vec3 Result = Box.Min + GetAABBHalfSize(Box);
    return Result;
}

inline aabb
Union(aabb a, aabb b)
{
    aabb Result;

    Result.Min = Min(a.Min, b.Min);
    Result.Max = Max(a.Max, b.Max);

    return Result;
}

dummy_internal aabb
CalculateAxisAlignedBoundingBox(u32 VertexCount, vec3 *Vertices)
{
    vec3 vMin = Vertices[0];
    vec3 vMax = Vertices[0];

    for (u32 VertexIndex = 1; VertexIndex < VertexCount; ++VertexIndex)
    {
        vec3 *Vertex = Vertices + VertexIndex;

        vMin = Min(vMin, *Vertex);
        vMax = Max(vMax, *Vertex);
    }

    aabb Result = {};

    Result.Min = vMin;
    Result.Max = vMax;

    return Result;
}

dummy_internal aabb
ScaleBounds(aabb Bounds, vec3 Scale)
{
    aabb Result = {};

    vec3 ScaledHalfSize = (Bounds.Max - Bounds.Min) * Scale / 2.f;
    // todo:
    vec3 Center = vec3(0.f, ScaledHalfSize.y, 0.f);

    Result.Min = Center - ScaledHalfSize;
    Result.Max = Center + ScaledHalfSize;

    return Result;
}

dummy_internal aabb
TranslateBounds(aabb Bounds, vec3 Translation)
{
    aabb Result = {};

    Result.Min = Bounds.Min + Translation;
    Result.Max = Bounds.Max + Translation;

    return Result;
}

dummy_internal aabb
UpdateBounds(aabb Bounds, transform T)
{
    aabb Result = {};

    vec3 Translation = T.Translation;
    mat4 M = Transform(T);

    for (u32 Axis = 0; Axis < 3; ++Axis)
    {
        Result.Min[Axis] = Result.Max[Axis] = Translation[Axis];

        for (u32 Element = 0; Element < 3; ++Element)
        {
            f32 e = M[Axis][Element] * Bounds.Min[Element];
            f32 f = M[Axis][Element] * Bounds.Max[Element];

            if (e < f)
            {
                Result.Min[Axis] += e;
                Result.Max[Axis] += f;
            }
            else
            {
                Result.Min[Axis] += f;
                Result.Max[Axis] += e;
            }
        }
    }

    return Result;
}

#if 0
dummy_internal obb
CalculateOrientedBoundingBox(u32 VertexCount, vec3 *Vertices)
{
    obb Result = {};

    i32 a;
    i32 b;
    vec3 PrimaryDirection[9];
    vec3 SecondaryDirection[5];

    CalculatePrimaryDiameter(VertexCount, Vertices, &a, &b);
    GetPrimaryBoxDirections(VertexCount, Vertices, a, b, PrimaryDirection);

    f32 Area = F32_MAX;

    // Loop over all candidates for primary axis
    for (u32 k = 0; k < 9; ++k)
    {
        vec3 s = Normalize(PrimaryDirection[k]);
        CalculateSecondaryDiameter(VertexCount, Vertices, s, &a, &b);
        GetSecondaryBoxDirections(VertexCount, Vertices, s, a, b, SecondaryDirection);

        // Loop over all candidates for secondary axis
        for (u32 j = 0; j < 5; ++j)
        {
            vec3 t = Normalize(SecondaryDirection[j]);
            vec3 u = Cross(s, t);

            f32 smin = Dot(s, Vertices[0]);
            f32 smax = smin;

            f32 tmin = Dot(t, Vertices[0]);
            f32 tmax = tmin;

            f32 umin = Dot(u, Vertices[0]);
            f32 umax = umin;

            for (u32 VertexIndex = 1; VertexIndex < VertexCount; ++VertexIndex)
            {
                f32 ds = Dot(s, Vertices[VertexIndex]);
                f32 dt = Dot(t, Vertices[VertexIndex]);
                f32 du = Dot(u, Vertices[VertexIndex]);

                smin = Min(smin, ds);
                smax = Max(smax, ds);

                tmin = Min(tmin, dt);
                tmax = Max(tmax, dt);

                umin = Min(umin, du);
                umax = Max(umax, du);
            }

            f32 hx = (smax - smin) * 0.5f;
            f32 hy = (tmax - tmin) * 0.5f;
            f32 hz = (umax - umin) * 0.5f;

            // Calculate one-eighth surface area and see if it's better
            f32 m = hx * hy + hy * hz + hz * hx;

            if (m < Area)
            {
                Area = m;

                Result.Center = (s * (smin + smax) + t * (tmin + tmax) + u * (umin + umax)) * 0.5f;
                Result.HalfExtent = vec3(hx, hy, hz);
                Result.AxisX = s;
                Result.AxisY = t;
                Result.AxisZ = u;
            }
        }
    }

    return Result;
}
#endif
