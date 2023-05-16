//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/constants.glsl"
//! #include "common/lights.glsl"
//! #include "common/uniform.glsl"
//! #include "common/shadows.glsl"

// Constant normal incidence Fresnel factor for all dielectrics
const vec3 Fdielectric = vec3(0.04);

struct standard_material
{
    vec3 AlbedoColor;
    float Metalness;
    float Roughness;
    float AmbientOcclusion;

    sampler2D AlbedoMap;
    sampler2D NormalMap;
    sampler2D MetalnessMap;
    sampler2D RoughnessMap;

    bool HasAlbedoMap;
    bool HasNormalMap;
    bool HasMetalnessMap;
    bool HasRoughnessMap;
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

layout(binding=1) uniform samplerCube u_IrradianceTexture;
layout(binding=2) uniform samplerCube u_SpecularTexture;
layout(binding=3) uniform sampler2D u_SpecularBRDF;

uniform mat4 u_LightSpaceMatrix;
uniform standard_material u_Material;

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
    // Sample input textures to get shading model params
    vec3 Albedo = GetMaterial(u_Material.HasAlbedoMap, u_Material.AlbedoMap, u_Material.AlbedoColor);
    //float Metalness = GetMaterial(u_Material.HasMetalnessMap, u_Material.MetalnessMap, u_Material.Metalness);
    //float Roughness = GetMaterial(u_Material.HasRoughnessMap, u_Material.RoughnessMap, u_Material.Roughness);

    // todo
    // {
    float Metalness = u_Material.HasMetalnessMap 
        ? texture(u_Material.MetalnessMap, fs_in.TextureCoords).b
        : u_Material.Metalness;
    
    float Roughness = u_Material.HasRoughnessMap 
        ? texture(u_Material.RoughnessMap, fs_in.TextureCoords).g
        : u_Material.Roughness;
    // }

    float AmbientOcclusion = u_Material.AmbientOcclusion;

    vec3 Normal = GetMaterial(u_Material.HasNormalMap, u_Material.NormalMap, fs_in.Normal);

    // Get current fragment's normal and transform to world space
    if (u_Material.HasNormalMap)
    {
        Normal = Normal * 2.f - 1.f; 
        Normal = fs_in.TangentBasis * Normal;
    }
    
    Normal = normalize(Normal);

    // Outgoing light direction (vector from world-space fragment position to the "eye")
    vec3 Lo = normalize(u_CameraPosition - fs_in.WorldPosition);

    // Angle between surface normal and outgoing light direction
    float cosLo = max(0.f, dot(Normal, Lo));

    // Specular reflection vector.
    vec3 Lr = 2.f * cosLo * Normal - Lo;

    // Fresnel reflectance at normal incidence (for metals use albedo color)
    vec3 F0 = mix(Fdielectric, Albedo, Metalness);

    // Direct lighting calculation for analytical lights
    vec3 DirectLighting = vec3(0.f);

    // Directional Light
    {
        vec3 Li = -u_DirectionalLight.Direction;
		vec3 Lradiance = u_DirectionalLight.Color;

        // Half-vector between Li and Lo
        vec3 Lh = normalize(Li + Lo);

        // Calculate angles between surface normal and various light vectors
        float cosLi = max(0.0, dot(Normal, Li));
        float cosLh = max(0.0, dot(Normal, Lh));

        // Calculate Fresnel term for direct lighting
        vec3 F  = FresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
        // Calculate normal distribution for specular BRDF
        float D = DistributionGGX(cosLh, Roughness);
        // Calculate geometric attenuation for specular BRDF
        float G = SchlickGGX(cosLi, cosLo, Roughness);

        // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
        // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
        // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
        vec3 kd = mix(vec3(1.0) - F, vec3(0.0), Metalness);

        // Lambert diffuse BRDF.
        // We don't scale by 1/PI for lighting & material units to be more convenient
        vec3 DiffuseBRDF = kd * Albedo;

        // Cook-Torrance specular microfacet BRDF
        vec3 SpecularBRDF = (F * D * G) / max(EPSILON, 4.0 * cosLi * cosLo);

        // Total contribution for this light.
        DirectLighting += (DiffuseBRDF + SpecularBRDF) * Lradiance * cosLi;
    }

    // Ambient lighting (IBL)
    vec3 AmbientLighting = vec3(0.f);

    {
        // Sample diffuse irradiance at normal direction.
        vec3 irradiance = texture(u_IrradianceTexture, Normal).rgb;

        // Calculate Fresnel term for ambient lighting.
        // Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
        // use cosLo instead of angle with light's half-vector (cosLh above).
        vec3 F = FresnelSchlick(F0, cosLo);

        // Get diffuse contribution factor (as with direct lighting).
        vec3 kd = mix(vec3(1.0) - F, vec3(0.0), Metalness);

        // Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either
        vec3 diffuseIBL = kd * Albedo * irradiance;

        // Sample pre-filtered specular reflection environment at correct mipmap level
        int specularTextureLevels = textureQueryLevels(u_SpecularTexture);
        vec3 specularIrradiance = textureLod(u_SpecularTexture, Lr, Roughness * specularTextureLevels).rgb;

        // Split-sum approximation factors for Cook-Torrance specular BRDF
        vec2 specularBRDF = texture(u_SpecularBRDF, vec2(cosLo, Roughness)).rg;

        // Total specular IBL contribution
        vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

        // Total ambient lighting contribution.
        AmbientLighting = diffuseIBL + specularIBL;
    }

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

    vec3 Result = AmbientLighting + DirectLighting * Shadow;

    // Point Lights
    for(uint LightIndex = 0; LightIndex < u_PointLightCount; ++LightIndex)
    {
        point_light Light = u_PointLights[LightIndex];

        float Attenuation = CalculateInverseSquareAttenuation(Light.Position, fs_in.WorldPosition, Light.Attenuation);

        vec3 Li = normalize(Light.Position - fs_in.WorldPosition);
		vec3 Lradiance = Light.Color * Attenuation;

        // todo: duplicate
        // {
        // Half-vector between Li and Lo
        vec3 Lh = normalize(Li + Lo);

        // Calculate angles between surface normal and various light vectors
        float cosLi = max(0.0, dot(Normal, Li));
        float cosLh = max(0.0, dot(Normal, Lh));

        // Calculate Fresnel term for direct lighting
        vec3 F  = FresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
        // Calculate normal distribution for specular BRDF
        float D = DistributionGGX(cosLh, Roughness);
        // Calculate geometric attenuation for specular BRDF
        float G = SchlickGGX(cosLi, cosLo, Roughness);

        // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
        // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
        // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
        vec3 kd = mix(vec3(1.0) - F, vec3(0.0), Metalness);

        // Lambert diffuse BRDF.
        // We don't scale by 1/PI for lighting & material units to be more convenient
        vec3 DiffuseBRDF = kd * Albedo;

        // Cook-Torrance specular microfacet BRDF
        vec3 SpecularBRDF = (F * D * G) / max(EPSILON, 4.0 * cosLi * cosLo);

        // Total contribution for this light.
        Result += (DiffuseBRDF + SpecularBRDF) * Lradiance * cosLi;
        // }
    }

    Result += CascadeColor;
    Result *= fs_in.Color;

    // Final fragment color
    out_Color = vec4(Result, 1.f);
}
