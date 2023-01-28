//! #include "common/version.glsl"

in VS_OUT 
{
    vec2 TextureCoords;
} fs_in; 

out vec4 out_Color;

uniform sampler2D u_Texture;

void main()
{
    vec4 Value = texture(u_Texture, fs_in.TextureCoords);

    if (Value.a == 0.f)
    {
        discard;
    }

    out_Color = Value;
    //out_Color = vec4(0.f, 1.f, 0.f, 1.f);
}
