//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/constants.glsl"
//! #include "common/lights.glsl"
//! #include "common/uniform.glsl"
//! #include "common/shadows.glsl"

struct phong_material
{
    float SpecularShininess;

    vec3 AmbientColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;

    sampler2D DiffuseMap;
    sampler2D SpecularMap;
    sampler2D ShininessMap;
    sampler2D NormalMap;
    
    bool HasDiffuseMap;
    bool HasSpecularMap;
    bool HasShininessMap;
    bool HasNormalMap;
};

in VS_OUT 
{
    vec3 WorldPosition;
    vec3 Normal;
    vec3 CascadeBlend;
    vec2 TextureCoords;
    mat3 TangentBasis;
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

    // Mapping the linear color from the texture to a suitable exponent ([0, 1] -> [20, 128])
    if (u_Material.HasShininessMap)
    {
        SpecularShininess *= 108.f;
        SpecularShininess += 20.f;
    }

    if (u_Material.HasNormalMap)
    {
        Normal = Normal * 2.f - 1.f; 
        Normal = fs_in.TangentBasis * Normal;
    }
    
    Normal = normalize(Normal);

    vec3 EyeDirection = normalize(u_CameraPosition - fs_in.WorldPosition);
    
    vec3 Result = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

    vec3 Ambient = AmbientColor * DiffuseColor;

    // Shadows
    float Shadow = 1.f;
    int CascadeIndex1;
    int CascadeIndex2;

    if (u_EnableShadows)
    {
        vec3 CascadeBlend = fs_in.CascadeBlend;

        vec3 LightDirection = normalize(-u_DirectionalLight.Direction);
        float CosTheta = clamp(dot(Normal, LightDirection), 0.f, 1.f);
        float Bias = clamp(0.005f * tan(acos(CosTheta)), 0.f, 0.01f);
        
        Shadow = CalculateInfiniteShadow(CascadeBlend, fs_in.WorldPosition, Bias, CascadeIndex1, CascadeIndex2);
    }

    // Visualising cascades
    vec3 CascadeColor = vec3(0.f);

    if (u_ShowCascades)
    {
        if (CascadeIndex1 == 0 && CascadeIndex2 == 1)
        {
            CascadeColor = vec3(0.5f, 0.f, 0.f);
        }
        else if (CascadeIndex2 == 1 && CascadeIndex1 == 2)
        {
            CascadeColor = vec3(0.f, 0.5f, 0.f);
        }
        else if (CascadeIndex1 == 2 && CascadeIndex2 == 3)
        {
            CascadeColor = vec3(0.f, 0.f, 0.5f);
        }
        else 
        {
            CascadeColor = vec3(0.5f, 0.5f, 0.f);
        }
    }

    Result = Ambient + Result * Shadow;

    for (int PointLightIndex = 0; PointLightIndex < u_PointLightCount; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, fs_in.WorldPosition);
    }

    Result += CascadeColor;
    Result *= fs_in.Color;

    out_Color = vec4(Result, 1.f);
}
