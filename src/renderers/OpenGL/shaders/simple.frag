//! #include "common/version.glsl"

out vec4 out_Color;

uniform vec4 u_Color;
uniform bool u_Blink;

void main()
{
    out_Color = u_Color;

    if (u_Blink)
    {
        //out_Color.a = 0.5f + (sin(u_Time * 5.f) + 1.f) / 4.f;
    }
}