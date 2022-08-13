//! #include "common/version.glsl"
//! #include "common/blinn_phong.glsl"
//! #include "common/uniform.glsl"

in VS_OUT {
    vec3 VertexPosition;
    vec3 Normal;
    vec3 CascadeCoord0;
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

    vec3 EyeDirection = normalize(u_CameraPosition - fs_in.VertexPosition);
    
#if 1
    vec3 Result = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

    for (int PointLightIndex = 0; PointLightIndex < u_PointLightCount; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, fs_in.VertexPosition);
    }

    // Shadow
    vec3 CascadeCoord0 = fs_in.CascadeCoord0;
    vec3 CascadeBlend = fs_in.CascadeBlend;

    vec4 ShadowResult = CalculateInfiniteShadow(CascadeCoord0, CascadeBlend);

    float Shadow = ShadowResult.x;
    int CascadeIndex1 = int(ShadowResult.y);
    int CascadeIndex2 = int(ShadowResult.z);
    float Weight = ShadowResult.w;

    // todo: pcf?
    vec4 WorldPosition = vec4(fs_in.VertexPosition, 1.f);
    vec4 FragLightPosition1 = u_CascadeViewProjection[CascadeIndex1] * WorldPosition;
    vec4 FragLightPosition2 = u_CascadeViewProjection[CascadeIndex2] * WorldPosition;

    vec4 FragPosLightSpace = mix(FragLightPosition2, FragLightPosition1, Weight);

    vec3 ProjCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // transform to [0,1] range
    ProjCoords = ProjCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float ClosestDepth = Shadow;
    // get depth of current fragment from light's perspective
    float CurrentDepth = ProjCoords.z;

#if 0
    vec3 LightDirection = normalize(-u_DirectionalLight.Direction);
    // calculate bias (based on depth map resolution and slope)
    float Bias = max(0.05 * (1.0 - dot(Normal, LightDirection)), 0.005f);
#else
    float Bias = 0.005f;
#endif

    Shadow = CurrentDepth - Bias > ClosestDepth ? 0.5f : 1.f;
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
    

#else
    vec3 Result = DiffuseColor;
#endif

    out_Color = vec4(Result, 1.f);
}