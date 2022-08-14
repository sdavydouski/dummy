//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/uniform.glsl"
//! #include "common/shadows.glsl"

#define MAX_WEIGHT_COUNT 4

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec3 in_Bitangent;
layout(location = 4) in vec2 in_TextureCoords;
layout(location = 5) in vec4 in_Weights;
layout(location = 6) in ivec4 in_JointIndices;

out VS_OUT 
{
    vec3 WorldPosition;
    vec3 Normal;
    vec3 CascadeBlend;
    vec2 TextureCoords;
    mat3 TBN;
    unsigned int Highlight;
} vs_out;

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

    vec3 T = normalize(vec3(Model * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(Model * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(Model * vec4(in_Normal, 0.f)));

    vs_out.WorldPosition = WorldPosition.xyz;
    vs_out.Normal = mat3(transpose(inverse(Model))) * in_Normal;
    vs_out.TextureCoords = in_TextureCoords;
    vs_out.Highlight = 0;
    vs_out.TBN = mat3(T, B, N);
    vs_out.CascadeBlend = CalculateCascadeBlend(WorldPosition.xyz, u_CameraDirection, u_CameraPosition);

    gl_Position = u_Projection * u_View * WorldPosition;
}