out vec4 out_Color;

layout (std140, binding = 0) uniform State
{
    mat4 u_Projection;
    mat4 u_View;
    vec3 u_CameraPosition;
    float u_Time;
};

uniform vec4 u_Color;
uniform bool u_Blink;

void main()
{
    out_Color = u_Color;

    if (u_Blink)
    {
        out_Color.a = 0.5f + (sin(u_Time * 5.f) + 1.f) / 4.f;
    }
}