//! #include "common/version.glsl"

in GS_OUT {
    vec4 Color;
} fs_in; 

out vec4 out_Color;

void main()
{
    out_Color = fs_in.Color;
}
