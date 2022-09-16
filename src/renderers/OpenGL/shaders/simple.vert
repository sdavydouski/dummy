//! #include "common/version.glsl"
//! #include "common/uniform.glsl"

layout(location = 0) in vec3 in_Position;

uniform int u_Mode;
uniform mat4 u_Model;

void main()
{ 
    gl_Position = GetViewProjection(u_Mode) * u_Model * vec4(in_Position, 1.f);
}
