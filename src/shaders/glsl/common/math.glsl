//? #include "version.glsl"

#define PI 3.14159f
#define HALF_PI (PI / 2.f)
#define TWO_PI (PI * 2.f)
#define EPSILON 0.00001f

#define Lerp mix

float Square(float Value)
{
    return Value * Value;
}

float Saturate(float Value)
{
    return clamp(Value, 0.f, 1.f);
}

vec3 Saturate(vec3 Value)
{
    return clamp(Value, 0.f, 1.f);
}

vec3 UnprojectPoint(vec3 p, mat4 ViewProjection)
{
    mat4 ViewProjectionInv = inverse(ViewProjection);
    vec4 UnprojectedPoint = ViewProjectionInv * vec4(p, 1.f);
    vec3 Result = UnprojectedPoint.xyz / UnprojectedPoint.w;
    
    return Result;
}

// Compute orthonormal basis for converting from tanget/shading space to world space
void ComputeBasisVectors(const vec3 N, out vec3 S, out vec3 T)
{
    // Branchless select non-degenerate T
    T = cross(N, vec3(0.0, 1.0, 0.0));
    T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(EPSILON, dot(T, T)));

    T = normalize(T);
    S = normalize(cross(N, T));
}

// Convert point from tangent/shading space to world space
vec3 TangentToWorld(const vec3 v, const vec3 N, const vec3 S, const vec3 T)
{
    return S * v.x + T * v.y + N * v.z;
}

// Compute Van der Corput radical inverse
// See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Sample i-th point from Hammersley point set of NumSamples points total
vec2 SampleHammersley(uint i, float InvNumSamples)
{
    return vec2(i * InvNumSamples, RadicalInverse_VdC(i));
}

// Uniformly sample point on a hemisphere.
// Cosine-weighted sampling would be a better fit for Lambertian BRDF but since this
// compute shader runs only once as a pre-processing step performance is not *that* important.
// See: "Physically Based Rendering" 2nd ed., section 13.6.1.
vec3 SampleHemisphere(float u1, float u2)
{
    const float u1p = sqrt(max(0.0, 1.0 - u1*u1));
    return vec3(cos(TWO_PI * u2) * u1p, sin(TWO_PI * u2) * u1p, u1);
}

// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
vec3 SampleGGX(float u1, float u2, float roughness)
{
    float alpha = roughness * roughness;

    float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha*alpha - 1.0) * u2));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta); // Trig. identity
    float phi = TWO_PI * u1;

    // Convert to Cartesian upon return.
    return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// Single term for separable Schlick-GGX below
float SchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method
float SchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.f;

    // Epic suggests using this roughness remapping for analytic lights
    float k = (r * r) / 8.f; 

    return SchlickG1(cosLi, k) * SchlickG1(cosLo, k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method (IBL version)
float SchlickGGX_IBL(float cosLi, float cosLo, float roughness)
{
    float r = roughness;
    float k = (r * r) / 2.0; // Epic suggests using this roughness remapping for IBL lighting
    return SchlickG1(cosLi, k) * SchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor
vec3 FresnelSchlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.f) - F0) * pow(1.f - cosTheta, 5.f);
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float DistributionGGX(float cosLh, float roughness)
{
    float alpha   = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}
