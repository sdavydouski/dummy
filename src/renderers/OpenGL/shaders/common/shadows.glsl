//? #include "version.glsl"
//? #include "math.glsl"

#define shadowSampler sampler2DShadow

uniform bool u_ShowCascades;
uniform vec2 u_CascadeBounds[4];
uniform mat4 u_CascadeViewProjection[4];
uniform shadowSampler u_ShadowMaps[4];

float SampleShadowMap(shadowSampler ShadowMap, vec2 Coords, float Compare)
{
	return max(texture(ShadowMap, vec3(Coords, Compare)).r, 0.4f);
}

#if 0
float SampleShadowMapLinear(shadowSampler ShadowMap, vec2 Coords, float Compare, vec2 TexelSize)
{
	vec2 PixelPosition = Coords / TexelSize + vec2(0.5);
	vec2 FracPart = fract(PixelPosition);
	vec2 StartTexel = (PixelPosition - FracPart) * TexelSize;
	
	float blTexel = SampleShadowMap(ShadowMap, StartTexel, Compare);
	float brTexel = SampleShadowMap(ShadowMap, StartTexel + vec2(TexelSize.x, 0.f), Compare);
	float tlTexel = SampleShadowMap(ShadowMap, StartTexel + vec2(0.f, TexelSize.y), Compare);
	float trTexel = SampleShadowMap(ShadowMap, StartTexel + TexelSize, Compare);
	
	float mixA = Lerp(blTexel, tlTexel, FracPart.y);
	float mixB = Lerp(brTexel, trTexel, FracPart.y);
	
	return Lerp(mixA, mixB, FracPart.x);
}
#endif

float SampleShadowMapPCF(shadowSampler ShadowMap, vec2 Coords, float Compare, vec2 TexelSize)
{
	float SamplesCount = 5.f;
	float SamplesStart = (SamplesCount - 1.f) / 2.f;

	float Result = 0.f;

	for(float y = -SamplesStart; y <= SamplesStart; y += 1.f)
	{
		for(float x = -SamplesStart; x <= SamplesStart; x += 1.f)
		{
			vec2 CoordsOffset = vec2(x,y) * TexelSize;
			Result += SampleShadowMap(ShadowMap, Coords + CoordsOffset, Compare);
		}
	}

	return Result / Square(SamplesCount);
}

vec3 CalculateInfiniteShadow(vec3 CascadeBlend, vec4 WorldPosition, float Bias = 0.f)
{
    // Calculate layer indices i and j.
    bool BeyondCascade2 = (CascadeBlend.y >= 0.f);
    bool BeyondCascade3 = (CascadeBlend.z >= 0.f);

    int CascadeIndex1 = int(BeyondCascade2) * 2;
    int CascadeIndex2 = int(BeyondCascade3) * 2 + 1;

    // Calculate blend weight w.
    vec3 Blend = Saturate(CascadeBlend);
    float Weight = (BeyondCascade2) ? Blend.y - Blend.z : 1.f - Blend.x;

    //
    vec4 FragLightPosition1 = u_CascadeViewProjection[CascadeIndex1] * WorldPosition;
    vec4 FragLightPosition2 = u_CascadeViewProjection[CascadeIndex2] * WorldPosition;
    vec3 p1 = FragLightPosition1.xyz * 0.5f + 0.5f;
    vec3 p2 = FragLightPosition2.xyz * 0.5f + 0.5f;

    //
    vec2 ShadowMapSize = textureSize(u_ShadowMaps[0], 0);
    vec2 TexelSize = 1.f / ShadowMapSize;

	if (Bias == 0.f)
	{
		Bias = 1.f * TexelSize.x;
	}

    // Fetch from the first cascade.
    float Light1 = SampleShadowMapPCF(u_ShadowMaps[CascadeIndex1], p1.xy, p1.z - Bias, TexelSize);

    // Fetch from the second cascade.
    float Light2 = SampleShadowMapPCF(u_ShadowMaps[CascadeIndex2], p2.xy, p2.z - Bias, TexelSize);

    // Return blended average value.
    float Shadow = Lerp(Light2, Light1, Weight);

    return vec3(Shadow, CascadeIndex1, CascadeIndex2);
}
