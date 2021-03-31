#version 450

layout(location = 0) in vec3 in_Position;

out vec3 ex_VertexPosition;
out vec3 ex_NearPlanePosition;
out vec3 ex_FarPlanePosition;

uniform mat4 u_Projection;
uniform mat4 u_View;

vec3 UnprojectPoint(vec3 p, mat4 View, mat4 Projection)
{
    mat4 ViewInv = inverse(View);
    mat4 ProjectionInv = inverse(Projection);
    vec4 UnprojectedPoint = ViewInv * ProjectionInv * vec4(p, 1.f);
    vec3 Result = UnprojectedPoint.xyz / UnprojectedPoint.w;
    
    return Result;
}

void main()
{
    ex_VertexPosition = in_Position;
    ex_NearPlanePosition = UnprojectPoint(vec3(in_Position.xy, -1.f), u_View, u_Projection);
    ex_FarPlanePosition = UnprojectPoint(vec3(in_Position.xy, 1.f), u_View, u_Projection);

    gl_Position = vec4(in_Position, 1.f);
}