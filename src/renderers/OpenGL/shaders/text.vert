//! #include "common/version.glsl"
//! #include "common/constants.glsl"
//! #include "common/uniform.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TextureCoords;

out VS_OUT {
    vec2 TextureCoords;
} vs_out; 

uniform int u_Mode;
uniform mat4 u_Model;
uniform vec2 u_SpriteSize;
uniform sampler2D u_FontTextureAtlas;

void main()
{
    // [-1, 1] -> [0, 1]
    vec3 Position = (in_Position + 1.f) / 2.f; 

    vec2 TextureSize = textureSize(u_FontTextureAtlas, 0);
    vec2 NormalizedSpriteSize = u_SpriteSize / TextureSize;

    vs_out.TextureCoords = in_TextureCoords * NormalizedSpriteSize;

    gl_Position = GetViewProjection(u_Mode) * u_Model * vec4(Position, 1.f);
}
