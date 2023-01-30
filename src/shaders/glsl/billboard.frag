//! #include "common/version.glsl"

in GS_OUT 
{
    vec2 TextureCoords;
} fs_in; 

out vec4 out_Color;

uniform bool u_HasTexture;
uniform sampler2D u_Texture;
uniform vec4 u_Color;

void main()
{
    vec4 Value = u_Color;

    if (u_HasTexture)
    {
        Value = texture(u_Texture, fs_in.TextureCoords);

        if (Value.a < 0.1f)
        {
            discard;
        }
    }

    out_Color = Value;
}
