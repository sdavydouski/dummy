//! #include "math.glsl"

#define MAX_POINT_LIGHT_COUNT %d
//! #undef MAX_POINT_LIGHT_COUNT
//! #define MAX_POINT_LIGHT_COUNT 32

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
    vec3 LightDirection = normalize(Light.Position - VertexPosition);
    vec3 HalfwayDirection = normalize(LightDirection + EyeDirection);
    
    vec3 Diffuse = CalculateDiffuseReflection(Normal, LightDirection, Light.Color, AmbientColor, DiffuseColor);
    vec3 Specular = CalculateSpecularReflection(Normal, HalfwayDirection, LightDirection, Light.Color, SpecularColor, SpecularShininess);    

    float LightAttenuation = CalculateInverseSquareAttenuation(Light.Position, VertexPosition, Light.Attenuation);

    return (Diffuse + Specular) * LightAttenuation;
}

float CalculateInfiniteShadow(vec4 FragPosLightSpace, vec3 LightDirection, vec3 Normal, sampler2D ShadowMap)
{
	// perform perspective divide
    vec3 ProjCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // transform to [0,1] range
    ProjCoords = ProjCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float ClosestDepth = texture(ShadowMap, ProjCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float CurrentDepth = ProjCoords.z;

#if 1
    // calculate bias (based on depth map resolution and slope)
    float Bias = max(0.05 * (1.0 - dot(Normal, LightDirection)), 0.005f);
#else
    float Bias = 0.005f;
#endif

#if 1
    // PCF
    float Shadow = 0.0;
    vec2 TexelSize = 1.0 / textureSize(ShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMap, ProjCoords.xy + vec2(x, y) * TexelSize).r; 
            Shadow += CurrentDepth - Bias > pcfDepth  ? 0.5 : 0.0;        
        }    
    }
    Shadow /= 9.0;
#else
    float Shadow = CurrentDepth - Bias > ClosestDepth ? 0.5f : 0.f; 
#endif
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(ProjCoords.z > 1.f)
    {   
        Shadow = 0.f;
    }

    return Shadow;
}