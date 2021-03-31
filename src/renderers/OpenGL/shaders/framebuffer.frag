#version 450

in vec2 ex_TextureCoords;

out vec4 out_Color;

uniform sampler2D u_ScreenTexture;
uniform float u_Time;

void main()
{
#if 1
    //vec4 TexelColor = texture(u_ScreenTexture, ex_TextureCoords + 0.0005*vec2( sin(u_Time+2560.0*ex_TextureCoords.x),cos(u_Time+1440.0*ex_TextureCoords.y)));
    vec4 TexelColor = texture(u_ScreenTexture, ex_TextureCoords);
    out_Color = vec4(TexelColor.rgb, 1.f);
#else
    out_Color = vec4(1.f, 1.f, 0.f, 1.f);
#endif
}