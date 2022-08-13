//! #include "common/version.glsl"
//! #include "common/uniform.glsl"

in VS_OUT
{
    vec2 TextureCoords;
} fs_in;

out vec4 out_Color;

uniform sampler2D u_ScreenTexture;

float zNear = 0.1f;    // TODO: Replace by the zNear of your perspective projection
float zFar  = 320.f; // TODO: Replace by the zFar  of your perspective projection

float LinearizeDepth(float Depth)
{
    float z = Depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * zNear) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{  
    vec2 TextureCoords = fs_in.TextureCoords;
    
    // static wave effect
#if 0
    float Offset = u_Time * 2 * 3.14159f * 0.75f;
	TextureCoords.x += sin(TextureCoords.y * 4 * 2 * 3.14159f + Offset) / 100;
#endif

#if 0
	vec4 TexelColor = texture(u_ScreenTexture, TextureCoords);
	out_Color = vec4(TexelColor.rgb, 1.f);
#else
    float Depth = texture(u_ScreenTexture, TextureCoords).r;
    float LinearDepth = LinearizeDepth(Depth);
    //float LinearDepth = Depth;

	out_Color = vec4(vec3(LinearDepth), 1.0);
#endif
}