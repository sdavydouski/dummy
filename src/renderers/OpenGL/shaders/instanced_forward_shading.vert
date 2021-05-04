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

out VS_OUT {
    vec3 VertexPosition;
    vec3 Normal;
    vec2 TextureCoords;
    mat3 TBN;
    unsigned int Highlight;
} vs_out; 

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    mat4 InstanceModel = transpose(in_InstanceModel);

    vec3 T = normalize(vec3(InstanceModel * vec4(in_Tangent, 0.f)));
    vec3 B = normalize(vec3(InstanceModel * vec4(in_Bitangent, 0.f)));
    vec3 N = normalize(vec3(InstanceModel * vec4(in_Normal, 0.f)));
    
    vs_out.VertexPosition = (InstanceModel * vec4(in_Position, 1.f)).xyz;
    vs_out.Normal = mat3(transpose(inverse(InstanceModel))) * in_Normal;
    vs_out.TextureCoords = in_TextureCoords;
    vs_out.Highlight = in_Highlight;
    vs_out.TBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * InstanceModel * vec4(in_Position, 1.f);
}