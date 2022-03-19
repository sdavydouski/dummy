//! #include "common/version.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TextureCoords;

out VS_OUT
{
    vec2 TextureCoords;
} vs_out;

void main()
{
    vs_out.TextureCoords = in_TextureCoords;
    gl_Position = vec4(in_Position, 1.f);
}