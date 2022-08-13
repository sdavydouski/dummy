//! #include "common/version.glsl"
//! #include "common/uniform.glsl"

layout(location = 0) in vec3 in_Position;

uniform mat4 u_Model;

void main()
{ 
    gl_Position = u_Projection * u_View * u_Model * vec4(in_Position, 1.f);
}
