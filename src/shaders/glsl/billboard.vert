//! #include "common/version.glsl"

uniform vec3 u_Position;

void main()
{
    gl_Position = vec4(u_Position, 1.f);
}
