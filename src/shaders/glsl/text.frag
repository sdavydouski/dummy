//! #include "common/version.glsl"

in GS_OUT 
{
    vec2 TextureCoords;
} fs_in; 

out vec4 out_Color;

uniform vec4 u_Color;
uniform sampler2D u_FontTextureAtlas;

void main()
{
    float Value = texture(u_FontTextureAtlas, fs_in.TextureCoords).r;

    if (Value == 0.f)
    {
        discard;
    }

    out_Color = vec4(1.f, 1.f, 1.f, Value) * u_Color;
}
