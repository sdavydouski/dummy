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
layout(location = 2) in vec2 in_TextureCoords;

out vec3 ex_VertexPosition;
out vec3 ex_Normal;
out vec2 ex_TextureCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    ex_VertexPosition = (u_Model * vec4(in_Position, 1.f)).xyz;
    ex_Normal = mat3(transpose(inverse(u_Model))) * in_Normal;
    ex_TextureCoords = in_TextureCoords;

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
    phong_material Material,
    vec3 UnitNormal, 
    vec3 EyeDirection
)
{
    vec3 Ambient = Light.Color * Material.AmbientColor;

    vec3 LightDirection = normalize(-Light.Direction);
    float DiffuseImpact = max(dot(LightDirection, UnitNormal), 0.f);
    vec3 Diffuse = Light.Color * DiffuseImpact;

    vec3 ReflectedLightDirection = reflect(-LightDirection, UnitNormal);
    float SpecularImpact = pow(
        max(dot(ReflectedLightDirection, EyeDirection), 0.f),
        Material.SpecularShininess
    );
    vec3 Specular =  Light.Color * Material.SpecularColor * SpecularImpact;

    vec3 Result = (Ambient + Diffuse + Specular);

    return Result;
}

vec3 CalculatePointLight(
    point_light Light,
    phong_material Material,
    vec3 UnitNormal, 
    vec3 EyeDirection,
    vec3 VertexPosition
)
{
    vec3 Ambient = Light.Color * Material.AmbientColor;

    vec3 LightDirection = normalize(Light.Position - ex_VertexPosition);
    float DiffuseImpact = max(dot(LightDirection, UnitNormal), 0.f);
    vec3 Diffuse = Light.Color * DiffuseImpact;

    vec3 ReflectedLightDirection = reflect(-LightDirection, UnitNormal);
    float SpecularImpact = pow(
        max(dot(ReflectedLightDirection, EyeDirection), 0.f),
        Material.SpecularShininess
    );
    vec3 Specular =  Light.Color * Material.SpecularColor * SpecularImpact;

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
    vec3 UnitNormal = normalize(ex_Normal);
    vec3 EyeDirection = normalize(u_CameraPosition - ex_VertexPosition);

    vec3 LightImpact = CalculateDirectionalLight(u_DirectionalLight, u_Material, UnitNormal, EyeDirection);
#if 1
    for (int PointLightIndex = 0; PointLightIndex < MAX_NUMBER_OF_POINT_LIGHTS; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        LightImpact += CalculatePointLight(PointLight, u_Material, UnitNormal, EyeDirection, ex_VertexPosition);
    }
#endif
    
    vec4 DiffuseColor = vec4(u_Material.DiffuseColor, 1.f) * texture(u_Material.DiffuseMap, ex_TextureCoords);

#if 1
    out_Color = vec4(LightImpact, 1.f) * DiffuseColor;
#else
    out_Color = DiffuseColor;
#endif
}
)SHADER";

char *SkinnedMeshVertexShader = (char *)
R"SHADER(
#version 450

#define MAX_WEIGHT_COUNT 4

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TextureCoords;
layout(location = 3) in ivec4 in_JointIndices;
layout(location = 4) in vec4 in_Weights;

out vec3 ex_VertexPosition;
out vec3 ex_Normal;
out vec2 ex_TextureCoords;

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

    gl_Position = u_Projection * u_View * WorldPosition;
}
)SHADER";