layout(std140, binding = 0) uniform State
{
    mat4 u_Projection;
    mat4 u_View;
    vec3 u_CameraPosition;
    vec3 u_CameraDirection;
    float u_Time;
    float u_RenderShadowMap;
};