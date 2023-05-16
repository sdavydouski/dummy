//? #include "version.glsl"
//? #include "math.glsl"

struct directional_light
{
    vec3 Direction;
    vec3 Color;
};

struct light_attenuation
{
    float Constant;
    float Linear;
    float Quadratic;
};

struct point_light
{
    vec3 Position;
    vec3 Color;

    light_attenuation Attenuation;
};

float CalculateInverseSquareAttenuation(vec3 LightPosition, vec3 VertexPosition, light_attenuation LightAttenuation)
{
    float LightDistance = length(LightPosition - VertexPosition);

    return 1.f / (
        LightAttenuation.Constant + 
        LightAttenuation.Linear * LightDistance + 
        LightAttenuation.Quadratic * Square(LightDistance)
    );
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
    
    //return ((AmbientColor + DirectColor) * DiffuseColor);
    return (DirectColor * DiffuseColor);
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
    vec3 LightDirection = normalize(Light.Position - VertexPosition);
    vec3 HalfwayDirection = normalize(LightDirection + EyeDirection);
    
    vec3 Diffuse = CalculateDiffuseReflection(Normal, LightDirection, Light.Color, AmbientColor, DiffuseColor);
    vec3 Specular = CalculateSpecularReflection(Normal, HalfwayDirection, LightDirection, Light.Color, SpecularColor, SpecularShininess);    

    float LightAttenuation = CalculateInverseSquareAttenuation(Light.Position, VertexPosition, Light.Attenuation);

    return (Diffuse + Specular) * LightAttenuation;
}
