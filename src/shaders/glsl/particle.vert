//! #include "common/version.glsl"

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_Size;
layout (location = 2) in vec4 in_Color;

out VS_OUT 
{
    vec2 Size;
    vec4 Color;
} vs_out; 

void main()
{
    vs_out.Size = in_Size;
    vs_out.Color = in_Color;
    gl_Position = vec4(in_Position, 1.f);
}
