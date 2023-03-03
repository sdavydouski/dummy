#include "dummy.h"

dummy_internal void
BuildFrustrumPolyhedron(game_camera *Camera, polyhedron *Polyhedron)
{
    Polyhedron->VertexCount = 8;
    Polyhedron->EdgeCount = 12;
    Polyhedron->FaceCount = 6;

    mat4 WorldToCamera = GetCameraTransform(Camera);
    mat4 CameraToWorld = Inverse(WorldToCamera);

    f32 FocalLength = Camera->FocalLength;
    f32 AspectRatio = Camera->AspectRatio;
    f32 Near = Camera->NearClipPlane;
    f32 Far = Camera->FarClipPlane;

    // Generate vertices for the near side
    f32 y = Near / FocalLength;
    f32 x = y * AspectRatio;
    Polyhedron->Vertices[0] = vec4(CameraToWorld * vec4(x, y, -Near, 1.f)).xyz;
    Polyhedron->Vertices[1] = vec4(CameraToWorld * vec4(x, -y, -Near, 1.f)).xyz;
    Polyhedron->Vertices[2] = vec4(CameraToWorld * vec4(-x, -y, -Near, 1.f)).xyz;
    Polyhedron->Vertices[3] = vec4(CameraToWorld * vec4(-x, y, -Near, 1.f)).xyz;

    // Generate vertices for the far plane
    y = Far / FocalLength;
    x = y * AspectRatio;
    Polyhedron->Vertices[4] = vec4(CameraToWorld * vec4(x, y, -Far, 1.f)).xyz;
    Polyhedron->Vertices[5] = vec4(CameraToWorld * vec4(x, -y, -Far, 1.f)).xyz;
    Polyhedron->Vertices[6] = vec4(CameraToWorld * vec4(-x, -y, -Far, 1.f)).xyz;
    Polyhedron->Vertices[7] = vec4(CameraToWorld * vec4(-x, y, -Far, 1.f)).xyz;

    // Generate lateral planes
    f32 mx = 1.f / Sqrt(Square(FocalLength) + Square(AspectRatio));
    f32 my = 1.f / Sqrt(Square(FocalLength) + 1.f);
    Polyhedron->Planes[0] = Transform(plane(-mx * FocalLength, 0.f, -mx * AspectRatio, 0.f), WorldToCamera);
    Polyhedron->Planes[1] = Transform(plane(0.f, my * FocalLength, -my, 0.f), WorldToCamera);
    Polyhedron->Planes[2] = Transform(plane(mx * FocalLength, 0.f, -mx * AspectRatio, 0.f), WorldToCamera);
    Polyhedron->Planes[3] = Transform(plane(0.f, -my * FocalLength, -my, 0.f), WorldToCamera);

    // Generate near and far planes
    Polyhedron->Planes[4] = Transform(plane(0.f, 0.f, -1.f, -Near), WorldToCamera);
    Polyhedron->Planes[5] = Transform(plane(0.f, 0.f, 1.f, Far), WorldToCamera);

    // Generate all edges and lateral faces
    edge *Edge = Polyhedron->Edges;
    face *Face = Polyhedron->Faces;
    for (u32 Index = 0; Index < 4; ++Index, ++Edge, ++Face)
    {
        Edge[0].VertexIndex[0] = u8(Index);
        Edge[0].VertexIndex[1] = u8(Index + 4);
        Edge[0].FaceIndex[0] = u8(Index);
        Edge[0].FaceIndex[1] = u8((Index - 1) & 3);

        Edge[4].VertexIndex[0] = u8(Index);
        Edge[4].VertexIndex[1] = u8((Index + 1) & 3);
        Edge[4].FaceIndex[0] = 4;
        Edge[4].FaceIndex[1] = u8(Index);

        Edge[8].VertexIndex[0] = u8(((Index + 1) & 3) + 4);
        Edge[8].VertexIndex[1] = u8(Index + 4);
        Edge[8].FaceIndex[0] = 5;
        Edge[8].FaceIndex[1] = u8(Index);

        Face->EdgeCount = 4;
        Face->EdgeIndex[0] = u8(Index);
        Face->EdgeIndex[1] = u8((Index + 1) & 3);
        Face->EdgeIndex[2] = u8(Index + 4);
        Face->EdgeIndex[3] = u8(Index + 8);
    }

    // Generate near and far faces
    Face[0].EdgeCount = 4;
    Face[0].EdgeIndex[0] = 4;
    Face[0].EdgeIndex[1] = 5;
    Face[0].EdgeIndex[2] = 6;
    Face[0].EdgeIndex[3] = 7;

    Face[1].EdgeCount = 4;
    Face[1].EdgeIndex[0] = 8;
    Face[1].EdgeIndex[1] = 9;
    Face[1].EdgeIndex[2] = 10;
    Face[1].EdgeIndex[3] = 11;
}

dummy_internal bool32
ClipPolyhedron(polyhedron *Polyhedron, plane Plane, polyhedron *Result)
{
    f32 VertexLocations[MaxPolyhedronVertexCount] = {};
    i8 VertexCodes[MaxPolyhedronVertexCount] = {};
    i8 EdgeCodes[MaxPolyhedronEdgeCount] = {};
    u8 VertexRemap[MaxPolyhedronVertexCount] = {};
    u8 EdgeRemap[MaxPolyhedronEdgeCount] = {};
    u8 FaceRemap[MaxPolyhedronFaceCount] = {};
    u8 PlaneEdgeTable[MaxPolyhedronFaceEdgeCount] = {};

    f32 PolyhedronEpsilon = 0.001f;
    i32 MinCode = 6;
    i32 MaxCode = 0;

    // Classify vertices
    for (u32 VertexIndex = 0; VertexIndex < Polyhedron->VertexCount; ++VertexIndex)
    {
        VertexRemap[VertexIndex] = 0xFF;
        f32 d = DotPoint(Plane, Polyhedron->Vertices[VertexIndex]);
        VertexLocations[VertexIndex] = d;

        i32 Code = (d > -PolyhedronEpsilon) + (d > PolyhedronEpsilon) * 2;
        MinCode = Min(MinCode, Code);
        MaxCode = Max(MaxCode, Code);
        VertexCodes[VertexIndex] = Code;
    }

    if (MinCode != 0)
    {
        // No vertices on negative side of clip plane
        *Result = *Polyhedron;
        return true;
    }

    if (MaxCode <= 1)
    {
        // No vertices on positive side of clip plane
        return false;
    }

    // Classify edges
    for (u32 EdgeIndex = 0; EdgeIndex < Polyhedron->EdgeCount; ++EdgeIndex)
    {
        EdgeRemap[EdgeIndex] = 0xFF;
        edge *Edge = Polyhedron->Edges + EdgeIndex;
        EdgeCodes[EdgeIndex] = (i8) (VertexCodes[Edge->VertexIndex[0]] + VertexCodes[Edge->VertexIndex[1]]);
    }

    // Determine which faces will be in result
    u32 ResultFaceCount = 0;
    for (u32 FaceIndex = 0; FaceIndex < Polyhedron->FaceCount; ++FaceIndex)
    {
        FaceRemap[FaceIndex] = 0xFF;
        face *Face = Polyhedron->Faces + FaceIndex;
        
        for (u32 FaceEdgeIndex = 0; FaceEdgeIndex < Face->EdgeCount; ++FaceEdgeIndex)
        {
            if (EdgeCodes[Face->EdgeIndex[FaceEdgeIndex]] >= 3)
            {
                // Face has a vertex on the positive side of the plane
                Result->Planes[ResultFaceCount] = Polyhedron->Planes[FaceIndex];
                FaceRemap[FaceIndex] = (u8)(ResultFaceCount++);
                break;
            }
        }
    }

    u32 ResultVertexCount = 0;
    u32 ResultEdgeCount = 0;
    for (u32 EdgeIndex = 0; EdgeIndex < Polyhedron->EdgeCount; ++EdgeIndex)
    {
        if (EdgeCodes[EdgeIndex] >= 2)
        {
            // The edge is not completely clipped away
            edge *Edge= Polyhedron->Edges + EdgeIndex;
            edge *ResultEdge = Result->Edges + ResultEdgeCount;
            EdgeRemap[EdgeIndex] = (u8)(ResultEdgeCount++);

            ResultEdge->FaceIndex[0] = FaceRemap[Edge->FaceIndex[0]];
            ResultEdge->FaceIndex[1] = FaceRemap[Edge->FaceIndex[1]];

            // Loop over both vertices of edge
            for (u32 EdgeVertexIndex = 0; EdgeVertexIndex < 2; ++EdgeVertexIndex)
            {
                u8 VertexIndex = Edge->VertexIndex[EdgeVertexIndex];

                if (VertexCodes[VertexIndex] != 0)
                {
                    // This vertex on positive side of plane or in plane
                    u8 RemappedVertexIndex = VertexRemap[VertexIndex];
                    if (RemappedVertexIndex == 0xFF)
                    {
                        RemappedVertexIndex = ResultVertexCount++;
                        VertexRemap[VertexIndex] = RemappedVertexIndex;
                        Result->Vertices[RemappedVertexIndex] = Polyhedron->Vertices[VertexIndex];
                    }

                    ResultEdge->VertexIndex[EdgeVertexIndex] = RemappedVertexIndex;
                }
                else
                {
                    // This vertex on negative side, and other vertex on positive side
                    u8 OtherVertexIndex = Edge->VertexIndex[1 - EdgeVertexIndex];

                    vec3 p1 = Polyhedron->Vertices[VertexIndex];
                    vec3 p2 = Polyhedron->Vertices[OtherVertexIndex];

                    f32 d1 = VertexLocations[VertexIndex];
                    f32 d2 = VertexLocations[OtherVertexIndex];
                    f32 t = d2 / (d2 - d1);

                    Result->Vertices[ResultVertexCount] = p2 * (1.f - t) + p1 * t;
                    ResultEdge->VertexIndex[EdgeVertexIndex] = (u8)(ResultVertexCount++);
                }
            }
        }
    }

    u32 PlaneEdgeCount = 0;
    for (u32 FaceIndex = 0; FaceIndex < Polyhedron->FaceCount; ++FaceIndex)
    {
        u8 RemappedFaceIndex = FaceRemap[FaceIndex];

        if (RemappedFaceIndex != 0xFF) 
        {
            // The face is not completely clipped away
            edge *NewEdge = 0;
            u8 NewEdgeIndex = 0xFF;

            face *Face = Polyhedron->Faces + FaceIndex;
            face *ResultFace = Result->Faces + RemappedFaceIndex;
            u32 ResultFaceEdgeCount = 0;
            
            // Loop over face's original edges
            for (u32 FaceEdgeIndex = 0; FaceEdgeIndex < Face->EdgeCount; ++FaceEdgeIndex)
            {
                u8 EdgeIndex = Face->EdgeIndex[FaceEdgeIndex];
                i32 Code = EdgeCodes[EdgeIndex];

                if (Code & 1)
                {
                    // One endpoint on negative side of plane, and other either
                    // on positive side (code == 3) or in plane (code == 1)
                    if (!NewEdge)
                    {
                        // At this point, we know we need a new edge
                        NewEdgeIndex = ResultEdgeCount;
                        NewEdge = Result->Edges + ResultEdgeCount;
                        PlaneEdgeTable[PlaneEdgeCount++] = (u8)(ResultEdgeCount++);

                        NewEdge->VertexIndex[0] = 0xFF;
                        NewEdge->VertexIndex[1] = 0xFF;
                        NewEdge->FaceIndex[0] = RemappedFaceIndex;
                        NewEdge->FaceIndex[1] = 0xFF;
                    }

                    edge *Edge = Polyhedron->Edges + EdgeIndex;
                    bool32 ccw = Edge->FaceIndex[0] == FaceIndex;
                    bool32 InsertEdge = ccw ^ (VertexCodes[Edge->VertexIndex[0]] == 0);

                    if (Code == 3)
                    {
                        // Original edge has been clipped
                        u8 RemappedEdgeIndex = EdgeRemap[EdgeIndex];
                        ResultFace->EdgeIndex[ResultFaceEdgeCount++] = RemappedEdgeIndex;

                        edge *ResultEdge = Result->Edges + RemappedEdgeIndex;
                        if (InsertEdge)
                        {
                            NewEdge->VertexIndex[0] = ResultEdge->VertexIndex[ccw];
                            ResultFace->EdgeIndex[ResultFaceEdgeCount++] = NewEdgeIndex;
                        }
                        else
                        {
                            NewEdge->VertexIndex[1] = ResultEdge->VertexIndex[!ccw];
                        }
                    }
                    else
                    {
                        // Original edge has been deleted, code == 1
                        if (InsertEdge)
                        {
                            NewEdge->VertexIndex[0] = VertexRemap[Edge->VertexIndex[!ccw]];
                            ResultFace->EdgeIndex[ResultFaceEdgeCount++] = NewEdgeIndex;
                        }
                        else
                        {
                            NewEdge->VertexIndex[1] = VertexRemap[Edge->VertexIndex[ccw]];
                        }
                    }
                }
                else if (Code != 0)
                {
                    // Neither endpoint is on the negative side of the clipping plane
                    u8 RemappedEdgeIndex = EdgeRemap[EdgeIndex];
                    ResultFace->EdgeIndex[ResultFaceEdgeCount++] = RemappedEdgeIndex;

                    if (Code == 2)
                    {
                        PlaneEdgeTable[PlaneEdgeCount++] = RemappedEdgeIndex;
                    }
                }
            }

            if (NewEdge && Max(NewEdge->VertexIndex[0], NewEdge->VertexIndex[1]) == 0xFF)
            {
                // The input polyhedron was invalid
                *Result = *Polyhedron;
                return true;
            }

            ResultFace->EdgeCount = (u8)ResultEdgeCount;
        }
    }

    if (PlaneEdgeCount > 2)
    {
        Result->Planes[ResultFaceCount] = Plane;
        face *ResultFace = Result->Faces + ResultFaceCount;
        ResultFace->EdgeCount = (u8)PlaneEdgeCount;

        for (u32 PlaneEdgeIndex = 0; PlaneEdgeIndex < PlaneEdgeCount; ++PlaneEdgeIndex)
        {
            u8 EdgeIndex = PlaneEdgeTable[PlaneEdgeIndex];
            ResultFace->EdgeIndex[PlaneEdgeIndex] = EdgeIndex;

            edge *ResultEdge = Result->Edges + EdgeIndex;
            u8 k = ResultEdge->FaceIndex[1] == 0xFF;
            ResultEdge->FaceIndex[k] = (u8)ResultFaceCount;
        }

        ResultFaceCount++;
    }

    Result->VertexCount = (u8)ResultVertexCount;
    Result->EdgeCount = (u8)ResultEdgeCount;
    Result->FaceCount = (u8)ResultFaceCount;

    return true;
}

dummy_internal u32
CalculateShadowRegion(polyhedron *Polyhedron, vec4 LightPosition, plane *ShadowPlanes)
{
    u32 ShadowPlaneCount = 0;

    f32 ShadowRegionEpsilon = 0.000001f;
    bool32 FrontArray[MaxPolyhedronFaceCount] = {};

    // Classify faces of polyhedron and record back planes
    for (u32 PlaneIndex = 0; PlaneIndex < Polyhedron->FaceCount; ++PlaneIndex)
    {
        plane Plane = Polyhedron->Planes[PlaneIndex];
        face Face = Polyhedron->Faces[PlaneIndex];
        FrontArray[PlaneIndex] = Dot(Plane, LightPosition) > 0.f;

        if (FrontArray[PlaneIndex])
        {
            ShadowPlanes[ShadowPlaneCount++] = Plane;
        }
    }

    // Construct planes containing silhouette edges and light position
    for (u32 EdgeIndex = 0; EdgeIndex < Polyhedron->EdgeCount; ++EdgeIndex)
    {
        edge *Edge = Polyhedron->Edges + EdgeIndex;

        bool32 Front = FrontArray[Edge->FaceIndex[0]];

        if (Front ^ FrontArray[Edge->FaceIndex[1]])
        {
            // This edge is on the silhouette
            vec3 v0 = Polyhedron->Vertices[Edge->VertexIndex[0]];
            vec3 v1 = Polyhedron->Vertices[Edge->VertexIndex[1]];

            vec3 n = Cross(LightPosition.xyz - v0 * LightPosition.w, v1 - v0);

            // Make sure plane is not degenerate
            f32 m = SquaredMagnitude(n);
            if (m > ShadowRegionEpsilon)
            {
                // Normalize and point inward
                n *= (Front ? 1.f : -1.f) / Sqrt(m);
                ShadowPlanes[ShadowPlaneCount++] = plane(n, -Dot(n, v0));

                if (ShadowPlaneCount == MaxPolyhedronFaceCount)
                {
                    break;
                }
            }
        }
    }

    return ShadowPlaneCount;
}

dummy_internal bool32
AxisAlignedBoxVisible(u32 PlaneCount, plane *Planes, bounds Box)
{
    vec3 BoxCenter = GetAABBCenter(Box);
    vec3 BoxHalfSize = GetAABBHalfSize(Box);

    for (u32 PlaneIndex = 0; PlaneIndex < PlaneCount; ++PlaneIndex)
    {
        plane Plane = Planes[PlaneIndex];

        f32 EffectiveRadius = Abs(Plane.Normal.x * BoxHalfSize.x) + Abs(Plane.Normal.y * BoxHalfSize.y) + Abs(Plane.Normal.z * BoxHalfSize.z);

        if (DotPoint(Plane, BoxCenter) <= -EffectiveRadius)
        {
            return false;
        }
    }

    return true;
}
