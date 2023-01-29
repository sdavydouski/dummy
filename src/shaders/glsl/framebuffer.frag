//? #include "common/version.glsl"
//? #include "common/constants.glsl"
//! #include "common/phong.glsl"
//? #include "common/uniform.glsl"

in VS_OUT
{
    vec2 TextureCoords;
} fs_in;

out vec4 out_Color;

uniform sampler2D u_ScreenTexture;
//uniform float u_zNear;
//uniform float u_zFar;

/*
float LinearizeDepth(float Depth)
{
    float zNear = u_zNear;
    float zFar = u_zFar;

    float z = Depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * zNear) / (zFar + zNear - z * (zFar - zNear));
}
*/

void main()
{  
    vec2 TextureCoords = fs_in.TextureCoords;
    
    // static wave effect
#if 0
    float Offset = u_Time * 2 * 3.14159f * 0.75f;
	TextureCoords.x += sin(TextureCoords.y * 4 * 2 * 3.14159f + Offset) / 100;
#endif

	vec4 TexelColor = texture(u_ScreenTexture, TextureCoords);
	out_Color = TexelColor;
    //out_Color = vec4(1.f, 0.f, 0.f, 1.f);
}