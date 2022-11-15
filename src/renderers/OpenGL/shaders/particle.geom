//! #include "common/version.glsl"
//! #include "common/constants.glsl"
//! #include "common/uniform.glsl"

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in VS_OUT {
    vec2 Size;
    vec4 Color;
} gs_in[]; 

out GS_OUT {
    vec4 Color;
} gs_out; 

uniform vec3 u_CameraXAxis;
uniform vec3 u_CameraYAxis;

void main()
{
    vec3 Center = gl_in[0].gl_Position.xyz;
    vec2 HalfSize = gs_in[0].Size / 2.f;
    vec3 xAxis = u_CameraXAxis;
    vec3 yAxis = u_CameraYAxis;

    gs_out.Color = gs_in[0].Color;

    mat4 ViewProjection = u_WorldProjection * u_View;

    // Bottom-left
    {
        vec3 VertexPosition = Center - xAxis * HalfSize.x - yAxis * HalfSize.y;
        gl_Position = ViewProjection * vec4(VertexPosition, 1.f);

        EmitVertex();
    }

    // Bottom-right
    {
        vec3 VertexPosition = Center + xAxis * HalfSize.x - yAxis * HalfSize.y;
        gl_Position = ViewProjection * vec4(VertexPosition, 1.f);

        EmitVertex();
    }

    // Top-left
    {
        vec3 VertexPosition = Center - xAxis * HalfSize.x + yAxis * HalfSize.y;
        gl_Position = ViewProjection * vec4(VertexPosition, 1.f);

        EmitVertex();
    }

    // Top-right
    {
        vec3 VertexPosition = Center + xAxis * HalfSize.x + yAxis * HalfSize.y;
        gl_Position = ViewProjection * vec4(VertexPosition, 1.f);

        EmitVertex();
    }
}
