#include "dummy.h"

inline aabb
CreateAABBMinMax(vec3 Min, vec3 Max)
{
    aabb Result = {};

    vec3 HalfSize = (Max - Min) / 2.f;

    Result.Center = Min + HalfSize;
    Result.HalfExtent = HalfSize;

    return Result;
}

inline aabb
CreateAABBCenterHalfExtent(vec3 Center, vec3 HalfExtent)
{
    aabb Result = {};

    Result.Center = Center;
    Result.HalfExtent = HalfExtent;

    return Result;
}

dummy_internal aabb
UpdateBounds(aabb Bounds, transform T)
{
    aabb Result = {};

    mat4 M = Transform(T);
    vec3 Translation = GetTranslation(M);

    for (u32 Axis = 0; Axis < 3; ++Axis)
    {
        Result.Center[Axis] = Translation[Axis];
        Result.HalfExtent[Axis] = 0.f;

        for (u32 Element = 0; Element < 3; ++Element)
        {
            Result.Center[Axis] += M[Axis][Element] * Bounds.Center[Element];
            Result.HalfExtent[Axis] += Abs(M[Axis][Element]) * Bounds.HalfExtent[Element];
        }
    }

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

    aabb Result = CreateAABBMinMax(vMin, vMax);

    return Result;
}

dummy_internal aabb
CalculateAxisAlignedBoundingBox(u32 MeshCount, mesh *Meshes)
{
    aabb Result = {};

    if (MeshCount > 0)
    {
        mesh *FirstMesh = First(Meshes);
        aabb Box = CalculateAxisAlignedBoundingBox(FirstMesh->VertexCount, FirstMesh->Positions);

        vec3 vMin = Box.Min();
        vec3 vMax = Box.Max();

        for (u32 MeshIndex = 1; MeshIndex < MeshCount; ++MeshIndex)
        {
            mesh *Mesh = Meshes + MeshIndex;
            aabb Box = CalculateAxisAlignedBoundingBox(Mesh->VertexCount, Mesh->Positions);

            vMin = Min(vMin, Box.Min());
            vMax = Max(vMax, Box.Max());
        }

        Result = CreateAABBMinMax(vMin, vMax);
    }

    return Result;
}

dummy_internal aabb
CalculateAxisAlignedBoundingBox(obb Box)
{
    vec3 Min = Box.Center - Box.HalfExtent.x * Box.AxisX - Box.HalfExtent.y * Box.AxisY - Box.HalfExtent.z * Box.AxisZ;
    vec3 Max = Box.Center + Box.HalfExtent.x * Box.AxisX + Box.HalfExtent.y * Box.AxisY + Box.HalfExtent.z * Box.AxisZ;

    aabb Result = CreateAABBMinMax(Min, Max);

    return Result;
}

dummy_internal f32
CalculateDiameter(u32 VertexCount, vec3 *Vertices, i32 *VertexIndexA, i32 *VertexIndexB)
{
    vec3 Directions[] =
    {
        vec3(1.f, 0.f, 0.f),  
        vec3(0.f, 1.f, 0.f),  
        vec3(0.f, 0.f, 1.f),
        vec3(1.f, 1.f, 0.f),  
        vec3(1.f, 0.f, 1.f),  
        vec3(0.f, 1.f, 1.f),
        vec3(1.f, -1.f, 0.f), 
        vec3(1.f, 0.f, -1.f), 
        vec3(0.f, 1.f, -1.f),
        vec3(1.f, 1.f, 1.f),  
        vec3(1.f, -1.f, 1.f), 
        vec3(1.f, 1.f, -1.f), 
        vec3(1.f, -1.f, -1.f)
    };

    f32 dmin[ArrayCount(Directions)];
    f32 dmax[ArrayCount(Directions)];

    i32 imin[ArrayCount(Directions)];
    i32 imax[ArrayCount(Directions)];

    // Find min and max dot products for each direction and record vertex indices
    for (u32 DirectionIndex = 0; DirectionIndex < ArrayCount(Directions); ++DirectionIndex)
    {
        vec3 Direction = Directions[DirectionIndex];

        dmin[DirectionIndex] = dmax[DirectionIndex] = Dot(Direction, Vertices[0]);
        imin[DirectionIndex] = imax[DirectionIndex] = 0;

        for (u32 VertexIndex = 1; VertexIndex < VertexCount; ++VertexIndex)
        {
            f32 d = Dot(Direction, Vertices[VertexIndex]);

            if (d < dmin[DirectionIndex])
            {
                dmin[DirectionIndex] = d;
                imin[DirectionIndex] = VertexIndex;
            }
            else if (d > dmax[DirectionIndex])
            {
                dmax[DirectionIndex] = d;
                imax[DirectionIndex] = VertexIndex;
            }
        }
    }

    // Find direction for which vertices at min and max extents are furthest apart
    f32 d2 = SquaredMagnitude(Vertices[imax[0]] - Vertices[imin[0]]);
    i32 k = 0;

    for (u32 DirectionIndex = 1; DirectionIndex < ArrayCount(Directions); ++DirectionIndex)
    {
        f32 m2 = SquaredMagnitude(Vertices[imax[DirectionIndex]] - Vertices[imin[DirectionIndex]]);

        if (m2 > d2)
        {
            d2 = m2;
            k = DirectionIndex;
        }
    }

    *VertexIndexA = imin[k];
    *VertexIndexB = imax[k];

    return d2;
}

dummy_internal void
CalculateSecondaryDiameter(u32 VertexCount, vec3 *Vertices, vec3 Axis, i32 *VertexIndexA, i32 *VertexIndexB)
{
    vec2 Directions[] =
    {
        vec2(1.f, 0.f),
        vec2(0.f, 1.f),
        vec2(1.f, 1.f),
        vec2(1.f, -1.f)
    };

    f32 dmin[ArrayCount(Directions)];
    f32 dmax[ArrayCount(Directions)];

    i32 imin[ArrayCount(Directions)];
    i32 imax[ArrayCount(Directions)];

    // Create vectors x and y perpendicular to the primary axis
    vec3 x = Perpendicular(Axis);
    vec3 y = Cross(Axis, x);

    // Find min and max dot products for each direction and record vertex indices
    for (u32 DirectionIndex = 0; DirectionIndex < ArrayCount(Directions); ++DirectionIndex)
    {
        vec3 t = x * Directions[DirectionIndex].x + y * Directions[DirectionIndex].y;

        dmin[DirectionIndex] = dmax[DirectionIndex] = Dot(t, Vertices[0]);
        imin[DirectionIndex] = imax[DirectionIndex] = 0;

        for (u32 VertexIndex = 1; VertexIndex < VertexCount; ++VertexIndex)
        {
            f32 d = Dot(t, Vertices[VertexIndex]);

            if (d < dmin[DirectionIndex])
            {
                dmin[DirectionIndex] = d;
                imin[DirectionIndex] = VertexIndex;
            }
            else if (d > dmax[DirectionIndex])
            {
                dmax[DirectionIndex] = d;
                imax[DirectionIndex] = VertexIndex;
            }
        }
    }

    // Find diameter in plane perpendicular to primary axis
    vec3 dv = Vertices[imax[0]] - Vertices[imin[0]];
    i32 k = 0;
    f32 d2 = SquaredMagnitude(dv - Axis * Dot(dv, Axis));

    for (u32 DirectionIndex = 1; DirectionIndex < ArrayCount(Directions); ++DirectionIndex)
    {
        dv = Vertices[imax[DirectionIndex]] - Vertices[imin[DirectionIndex]];
        f32 m2 = SquaredMagnitude(dv - Axis * Dot(dv, Axis));

        if (m2 > d2)
        {
            d2 = m2;
            k = DirectionIndex;
        }
    }

    *VertexIndexA = imin[k];
    *VertexIndexB = imax[k];
}

dummy_internal void
FindExtremalVertices(u32 VertexCount, vec3 *Vertices, plane Plane, i32 *VertexIndexE, i32 *VertexIndexF)
{
    *VertexIndexE = 0;
    *VertexIndexF = 0;

    f32 dmin = DotPoint(Plane, Vertices[0]);
    f32 dmax = dmin;

    for (u32 VertexIndex = 1; VertexIndex < VertexCount; ++VertexIndex)
    {
        f32 m = DotPoint(Plane, Vertices[VertexIndex]);
        
        if (m < dmin)
        {
            dmin = m;
            *VertexIndexE = VertexIndex;
        }
        else if (m > dmax)
        {
            dmax = m;
            *VertexIndexF = VertexIndex;
        }
    }
}

dummy_internal void
GetPrimaryBoxDirections(u32 VertexCount, vec3 *Vertices, i32 VertexIndexA, i32 VertexIndexB, vec3 *Directions)
{
    i32 VertexIndexC = 0;
    Directions[0] = Vertices[VertexIndexB] - Vertices[VertexIndexA];
    f32 dmax = DistPointLine(Vertices[0], Vertices[VertexIndexA], Directions[0]);

    for (u32 VertexIndex = 1; VertexIndex < VertexCount; ++VertexIndex)
    {
        f32 m = DistPointLine(Vertices[VertexIndex], Vertices[VertexIndexA], Directions[0]);

        if (m > dmax)
        {
            dmax = m;
            VertexIndexC = VertexIndex;
        }
    }

    Directions[1] = Vertices[VertexIndexC] - Vertices[VertexIndexA];
    Directions[2] = Vertices[VertexIndexC] - Vertices[VertexIndexB];

    vec3 Normal = Cross(Directions[0], Directions[1]);
    plane Plane(Normal, -Dot(Normal, Vertices[VertexIndexA]));

    i32 VertexIndexE;
    i32 VertexIndexF;
    FindExtremalVertices(VertexCount, Vertices, Plane, &VertexIndexE, &VertexIndexF);

    Directions[3] = Vertices[VertexIndexE] - Vertices[VertexIndexA];
    Directions[4] = Vertices[VertexIndexE] - Vertices[VertexIndexB];
    Directions[5] = Vertices[VertexIndexE] - Vertices[VertexIndexC];
    Directions[6] = Vertices[VertexIndexF] - Vertices[VertexIndexA];
    Directions[7] = Vertices[VertexIndexF] - Vertices[VertexIndexB];
    Directions[8] = Vertices[VertexIndexF] - Vertices[VertexIndexC];
}

dummy_internal void
GetSecondaryBoxDirections(u32 VertexCount, vec3 *Vertices, vec3 Axis, i32 VertexIndexA, i32 VertexIndexB, vec3 *Directions)
{
    Directions[0] = Vertices[VertexIndexB] - Vertices[VertexIndexA];
    
    vec3 Normal = Cross(Axis, Directions[0]);
    plane Plane(Normal, -Dot(Normal, Vertices[VertexIndexA]));

    i32 VertexIndexE;
    i32 VertexIndexF;
    FindExtremalVertices(VertexCount, Vertices, Plane, &VertexIndexE, &VertexIndexF);

    Directions[1] = Vertices[VertexIndexE] - Vertices[VertexIndexA];
    Directions[2] = Vertices[VertexIndexE] - Vertices[VertexIndexB];
    Directions[3] = Vertices[VertexIndexF] - Vertices[VertexIndexA];
    Directions[4] = Vertices[VertexIndexF] - Vertices[VertexIndexB];

    for (u32 DirectionIndex = 0; DirectionIndex < 5; ++DirectionIndex)
    {
        Directions[DirectionIndex] -= Axis * Dot(Directions[DirectionIndex], Axis);
    }
}

dummy_internal obb
CalculateOrientedBoundingBox(u32 VertexCount, vec3 *Vertices)
{
    obb Result = {};

    i32 VertexIndexA;
    i32 VertexIndexB;
    vec3 PrimaryDirections[9];
    vec3 SecondaryDirections[5];

    CalculateDiameter(VertexCount, Vertices, &VertexIndexA, &VertexIndexB);
    GetPrimaryBoxDirections(VertexCount, Vertices, VertexIndexA, VertexIndexB, PrimaryDirections);

    f32 Area = F32_MAX;

    // Loop over all candidates for primary axis
    for (u32 PrimaryDirectionIndex = 0; PrimaryDirectionIndex < ArrayCount(PrimaryDirections); ++PrimaryDirectionIndex)
    {
        if (Magnitude(PrimaryDirections[PrimaryDirectionIndex]) > 0.f)
        {
            vec3 s = Normalize(PrimaryDirections[PrimaryDirectionIndex]);
            CalculateSecondaryDiameter(VertexCount, Vertices, s, &VertexIndexA, &VertexIndexB);
            GetSecondaryBoxDirections(VertexCount, Vertices, s, VertexIndexA, VertexIndexB, SecondaryDirections);

            // Loop over all candidates for secondary axis
            for (u32 SecondaryDirectionIndex = 0; SecondaryDirectionIndex < ArrayCount(SecondaryDirections); ++SecondaryDirectionIndex)
            {
                if (Magnitude(SecondaryDirections[SecondaryDirectionIndex]) > 0.f)
                {
                    vec3 t = Normalize(SecondaryDirections[SecondaryDirectionIndex]);
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
        }
    }

    return Result;
}

inline void
CalculateVertices(obb Box, vec3 *Vertices)
{
    f32 Signs[] = { -1.f, 1.f };

    for (u32 x = 0; x < 2; ++x)
    {
        for (u32 y = 0; y < 2; ++y)
        {
            for (u32 z = 0; z < 2; ++z)
            {
                *Vertices++ = 
                    Box.Center + 
                    Signs[x] * Box.AxisX * Box.HalfExtent.x + 
                    Signs[y] * Box.AxisY * Box.HalfExtent.y + 
                    Signs[z] * Box.AxisZ * Box.HalfExtent.z;
            }
        }
    }
}
