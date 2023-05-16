//? #include "version.glsl"
//? #include "math.glsl"

uniform bool u_EnableShadows;
uniform bool u_ShowCascades;
uniform vec2 u_CascadeBounds[4];
uniform mat4 u_CascadeViewProjection[4];
uniform sampler2DShadow u_CascadeShadowMaps[4];

vec3 CalculateCascadeBlend(vec3 WorldPosition, vec3 CameraDirection, vec3 CameraPosition)
{
    vec4 p = vec4(WorldPosition, 1.f);
    vec3 n = CameraDirection;
    vec3 c = CameraPosition;

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

    vec3 CascadeBlend;
    CascadeBlend.x = dot(f1, p);
    CascadeBlend.y = dot(f2, p);
    CascadeBlend.z = dot(f3, p);

    return CascadeBlend;
}

float SampleShadowMap(sampler2DShadow ShadowMap, vec2 Coords, float Compare)
{
    return max(texture(ShadowMap, vec3(Coords, Compare)).r, 0.4f);
}

#if 0
float SampleShadowMapLinear(sampler2D ShadowMap, vec2 Coords, float Compare, vec2 TexelSize)
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

float SampleShadowMapPCF(sampler2DShadow ShadowMap, vec2 Coords, float Compare, vec2 TexelSize)
{
    float SamplesCount = 3.f;
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

float CalculateInfiniteShadow(vec3 CascadeBlend, vec3 WorldPosition, float Bias, out int CascadeIndex1, out int CascadeIndex2)
{
    // Calculate layer indices
    bool BeyondCascade2 = (CascadeBlend.y >= 0.f);
    bool BeyondCascade3 = (CascadeBlend.z >= 0.f);

    CascadeIndex1 = int(BeyondCascade2) * 2;
    CascadeIndex2 = int(BeyondCascade3) * 2 + 1;

    // Calculate blend weight
    vec3 Blend = Saturate(CascadeBlend);
    float Weight = (BeyondCascade2) ? Blend.y - Blend.z : 1.f - Blend.x;

    // Calculate cascade fragment coordinates 
    vec4 FragLightPosition1 = u_CascadeViewProjection[CascadeIndex1] * vec4(WorldPosition, 1.f);
    vec4 FragLightPosition2 = u_CascadeViewProjection[CascadeIndex2] * vec4(WorldPosition, 1.f);
    vec3 p1 = FragLightPosition1.xyz * 0.5f + 0.5f;
    vec3 p2 = FragLightPosition2.xyz * 0.5f + 0.5f;

    // Calculate texel size
    vec2 ShadowMapSize = textureSize(u_CascadeShadowMaps[0], 0);
    vec2 TexelSize = 1.f / ShadowMapSize;

    // Fetch from the first cascade
    float Light1 = SampleShadowMapPCF(u_CascadeShadowMaps[CascadeIndex1], p1.xy, p1.z - Bias, TexelSize);

    // Fetch from the second cascade
    float Light2 = SampleShadowMapPCF(u_CascadeShadowMaps[CascadeIndex2], p2.xy, p2.z - Bias, TexelSize);

    // Return blended average value
    float Shadow = Lerp(Light2, Light1, Weight);

    return Shadow;
}
