//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/uniform.glsl" 

layout(location = 0) in vec3 in_Position;

out VS_OUT
{
    vec3 NearPlanePosition;
    vec3 FarPlanePosition;
} vs_out;

void main()
{
    vs_out.NearPlanePosition = UnprojectPoint(vec3(in_Position.xy, -1.f), u_View, u_Projection);
    vs_out.FarPlanePosition = UnprojectPoint(vec3(in_Position.xy, 1.f), u_View, u_Projection);

    gl_Position = vec4(in_Position, 1.f);
}