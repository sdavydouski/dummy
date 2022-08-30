#pragma once

#define MaxPolyhedronVertexCount 28
#define MaxPolyhedronFaceCount 16
#define MaxPolyhedronEdgeCount (MaxPolyhedronFaceCount - 2) * 3
#define MaxPolyhedronFaceEdgeCount MaxPolyhedronFaceCount - 1

struct edge
{
    u8 VertexIndex[2];
    u8 FaceIndex[2];
};

struct face
{
    u8 EdgeCount;
    u8 EdgeIndex[MaxPolyhedronFaceEdgeCount];
};

struct polyhedron
{
    u8 VertexCount;
    u8 EdgeCount;
    u8 FaceCount;

    vec3 Vertices[MaxPolyhedronVertexCount];
    edge Edges[MaxPolyhedronEdgeCount];
    face Faces[MaxPolyhedronFaceCount];
    plane Planes[MaxPolyhedronFaceCount];
};
