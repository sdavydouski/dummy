//? #include "version.glsl"
//? #include "constants.glsl"
//? #include "phong.glsl"

layout(std140, binding = 0) uniform Transform
{
    mat4 u_ViewProjection;
    mat4 u_ScreenProjection;
    mat4 u_SkyProjection;

    vec3 u_CameraPosition;
    vec3 u_CameraDirection;

    float u_Time;
};

layout(std140, binding = 1) uniform Shading
{
    // todo: multile directional lights
    directional_light u_DirectionalLight;

    int u_PointLightCount;
    point_light u_PointLights[MAX_POINT_LIGHT_COUNT];
};

// todo: ?
mat4 GetViewProjection(int Mode)
{
    mat4 Result = mat4(1.f);

    if (Mode == WORLD_SPACE_MODE)
    {
        Result = u_ViewProjection;
    }
    else if (Mode == SCREEN_SPACE_MODE)
    {
        Result = u_ScreenProjection;
    }

    return Result;
}
