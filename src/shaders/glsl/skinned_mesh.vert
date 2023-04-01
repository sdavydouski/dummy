//! #include "common/version.glsl"
//! #include "common/constants.glsl"
//! #include "common/math.glsl"
//! #include "common/phong.glsl"
//! #include "common/uniform.glsl"
//! #include "common/shadows.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec3 in_Bitangent;
layout(location = 4) in vec2 in_TextureCoords;

layout(std140, binding = 0) buffer SkinningMatrices
{
    mat4 in_SkinningMatrices[];
};

out VS_OUT 
{
    vec3 WorldPosition;
    vec3 Normal;
    vec3 CascadeBlend;
    vec2 TextureCoords;
    mat3 TBN;
    vec3 Color;
} vs_out;

void main()
{
    mat4 SkinningMatrix = in_SkinningMatrices[gl_VertexID];

    vec4 WorldPosition = SkinningMatrix * vec4(in_Position, 1.f);

    vec3 T = normalize(vec3(SkinningMatrix * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(SkinningMatrix * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(SkinningMatrix * vec4(in_Normal, 0.f)));

    vs_out.WorldPosition = WorldPosition.xyz;
    vs_out.Normal = mat3(transpose(inverse(SkinningMatrix))) * in_Normal;
    vs_out.TextureCoords = in_TextureCoords;
    vs_out.TBN = mat3(T, B, N);
    vs_out.CascadeBlend = CalculateCascadeBlend(WorldPosition.xyz, u_CameraDirection, u_CameraPosition);
    vs_out.Color = vec3(1.f);

    gl_Position = u_ViewProjection * WorldPosition;
}
