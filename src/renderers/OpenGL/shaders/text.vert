//! #include "common/version.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TextureCoords;

out VS_OUT {
    vec2 TextureCoords;
} vs_out; 

uniform mat4 u_TextProjection;
uniform mat4 u_TextView;
uniform mat4 u_Model;
uniform vec2 u_SpriteSize;

void main()
{
    vs_out.TextureCoords = in_TextureCoords * u_SpriteSize;

    vec3 Position = in_Position;

    // todo:
#if 1
    if (Position.x == -1.f)
    {
        Position.x = 0.f;
    }
    if (Position.y == -1.f)
    {
        Position.y = 0.f;
    }
#endif

    gl_Position = u_TextProjection * u_TextView * u_Model * vec4(Position, 1.f);
}
