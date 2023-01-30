//! #include "common/version.glsl"

in GS_OUT {
    vec4 Color;
    vec2 TextureCoords;
} fs_in; 

out vec4 out_Color;

uniform sampler2D u_Texture;

void main()
{
    vec4 Texel = texture(u_Texture, fs_in.TextureCoords);

    if (Texel.a < 0.1f)
    {
        discard;
    }

    out_Color = fs_in.Color;
    //out_Color = Texel * fs_in.Color;
}
