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
    vs_out.NearPlanePosition = UnprojectPoint(vec3(in_Position.xy, 0.f), u_ViewProjection);
    vs_out.FarPlanePosition = UnprojectPoint(vec3(in_Position.xy, 1.f), u_ViewProjection);

    gl_Position = vec4(in_Position, 1.f);
}