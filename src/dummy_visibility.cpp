internal void
BuildFrustrumPolyhedron(game_camera *Camera, f32 FocalLength, f32 AspectRatio, f32 Near, f32 Far, polyhedron *Polyhedron)
{
    Polyhedron->VertexCount = 8;
    Polyhedron->EdgeCount = 12;
    Polyhedron->FaceCount = 6;

    mat4 WorldToCamera = LookAt(Camera->Position, Camera->Position + Camera->Direction, Camera->Up);
    mat4 CameraToWorld = Inverse(WorldToCamera);

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

internal b32
AxisAlignedBoxVisible(u32 PlaneCount, plane *Planes, aabb Box)
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
