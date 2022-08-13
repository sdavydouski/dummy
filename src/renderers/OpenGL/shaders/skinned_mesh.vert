//! #include "common/version.glsl"
//! #include "common/uniform.glsl"
//! #include "common/blinn_phong.glsl"

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
    vec3 VertexPosition;
    vec3 Normal;
    vec3 CascadeCoord0;
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

    vs_out.VertexPosition = WorldPosition.xyz;
    vs_out.Normal = mat3(transpose(inverse(Model))) * in_Normal;
    vs_out.TextureCoords = in_TextureCoords;
    vs_out.Highlight = 0;
    vs_out.TBN = mat3(T, B, N);

    vec4 p = WorldPosition;

    vec3 n = u_CameraDirection;
    vec3 c = u_CameraPosition;

    vec4 f1;
    f1.xyz = n;
    f1.w = -dot(n, c) - u_CascadeBounds[1].x;
    f1 *= (1.f / (u_CascadeBounds[0].y - u_CascadeBounds[1].x));

    vec4 f2;
    f2.xyz = n;
    f2.w = -dot(n, c) - u_CascadeBounds[2].x;
    f2 *= (1.f / (u_CascadeBounds[1].y - u_CascadeBounds[2].x));

    vec4 f3;
    f3.xyz = n;
    f3.w = -dot(n, c) - u_CascadeBounds[3].x;
    f3 *= (1.f / (u_CascadeBounds[2].y - u_CascadeBounds[3].x));

    vec3 u;
    u.x = dot(f1, p);
    u.y = dot(f2, p);
    u.z = dot(f3, p);

    vs_out.CascadeCoord0 = (u_CascadeViewTexture0 * p).xyz;
    vs_out.CascadeBlend = u;

    gl_Position = u_Projection * u_View * WorldPosition;
}