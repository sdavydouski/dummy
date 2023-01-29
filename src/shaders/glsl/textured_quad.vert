//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/phong.glsl"
//! #include "common/uniform.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TextureCoords;

out VS_OUT 
{
    vec2 TextureCoords;
} vs_out; 

uniform int u_Mode;
uniform mat4 u_Model;

void main()
{
    vs_out.TextureCoords = in_TextureCoords;
    gl_Position = GetViewProjection(u_Mode) * u_Model * vec4(in_Position, 1.f);
}
