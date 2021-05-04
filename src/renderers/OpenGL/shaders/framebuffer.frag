in VS_OUT
{
    vec2 TextureCoords;
} fs_in;

out vec4 out_Color;

uniform sampler2D u_ScreenTexture;
uniform float u_Time;

void main()
{  
    vec2 TextureCoords = fs_in.TextureCoords;
    
    // static wave effect
#if 0
    float Offset = u_Time * 2 * 3.14159f * 0.75f;
	TextureCoords.x += sin(TextureCoords.y * 4 * 2 * 3.14159f + Offset) / 100;
#endif

	vec4 TexelColor = texture(u_ScreenTexture, TextureCoords);
	out_Color = vec4(TexelColor.rgb, 1.f);
}