//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/uniform.glsl"
//! #include "common/shadows.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec3 in_Bitangent;
layout(location = 4) in vec2 in_TextureCoords;

out VS_OUT {
    vec3 WorldPosition;
    vec3 Normal;
    vec3 CascadeBlend;
    vec2 TextureCoords;
    mat3 TBN;
} vs_out;

uniform mat4 u_Model;

void main()
{
    vec3 T = normalize(vec3(u_Model * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(u_Model * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(u_Model * vec4(in_Normal, 0.f)));

    vec4 WorldPosition = u_Model * vec4(in_Position, 1.f);

    vs_out.WorldPosition = WorldPosition.xyz;
    vs_out.Normal = mat3(transpose(inverse(u_Model))) * in_Normal;
    vs_out.TextureCoords = in_TextureCoords;
    vs_out.TBN = mat3(T, B, N);
    vs_out.CascadeBlend = CalculateCascadeBlend(WorldPosition.xyz, u_CameraDirection, u_CameraPosition);

    gl_Position = u_Projection * u_View * WorldPosition;
}