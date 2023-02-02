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

const float Gamma     = 2.2;
const float Exposure  = 1.0;
const float PureWhite = 1.0;

void main()
{  
    vec2 TextureCoords = fs_in.TextureCoords;
    
    // static wave effect
#if 0
    float Offset = u_Time * 2 * 3.14159f * 0.75f;
	TextureCoords.x += sin(TextureCoords.y * 4 * 2 * 3.14159f + Offset) / 100;
#endif

	vec3 TexelColor = texture(u_ScreenTexture, TextureCoords).rgb;

    // Reinhard tonemapping operator
	// see: "Photographic Tone Reproduction for Digital Images", eq. 4
	float Luminance = dot(TexelColor, vec3(0.2126f, 0.7152f, 0.0722f));
	float MappedLuminance = (Luminance * (1.f + Luminance/ (PureWhite * PureWhite))) / (1.f + Luminance);

	// Scale color by ratio of average luminances
	vec3 MappedColor = (MappedLuminance / Luminance) * TexelColor;

	// Gamma correction
	out_Color = vec4(pow(MappedColor, vec3(1.f / Gamma)), 1.f);
}
