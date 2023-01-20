//? #include "version.glsl"
//? #include "constants.glsl"

layout(std140, binding = 0) uniform State
{
    mat4 u_WorldProjection;
    mat4 u_ScreenProjection;
    mat4 u_View;
    vec3 u_CameraPosition;
    vec3 u_CameraDirection;
    float u_Time;
};

mat4 GetViewProjection(int Mode)
{
    mat4 Result = mat4(1.f);

    if (Mode == WORLD_SPACE_MODE)
    {
        Result = u_WorldProjection * u_View;
    }
    else if (Mode == SCREEN_SPACE_MODE)
    {
        Result = u_ScreenProjection;
    }

    return Result;
}
