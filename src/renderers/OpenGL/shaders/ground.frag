#version 450

in vec3 ex_VertexPosition;
in vec3 ex_NearPlanePosition;
in vec3 ex_FarPlanePosition;

out vec4 out_Color;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform vec3 u_CameraPosition;

float Checkerboard(vec2 R, float Scale) {
	float Result = float((
        int(floor(R.x / Scale)) +
		int(floor(R.y / Scale))
	) % 2);
    
    return Result;
}

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
    float t = -ex_NearPlanePosition.y / (ex_FarPlanePosition.y - ex_NearPlanePosition.y);
    vec3 R = ex_NearPlanePosition + t * (ex_FarPlanePosition - ex_NearPlanePosition);

    gl_FragDepth = ComputeDepth(R, u_View, u_Projection);

    float Color =
		Checkerboard(R.xz, 5.f) * 0.6f +
		Checkerboard(R.xz, 10.f) * 0.2f +
		Checkerboard(R.xz, 0.5f) * 0.1f +
		0.1f;

    Color *= float(t > 0);

    float DistanceFromCamera = length(u_CameraPosition - R);
    float Opacity = clamp(DistanceFromCamera * 0.005f, 0.f, 1.f);

	out_Color = vec4(Color * vec3(0.4f, 0.4f, 0.8f), 1.f - Opacity);
}