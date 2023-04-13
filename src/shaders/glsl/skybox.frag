//! #include "common/version.glsl"

out vec4 out_Color;

in VS_OUT 
{
    vec3 LocalPosition;
} fs_in; 

uniform samplerCube u_Skybox;

void main()
{
    vec3 EnvVector = normalize(fs_in.LocalPosition);

    vec3 EnvColor = texture(u_Skybox, EnvVector).rgb;
    //EnvColor = EnvColor / (EnvColor + vec3(1.f));
    //EnvColor = pow(EnvColor, vec3(1.f / 2.2f)); 

    out_Color = vec4(EnvColor, 1.f);
}
