in VS_OUT
{
    vec3 NearPlanePosition;
    vec3 FarPlanePosition;
    vec3 VertexPosition;
} fs_in;

out vec4 out_Color;

layout (std140, binding = 0) uniform State
{
    mat4 u_Projection;
    mat4 u_View;
	vec3 u_CameraPosition;
    float u_Time;
};

uniform directional_light u_DirectionalLight;
uniform int u_PointLightCount;
uniform point_light u_PointLights[MAX_POINT_LIGHT_COUNT];

// computes Z-buffer depth value, and converts the range.
float ComputeDepth(vec3 p, mat4 View, mat4 Projection) {
	// get the clip-space coordinates
	vec4 ClipSpacePosition = Projection * View * vec4(p.xyz, 1.f);

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
    vec3 GroundPoint = fs_in.NearPlanePosition + t * (fs_in.FarPlanePosition - fs_in.NearPlanePosition);

    float DistanceFromCamera = length(u_CameraPosition.xz - GroundPoint.xz);
    float Opacity = clamp(DistanceFromCamera * 0.005f, 0.f, 1.f);

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
    //

	// todo: very expensive
	gl_FragDepth = ComputeDepth(GroundPoint, u_View, u_Projection);
	out_Color = vec4(Result, 1.f - Opacity);
}