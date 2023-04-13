//! #include "common/version.glsl"

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_Size;
layout (location = 2) in vec2 in_SpriteSize;
layout (location = 3) in vec2 in_SpriteOffset;

out VS_OUT 
{
    vec2 Size;
    vec2 SpriteSize;
    vec2 SpriteOffset;
} vs_out; 

void main()
{
    vs_out.Size = in_Size;
    vs_out.SpriteSize = in_SpriteSize;
    vs_out.SpriteOffset = in_SpriteOffset;

    gl_Position = vec4(in_Position, 1.f);
}
