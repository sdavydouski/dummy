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
    
    vec3 Result = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

#if 1
    for (int PointLightIndex = 0; PointLightIndex < MAX_NUMBER_OF_POINT_LIGHTS; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, ex_VertexPosition);
    }
#endif
    
    out_Color = vec4(Result, 1.f);
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

char *FramebufferVertexShader = (char *)
R"SHADER(
#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TextureCoords;

out vec2 ex_TextureCoords;

void main()
{
    ex_TextureCoords = in_TextureCoords;
    gl_Position = vec4(in_Position, 1.f);
}
)SHADER";

char *FramebufferFragmentShader = (char *)
R"SHADER(
#version 450

in vec2 ex_TextureCoords;

out vec4 out_Color;

uniform sampler2D u_ScreenTexture;
uniform float u_Time;

void main()
{
#if 1
    vec4 TexelColor = texture(u_ScreenTexture, ex_TextureCoords + 0.0005*vec2( sin(u_Time+2560.0*ex_TextureCoords.x),cos(u_Time+1440.0*ex_TextureCoords.y)));
    //vec4 TexelColor = texture(u_ScreenTexture, ex_TextureCoords);
    out_Color = vec4(TexelColor.rgb, 1.f);
#else
    out_Color = vec4(1.f, 1.f, 0.f, 1.f);
#endif
}
)SHADER";

char *GroundVertexShader = (char *)
R"SHADER(
#version 450

layout(location = 0) in vec3 in_Position;

out vec3 ex_VertexPosition;
out vec3 ex_NearPlanePosition;
out vec3 ex_FarPlanePosition;

uniform mat4 u_Projection;
uniform mat4 u_View;

vec3 UnprojectPoint(vec3 p, mat4 View, mat4 Projection)
{
    mat4 ViewInv = inverse(View);
    mat4 ProjectionInv = inverse(Projection);
    vec4 UnprojectedPoint = ViewInv * ProjectionInv * vec4(p, 1.f);
    vec3 Result = UnprojectedPoint.xyz / UnprojectedPoint.w;
    
    return Result;
}

void main()
{
    ex_VertexPosition = in_Position;
    ex_NearPlanePosition = UnprojectPoint(vec3(in_Position.xy, -1.f), u_View, u_Projection);
    ex_FarPlanePosition = UnprojectPoint(vec3(in_Position.xy, 1.f), u_View, u_Projection);

    gl_Position = vec4(in_Position, 1.f);
}
)SHADER";

char *GroundFragmentShader = (char *)
R"SHADER(
#version 450

in vec3 ex_VertexPosition;
in vec3 ex_NearPlanePosition;
in vec3 ex_FarPlanePosition;

out vec4 out_Color;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform vec3 u_CameraPosition;

float Checkerboard(vec2 R, float Scale) {
	float Result = float((
        int(floor(R.x / Scale)) +
		int(floor(R.y / Scale))
	) % 2);
    
    return Result;
}

// computes Z-buffer depth value, and converts the range.
float ComputeDepth(vec3 p, mat4 View, mat4 Projection) {
	// get the clip-space coordinates
	vec4 ClipSpacePosition = Projection * View * vec4(p.xyz, 1.f);

	// get the depth value in normalized device coordinates
	float ClipSpaceDepth = ClipSpacePosition.z / ClipSpacePosition.w;

	// and compute the range based on gl_DepthRange settings (not necessary with default settings, but left for completeness)
	float Far = gl_DepthRange.far;
	float Near = gl_DepthRange.near;

	float Depth = (((Far - Near) * ClipSpaceDepth) + Near + Far) / 2.f;

	// and return the result
	return Depth;
}

void main()
{
    float t = -ex_NearPlanePosition.y / (ex_FarPlanePosition.y - ex_NearPlanePosition.y);
    vec3 R = ex_NearPlanePosition + t * (ex_FarPlanePosition - ex_NearPlanePosition);

    gl_FragDepth = ComputeDepth(R, u_View, u_Projection);

    float Color =
		Checkerboard(R.xz, 5.f) * 0.6f +
		Checkerboard(R.xz, 10.f) * 0.2f +
		Checkerboard(R.xz, 0.5f) * 0.1f +
		0.1f;

    Color *= float(t > 0);

    float DistanceFromCamera = length(u_CameraPosition - R);
    float Opacity = clamp(DistanceFromCamera * 0.005f, 0.f, 1.f);

	out_Color = vec4(Color * vec3(0.4f, 0.4f, 0.8f), 1.f - Opacity);
}
)SHADER";