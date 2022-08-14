//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/uniform.glsl"
//! #include "common/blinn_phong.glsl"
//! #include "common/shadows.glsl"

in VS_OUT {
    vec3 WorldPosition;
    vec3 Normal;
    vec3 CascadeBlend;
    vec2 TextureCoords;
    mat3 TBN;
    flat unsigned int Highlight;
} fs_in; 

out vec4 out_Color;

uniform mat4 u_LightSpaceMatrix;

uniform phong_material u_Material;
// todo: multile directional lights
uniform directional_light u_DirectionalLight;
uniform int u_PointLightCount;
uniform point_light u_PointLights[MAX_POINT_LIGHT_COUNT];

void main()
{
    // todo: AmbientColor is always black
    vec3 AmbientColor = u_Material.AmbientColor;// + vec3(0.2f);

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

    vec3 EyeDirection = normalize(u_CameraPosition - fs_in.WorldPosition);
    
    vec3 Result = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

    for (int PointLightIndex = 0; PointLightIndex < u_PointLightCount; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, fs_in.WorldPosition);
    }

    // Shadow
    vec3 CascadeBlend = fs_in.CascadeBlend;

#if 1
    vec3 LightDirection = normalize(-u_DirectionalLight.Direction);
    float CosTheta = clamp(dot(Normal, LightDirection), 0.f, 1.f);
    float Bias = clamp(0.005 * tan(acos(CosTheta)), 0.f, 0.01f);
#endif

    vec3 ShadowResult = CalculateInfiniteShadow(CascadeBlend, fs_in.WorldPosition, Bias);

    float Shadow = ShadowResult.x;
    int CascadeIndex1 = int(ShadowResult.y);
    int CascadeIndex2 = int(ShadowResult.z);
    //

    Result *= Shadow;

    if (u_ShowCascades)
    {
        // Visualising cascades
        if (CascadeIndex1 == 0 && CascadeIndex2 == 1)
        {
            Result += vec3(0.5f, 0.f, 0.f);
        }
        else if (CascadeIndex2 == 1 && CascadeIndex1 == 2)
        {
            Result += vec3(0.f, 0.5f, 0.f);
        }
        else if (CascadeIndex1 == 2 && CascadeIndex2 == 3)
        {
            Result += vec3(0.f, 0.f, 0.5f);
        }
        else 
        {
            Result += vec3(0.5f, 0.5f, 0.f);
        }
    }
    
    out_Color = vec4(Result, 1.f);
}
