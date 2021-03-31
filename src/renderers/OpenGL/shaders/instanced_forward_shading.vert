#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec3 in_Bitangent;
layout(location = 4) in vec2 in_TextureCoords;
// -- ?
layout(location = 5) in ivec4 in_JointIndices;
layout(location = 6) in vec4 in_Weights;
// --
layout(location = 7) in mat4 in_InstanceModel;
layout(location = 11) in unsigned int in_Highlight;

out vec3 ex_VertexPosition;
out vec3 ex_Normal;
out vec2 ex_TextureCoords;
out mat3 ex_TBN;
out unsigned int ex_Highlight;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    mat4 InstanceModel = transpose(in_InstanceModel);

    ex_VertexPosition = (InstanceModel * vec4(in_Position, 1.f)).xyz;
    ex_Normal = mat3(transpose(inverse(InstanceModel))) * in_Normal;
    ex_TextureCoords = in_TextureCoords;
    ex_Highlight = in_Highlight;

    vec3 T = normalize(vec3(InstanceModel * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(InstanceModel * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(InstanceModel * vec4(in_Normal, 0.f)));

    ex_TBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * InstanceModel * vec4(in_Position, 1.f);
}