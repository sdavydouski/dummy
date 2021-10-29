in VS_OUT {
    vec3 VertexPosition;
    vec3 Normal;
    vec2 TextureCoords;
    mat3 TBN;
    flat unsigned int Highlight;
} fs_in; 

out vec4 out_Color;

layout (std140, binding = 0) uniform State
{
    mat4 u_Projection;
    mat4 u_View;
    vec3 u_CameraPosition;
    float u_Time;
};

uniform phong_material u_Material;
// todo: multile directional lights
uniform directional_light u_DirectionalLight;
uniform int u_PointLightCount;
uniform point_light u_PointLights[MAX_POINT_LIGHT_COUNT];

void main()
{
    vec3 AmbientColor = u_Material.AmbientColor;

    vec3 DiffuseColor = u_Material.HasDiffuseMap
        ? texture(u_Material.DiffuseMap, fs_in.TextureCoords).rgb
        : u_Material.DiffuseColor;

    vec3 SpecularColor = u_Material.HasSpecularMap
        ? texture(u_Material.SpecularMap, fs_in.TextureCoords).rgb
        : u_Material.SpecularColor;

    float SpecularShininess = u_Material.HasShininessMap
        ? u_Material.SpecularShininess * texture(u_Material.ShininessMap, fs_in.TextureCoords).r
        : u_Material.SpecularShininess;

    vec3 Normal;
    if (u_Material.HasNormalMap)
    {
        Normal = texture(u_Material.NormalMap, fs_in.TextureCoords).rgb;
        Normal = Normal * 2.f - 1.f;
        // todo: optimize   
        Normal = fs_in.TBN * Normal;
    }
    else
    {
        // todo: should I multiple by TBN here?
        Normal = fs_in.Normal;
    }
    Normal = normalize(Normal);

    vec3 EyeDirection = normalize(u_CameraPosition - fs_in.VertexPosition);
    
#if 1
    vec3 Result = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

    for (int PointLightIndex = 0; PointLightIndex < u_PointLightCount; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, fs_in.VertexPosition);
    }
#else
    vec3 Result = DiffuseColor;
#endif

 out_Color = vec4(Result, 1.f);

#if 0
    // apply gamma correction
    float gamma = 2.2f;
    out_Color.rgb = pow(out_Color.rgb, vec3(1.f / gamma));
#endif
}