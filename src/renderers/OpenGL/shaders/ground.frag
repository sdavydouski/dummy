//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/uniform.glsl"
//! #include "common/blinn_phong.glsl"
//! #include "common/shadows.glsl"

in VS_OUT
{
    vec3 NearPlanePosition;
    vec3 FarPlanePosition;
} fs_in;

out vec4 out_Color;

// todo: duplicate
uniform directional_light u_DirectionalLight;
uniform int u_PointLightCount;
uniform point_light u_PointLights[MAX_POINT_LIGHT_COUNT];

// computes Z-buffer depth value, and converts the range.
float ComputeDepth(vec3 p, mat4 View, mat4 Projection) {
	// get the clip-space coordinates
	vec4 ClipSpacePosition = Projection * View * vec4(p, 1.f);

	// get the depth value in normalized device coordinates
	float ClipSpaceDepth = ClipSpacePosition.z / ClipSpacePosition.w;

	// and compute the range based on gl_DepthRange settings (not necessary with default settings, but left for completeness)
	float Far = gl_DepthRange.far;
	float Near = gl_DepthRange.near;

	float Depth = (((Far - Near) * ClipSpaceDepth) + Near + Far) / 2.f;

	// and return the result
	return Depth;
}

void main()
{
    float t = -fs_in.NearPlanePosition.y / (fs_in.FarPlanePosition.y - fs_in.NearPlanePosition.y);

    if (t < 0.f) discard;

    vec3 GroundPoint = fs_in.NearPlanePosition + t * (fs_in.FarPlanePosition - fs_in.NearPlanePosition);

    float DistanceFromCamera = length(u_CameraPosition.xz - GroundPoint.xz);
    float Opacity = clamp(DistanceFromCamera * 0.004f, 0.f, 1.f);

    float GridScale = 4.f;
    vec2 Coord = GroundPoint.xz / GridScale;

    float LineWidth = 1.f;
	vec2 Grid = abs(fract(Coord - 0.5) - 0.5) / (LineWidth * fwidth(Coord));
	float Line = min(Grid.x, Grid.y);
	vec3 GridColor = vec3(min(Line, 0.75f));

	// Lighting
	vec3 EyeDirection = normalize(u_CameraPosition - GroundPoint);
	vec3 AmbientColor = vec3(0.f);
	vec3 DiffuseColor = GridColor;
	vec3 SpecularColor = vec3(0.f);
	float SpecularShininess = 1.f;
	vec3 Normal = vec3(0.f, 1.f, 0.f);

    vec3 Result = CalculateDirectionalLight(u_DirectionalLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection);

    for (int PointLightIndex = 0; PointLightIndex < u_PointLightCount; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, GroundPoint);
    }

	// Shadow

    // Ideally this should be in the vertex shader
    vec4 p = vec4(GroundPoint, 1.f);
    vec3 n = u_CameraDirection;
    vec3 c = u_CameraPosition;

    vec4 f1;
    f1.xyz = n;
    f1.w = -dot(n, c) - u_CascadeBounds[1].x;
    f1 *= (1.f / (u_CascadeBounds[0].y - u_CascadeBounds[1].x));

    vec4 f2;
    f2.xyz = n;
    f2.w = -dot(n, c) - u_CascadeBounds[2].x;
    f2 *= (1.f / (u_CascadeBounds[1].y - u_CascadeBounds[2].x));

    vec4 f3;
    f3.xyz = n;
    f3.w = -dot(n, c) - u_CascadeBounds[3].x;
    f3 *= (1.f / (u_CascadeBounds[2].y - u_CascadeBounds[3].x));

    vec3 u;
    u.x = dot(f1, p);
    u.y = dot(f2, p);
    u.z = dot(f3, p);

    vec3 CascadeBlend = u;
    //

    vec3 ShadowResult = CalculateInfiniteShadow(CascadeBlend, p);

    float Shadow = ShadowResult.x;
    int CascadeIndex1 = int(ShadowResult.y);
    int CascadeIndex2 = int(ShadowResult.z);

    Result *= Shadow;

    if (u_ShowCascades)
    {
        // Visualising cascades
        if (CascadeIndex1 == 0 && CascadeIndex2 == 1)
        {
            Result += vec3(1.f, 0.f, 0.f);
        }
        else if (CascadeIndex2 == 1 && CascadeIndex1 == 2)
        {
            Result += vec3(0.f, 1.f, 0.f);
        }
        else if (CascadeIndex1 == 2 && CascadeIndex2 == 3)
        {
            Result += vec3(0.f, 0.f, 1.f);
        }
        else 
        {
            Result += vec3(1.f, 1.f, 0.f);
        }
    }

	out_Color = vec4(Result, 1.f - Opacity);
	gl_FragDepth = ComputeDepth(GroundPoint, u_View, u_Projection);
}