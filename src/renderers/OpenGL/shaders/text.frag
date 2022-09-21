//! #include "common/version.glsl"

in VS_OUT {
    vec2 TextureCoords;
} fs_in; 

out vec4 out_Color;

uniform vec4 u_Color;
uniform vec2 u_TextureOffset;
uniform sampler2D u_FontTextureAtlas;

void main()
{
    float Value = texture(u_FontTextureAtlas, fs_in.TextureCoords + u_TextureOffset).r;

    out_Color = vec4(1.f, 1.f, 1.f, Value) * u_Color;
    gl_FragDepth = 1.f - Value;
}
