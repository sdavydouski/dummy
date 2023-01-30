//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/constants.glsl"
//! #include "common/phong.glsl"
//! #include "common/uniform.glsl"
//! #include "common/shadows.glsl"

in VS_OUT {
    vec3 WorldPosition;
    vec3 Normal;
    vec3 CascadeBlend;
    vec2 TextureCoords;
    mat3 TBN;
    vec3 Color;
} fs_in; 

out vec4 out_Color;

uniform mat4 u_LightSpaceMatrix;

uniform phong_material u_Material;

vec3 GetMaterial(bool HasMap, sampler2D Map, vec3 Fallback)
{
    vec3 Result = HasMap 
        ? texture(Map, fs_in.TextureCoords).rgb 
        : Fallback;

    return Result;
}

float GetMaterial(bool HasMap, sampler2D Map, float Fallback)
{
    float Result = HasMap 
        ? texture(Map, fs_in.TextureCoords).r
        : Fallback;

    return Result;
}

void main()
{

    vec3 AmbientColor = u_Material.AmbientColor;
    vec3 DiffuseColor = GetMaterial(u_Material.HasDiffuseMap, u_Material.DiffuseMap, u_Material.DiffuseColor);
    vec3 SpecularColor = GetMaterial(u_Material.HasSpecularMap, u_Material.SpecularMap, u_Material.SpecularColor);
    float SpecularShininess = GetMaterial(u_Material.HasShininessMap, u_Material.ShininessMap, u_Material.SpecularShininess);
    vec3 Normal = GetMaterial(u_Material.HasNormalMap, u_Material.NormalMap, fs_in.Normal);

    if (u_Material.HasNormalMap)
    {
        Normal = Normal * 2.f - 1.f; 
        Normal = fs_in.TBN * Normal;
    }
    
    Normal = normalize(Normal);

#if 1
    vec3 EyeDirection = normalize(u_CameraPosition - fs_in.WorldPosition);
    
    vec3 Result = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

    // Shadow
    vec3 CascadeBlend = fs_in.CascadeBlend;

    vec3 LightDirection = normalize(-u_DirectionalLight.Direction);
    float CosTheta = clamp(dot(Normal, LightDirection), 0.f, 1.f);
    float Bias = clamp(0.005 * tan(acos(CosTheta)), 0.f, 0.01f);

    vec3 ShadowResult = CalculateInfiniteShadow(CascadeBlend, fs_in.WorldPosition, Bias);

    float Shadow = ShadowResult.x;
    int CascadeIndex1 = int(ShadowResult.y);
    int CascadeIndex2 = int(ShadowResult.z);

    vec3 Ambient = AmbientColor * DiffuseColor;
    Result = Ambient + Result * Shadow;

    Result *= fs_in.Color;

    for (int PointLightIndex = 0; PointLightIndex < u_PointLightCount; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, fs_in.WorldPosition);
    }

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
#else
    out_Color = vec4(DiffuseColor, 1.f);
#endif
}
