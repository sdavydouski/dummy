//! #include "common/version.glsl"
//! #include "common/uniform.glsl"

layout(location = 0) in vec3 in_Position;

out VS_OUT {
    vec3 LocalPosition;
} vs_out; 

void main()
{ 
    // todo: move to uniform buffer
    mat4 SkyProjection = u_WorldProjection * mat4(mat3(u_View));

    vs_out.LocalPosition = in_Position;
    gl_Position = SkyProjection * vec4(in_Position, 1.f);
}
