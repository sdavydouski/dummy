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
out unsigned int ex_Highlight;

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
    ex_Highlight = 0;
    
    vec3 T = normalize(vec3(Model * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(Model * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(Model * vec4(in_Normal, 0.f)));

    ex_TBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * WorldPosition;
}