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
out unsigned int ex_Highlight;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    ex_VertexPosition = (u_Model * vec4(in_Position, 1.f)).xyz;
    ex_Normal = mat3(transpose(inverse(u_Model))) * in_Normal;
    ex_TextureCoords = in_TextureCoords;
    ex_Highlight = 0;

    vec3 T = normalize(vec3(u_Model * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(u_Model * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(u_Model * vec4(in_Normal, 0.f)));

    ex_TBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * u_Model * vec4(in_Position, 1.f);
}