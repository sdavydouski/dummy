#pragma once

char *SimpleVertexShader = (char *)
R"SHADER(
#version 450

layout(location = 0) in vec3 in_Position;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{ 
    gl_Position = u_Projection * u_View * u_Model * vec4(in_Position, 1.f);
}
)SHADER";

char *SimpleFragmentShader = (char *)
R"SHADER(
#version 450

out vec4 out_Color;

uniform vec4 u_Color;

void main()
{
    out_Color = u_Color;
}
)SHADER";

char *GridVertexShader = (char *)
R"SHADER(
#version 450

layout(location = 0) in vec3 in_Position;

out vec3 ex_VertexPosition; 

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    ex_VertexPosition = (u_Model * vec4(in_Position, 1.f)).xyz;
    gl_Position = u_Projection * u_View * u_Model * vec4(in_Position, 1.f);
}
)SHADER";

char *GridFragmentShader = (char *)
R"SHADER(
#version 450

in vec3 ex_VertexPosition;

out vec4 out_Color;

uniform vec3 u_CameraPosition;
uniform vec3 u_Color;

void main()
{
    float DistanceFromCamera = length(u_CameraPosition - ex_VertexPosition);
    float Opacity = clamp(DistanceFromCamera * 0.01f, 0.f, 1.f);

    out_Color = vec4(u_Color, 1.f - Opacity);
}
)SHADER";

char *ForwardShadingVertexShader = (char *)
R"SHADER(
#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec3 in_Bitangent;
layout(location = 4) in vec2 in_TextureCoords;

out vec3 ex_VertexPosition;
out vec3 ex_Normal;
out vec2 ex_TextureCoords;
out mat3 ex_TBN;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    ex_VertexPosition = (u_Model * vec4(in_Position, 1.f)).xyz;
    ex_Normal = mat3(transpose(inverse(u_Model))) * in_Normal;
    ex_TextureCoords = in_TextureCoords;

    vec3 T = normalize(vec3(u_Model * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(u_Model * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(u_Model * vec4(in_Normal, 0.f)));

    ex_TBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * u_Model * vec4(in_Position, 1.f);
}
)SHADER";

char *ForwardShadingFragmentShader = (char *)
R"SHADER(
#version 450

#define MAX_NUMBER_OF_POINT_LIGHTS 2

in vec3 ex_VertexPosition;
in vec3 ex_Normal;
in vec2 ex_TextureCoords;
in mat3 ex_TBN;

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
    vec3 ReflectedLightDirection = reflect(-LightDirection, Normal);
    
    // todo: dump ambient component (separate colors for lights?)
    vec3 Ambient = Light.Color * AmbientColor * 0.1f;

    float DiffuseImpact = max(dot(LightDirection, Normal), 0.f);
    vec3 Diffuse = Light.Color * DiffuseColor * DiffuseImpact;

    float SpecularImpact = pow(
        max(dot(ReflectedLightDirection, EyeDirection), 0.f),
        SpecularShininess
    );
    vec3 Specular =  Light.Color * SpecularColor * SpecularImpact;

    vec3 Result = (Ambient + Diffuse + Specular);

    return Result;
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
    vec3 ReflectedLightDirection = reflect(-LightDirection, Normal);
    
    // todo: dump ambient component (separate colors for lights?)
    vec3 Ambient = Light.Color * AmbientColor * 0.1f;

    float DiffuseImpact = max(dot(LightDirection, Normal), 0.f);
    vec3 Diffuse = Light.Color * DiffuseColor * DiffuseImpact;

    float SpecularImpact = pow(
        max(dot(ReflectedLightDirection, EyeDirection), 0.f),
        SpecularShininess
    );
    vec3 Specular =  Light.Color * SpecularColor * SpecularImpact;

    float LightDistance = length(Light.Position - VertexPosition);
    float LightAttenuation = 1.f / (
        Light.Attenuation.Constant + 
        Light.Attenuation.Linear * LightDistance + 
        Light.Attenuation.Quadratic * Square(LightDistance)
    );

    vec3 Result = (Ambient + Diffuse + Specular) * LightAttenuation;

    return Result;
}

uniform phong_material u_Material;
uniform directional_light u_DirectionalLight;
uniform point_light u_PointLights[MAX_NUMBER_OF_POINT_LIGHTS];
uniform vec3 u_CameraPosition;

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
    
    vec3 LightImpact = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

#if 1
    for (int PointLightIndex = 0; PointLightIndex < MAX_NUMBER_OF_POINT_LIGHTS; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        LightImpact += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, ex_VertexPosition);
    }
#endif
    
    out_Color = vec4(LightImpact, 1.f);
}
)SHADER";

char *SkinnedMeshVertexShader = (char *)
R"SHADER(
#version 450

#define MAX_WEIGHT_COUNT 4

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec3 in_Bitangent;
layout(location = 4) in vec2 in_TextureCoords;
layout(location = 5) in ivec4 in_JointIndices;
layout(location = 6) in vec4 in_Weights;

// todo:
#if 0
out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;  
#endif

out vec3 ex_VertexPosition;
out vec3 ex_Normal;
out vec2 ex_TextureCoords;
out mat3 ex_TBN;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform samplerBuffer u_SkinningMatricesSampler;

void main()
{
    mat4 Model = mat4(0.f);

    for (int Index = 0; Index <  MAX_WEIGHT_COUNT; ++Index)
    {
        int SkinningMatricesSamplerOffset = in_JointIndices[Index] * 4;

        vec4 Row0 = texelFetch(u_SkinningMatricesSampler, SkinningMatricesSamplerOffset + 0);
        vec4 Row1 = texelFetch(u_SkinningMatricesSampler, SkinningMatricesSamplerOffset + 1);
        vec4 Row2 = texelFetch(u_SkinningMatricesSampler, SkinningMatricesSamplerOffset + 2);
        vec4 Row3 = texelFetch(u_SkinningMatricesSampler, SkinningMatricesSamplerOffset + 3);

        mat4 SkinningMatrix = transpose(mat4(Row0, Row1, Row2, Row3));
                
        Model += SkinningMatrix * in_Weights[Index];
    }

    vec4 WorldPosition = Model * vec4(in_Position, 1.f);

    ex_VertexPosition = WorldPosition.xyz;
    ex_Normal = mat3(transpose(inverse(Model))) * in_Normal;
    ex_TextureCoords = in_TextureCoords;
    
    vec3 T = normalize(vec3(Model * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(Model * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(Model * vec4(in_Normal, 0.f)));

    ex_TBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * WorldPosition;
}
)SHADER";