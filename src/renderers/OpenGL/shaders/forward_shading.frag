#version 450

#define MAX_NUMBER_OF_POINT_LIGHTS 2

in vec3 ex_VertexPosition;
in vec3 ex_Normal;
in vec2 ex_TextureCoords;
in mat3 ex_TBN;
flat in unsigned int ex_Highlight;

out vec4 out_Color;

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

struct light_attenuation
{
    float Constant;
    float Linear;
    float Quadratic;
};

struct directional_light
{
    vec3 Direction;
    vec3 Color;
};

struct point_light
{
    vec3 Position;
    vec3 Color;

    light_attenuation Attenuation;
};

struct spot_light
{
    vec3 Position;
    vec3 Color;
    vec3 Direction;

    float InnerCutOffAngle;
    float OuterCutOffAngle;

    light_attenuation Attenuation;
};

float Square(float Value)
{
    return Value * Value;
}

float Saturate(float Value)
{
    return clamp(Value, 0.f, 1.f);
}

vec3 CalculateDiffuseReflection(
    vec3 Normal, 
    vec3 LightDirection, 
    vec3 LightColor, 
    vec3 AmbientColor, 
    vec3 DiffuseColor
)
{
    vec3 DirectColor = LightColor * Saturate(dot(Normal, LightDirection));
    return ((AmbientColor + DirectColor) * DiffuseColor);
}

vec3 CalculateSpecularReflection(
    vec3 Normal,
    vec3 HalfwayDirection,
    vec3 LightDirection,
    vec3 LightColor,
    vec3 SpecularColor,
    float SpecularShininess
)
{
    float Highlight = pow(Saturate(dot(Normal, HalfwayDirection)), SpecularShininess);// * float(dot(Normal, LightDirection) > 0.f);
    return (LightColor * SpecularColor * Highlight);
}

float CalculateInverseSquareAttenuation(vec3 LightPosition, vec3 VertexPosition, light_attenuation LightAttenuation)
{
    float LightDistance = length(LightPosition - VertexPosition);

    return 1.f / (
        LightAttenuation.Constant + 
        LightAttenuation.Linear * LightDistance + 
        LightAttenuation.Quadratic * Square(LightDistance)
    );
}

vec3 CalculateDirectionalLight(
    directional_light Light,
    vec3 AmbientColor,
    vec3 DiffuseColor,
    vec3 SpecularColor,
    float SpecularShininess,
    vec3 Normal, 
    vec3 EyeDirection
)
{
    vec3 LightDirection = normalize(-Light.Direction);  
    vec3 HalfwayDirection = normalize(LightDirection + EyeDirection);
    
    vec3 Diffuse = CalculateDiffuseReflection(Normal, LightDirection, Light.Color, AmbientColor, DiffuseColor);
    vec3 Specular = CalculateSpecularReflection(Normal, HalfwayDirection, LightDirection, Light.Color, SpecularColor, SpecularShininess);

    return (Diffuse + Specular);
}

vec3 CalculatePointLight(
    point_light Light,
    vec3 AmbientColor,
    vec3 DiffuseColor,
    vec3 SpecularColor,
    float SpecularShininess,
    vec3 Normal, 
    vec3 EyeDirection,
    vec3 VertexPosition
)
{
    vec3 LightDirection = normalize(Light.Position - ex_VertexPosition);
    vec3 HalfwayDirection = normalize(LightDirection + EyeDirection);
    
    vec3 Diffuse = CalculateDiffuseReflection(Normal, LightDirection, Light.Color, AmbientColor, DiffuseColor);
    vec3 Specular = CalculateSpecularReflection(Normal, HalfwayDirection, LightDirection, Light.Color, SpecularColor, SpecularShininess);    

    float LightAttenuation = CalculateInverseSquareAttenuation(Light.Position, VertexPosition, Light.Attenuation);

    return (Diffuse + Specular) * LightAttenuation;
}

uniform phong_material u_Material;
uniform directional_light u_DirectionalLight;
uniform point_light u_PointLights[MAX_NUMBER_OF_POINT_LIGHTS];
uniform vec3 u_CameraPosition;
uniform unsigned int u_Highlight;
uniform float u_Time;

#define MeshFlag_Highlight 0x1u

void main()
{
    vec3 AmbientColor = u_Material.AmbientColor;

    vec3 DiffuseColor = u_Material.HasDiffuseMap
        ? texture(u_Material.DiffuseMap, ex_TextureCoords).rgb
        : u_Material.DiffuseColor;

    vec3 SpecularColor = u_Material.HasSpecularMap
        ? texture(u_Material.SpecularMap, ex_TextureCoords).rgb
        : u_Material.SpecularColor;

    float SpecularShininess = u_Material.HasShininessMap
        ? u_Material.SpecularShininess * texture(u_Material.ShininessMap, ex_TextureCoords).r
        : u_Material.SpecularShininess;

    vec3 Normal;
    if (u_Material.HasNormalMap)
    {
        Normal = texture(u_Material.NormalMap, ex_TextureCoords).rgb;
        Normal = Normal * 2.f - 1.f;
        // todo: optimize   
        Normal = ex_TBN * Normal;
    }
    else
    {
        Normal = ex_Normal;
    }
    Normal = normalize(Normal);

    vec3 EyeDirection = normalize(u_CameraPosition - ex_VertexPosition);
    
    // todo: very expensive
#if 1
    vec3 Result = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

    for (int PointLightIndex = 0; PointLightIndex < MAX_NUMBER_OF_POINT_LIGHTS; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, ex_VertexPosition);
    }
#else
    vec3 Result = DiffuseColor;
#endif

    out_Color = vec4(Result, 1.f);

#if 1
    if (bool(ex_Highlight & MeshFlag_Highlight) || bool(u_Highlight & MeshFlag_Highlight))
    {
        float Value = 0.1f * abs(sin(u_Time * 2.f)) + 0.1f;
        out_Color += vec4(Value, Value, 0.f, 0.f);
    }
#endif
}