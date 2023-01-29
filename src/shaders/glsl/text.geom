//! #include "common/version.glsl"
//! #include "common/constants.glsl"
//! #include "common/math.glsl"
//! #include "common/phong.glsl"
//! #include "common/uniform.glsl"

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in VS_OUT {
    vec2 Size;
    vec2 SpriteSize;
    vec2 SpriteOffset;
} gs_in[]; 

out GS_OUT {
    vec2 TextureCoords;
} gs_out; 

uniform int u_Mode;
uniform vec3 u_CameraXAxis;
uniform vec3 u_CameraYAxis;
uniform sampler2D u_FontTextureAtlas;

void main()
{
    vec2 TextureSize = textureSize(u_FontTextureAtlas, 0);
    vec2 NormalizedSpriteSize = gs_in[0].SpriteSize / TextureSize;
    vec2 SpriteOffset = gs_in[0].SpriteOffset;

    vec3 Center = gl_in[0].gl_Position.xyz;
    vec2 HalfSize = gs_in[0].Size / 2.f;
    vec3 xAxis = u_CameraXAxis;
    vec3 yAxis = u_CameraYAxis;

    mat4 ViewProjection = GetViewProjection(u_Mode);

    // Bottom-left
    {
        vec3 VertexPosition = Center - xAxis * HalfSize.x - yAxis * HalfSize.y;
        gl_Position = ViewProjection * vec4(VertexPosition, 1.f);

        vec2 TextureCoords = vec2(0.f, 1.f);
        gs_out.TextureCoords = TextureCoords * NormalizedSpriteSize + SpriteOffset;

        EmitVertex();
    }

    // Bottom-right
    {
        vec3 VertexPosition = Center + xAxis * HalfSize.x - yAxis * HalfSize.y;
        gl_Position = ViewProjection * vec4(VertexPosition, 1.f);

        vec2 TextureCoords = vec2(1.f, 1.f);
        gs_out.TextureCoords = TextureCoords * NormalizedSpriteSize + SpriteOffset;

        EmitVertex();
    }

    // Top-left
    {
        vec3 VertexPosition = Center - xAxis * HalfSize.x + yAxis * HalfSize.y;
        gl_Position = ViewProjection * vec4(VertexPosition, 1.f);

        vec2 TextureCoords = vec2(0.f, 0.f);
        gs_out.TextureCoords = TextureCoords * NormalizedSpriteSize + SpriteOffset;

        EmitVertex();
    }

    // Top-right
    {
        vec3 VertexPosition = Center + xAxis * HalfSize.x + yAxis * HalfSize.y;
        gl_Position = ViewProjection * vec4(VertexPosition, 1.f);

        vec2 TextureCoords = vec2(1.f, 0.f);
        gs_out.TextureCoords = TextureCoords * NormalizedSpriteSize + SpriteOffset;

        EmitVertex();
    }
}
