//! #include "common/version.glsl"
//! #include "common/math.glsl"
//! #include "common/constants.glsl"
//! #include "common/phong.glsl"
//! #include "common/uniform.glsl"
//! #include "common/shadows.glsl"

in VS_OUT
{
    vec3 NearPlanePosition;
    vec3 FarPlanePosition;
} fs_in;

out vec4 out_Color;

// computes Z-buffer depth value, and converts the range.
float ComputeDepth(vec3 p, mat4 ViewProjection) {
	// get the clip-space coordinates
	vec4 ClipSpacePosition = ViewProjection * vec4(p, 1.f);

	// get the depth value in normalized device coordinates
	float ClipSpaceDepth = ClipSpacePosition.z / ClipSpacePosition.w;

	// and compute the range based on gl_DepthRange settings (not necessary with default settings, but left for completeness)
	float Far = gl_DepthRange.far;
	float Near = gl_DepthRange.near;

	float Depth = (((Far - Near) * ClipSpaceDepth) + Near + Far) / 2.f;

	// and return the result
	return Depth;
}

// https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
void main()
{
    float t = -fs_in.NearPlanePosition.y / (fs_in.FarPlanePosition.y - fs_in.NearPlanePosition.y);

    if (t < 0.f) discard;

    vec3 GroundPoint = fs_in.NearPlanePosition + t * (fs_in.FarPlanePosition - fs_in.NearPlanePosition);

    float DistanceFromCamera = length(u_CameraPosition.xz - GroundPoint.xz);
    float Opacity = clamp(DistanceFromCamera * 0.02f, 0.f, 1.f);

    // todo: grid is 1x1 meters?
    float GridScale = 1.f;
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

    vec3 Ambient = AmbientColor * DiffuseColor;
    float Shadow = 1.f;

    if (u_EnableShadows)
    {
		// Shadows
        vec3 CascadeBlend = CalculateCascadeBlend(GroundPoint, u_CameraDirection, u_CameraPosition);
        vec3 ShadowResult = CalculateInfiniteShadow(CascadeBlend, GroundPoint);

        Shadow = ShadowResult.x;

        int CascadeIndex1 = int(ShadowResult.y);
        int CascadeIndex2 = int(ShadowResult.z);

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
    }

    Result = Ambient + Result * Shadow;

    for (int PointLightIndex = 0; PointLightIndex < u_PointLightCount; ++PointLightIndex)
    {
        point_light PointLight = u_PointLights[PointLightIndex];
        Result += CalculatePointLight(PointLight, AmbientColor, DiffuseColor, SpecularColor, SpecularShininess, Normal, EyeDirection, GroundPoint);
    }

	out_Color = vec4(Result, 1.f - Opacity);
	gl_FragDepth = ComputeDepth(GroundPoint, u_ViewProjection);
}