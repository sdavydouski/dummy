#define PI 3.14159f
#define HALF_PI (PI / 2.f)
#define TWO_PI (PI * 2.f)

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

#if 0
vec3 UnprojectPoint(vec3 p, mat4 View, mat4 Projection)
{
    mat4 ViewInv = inverse(View);
    mat4 ProjectionInv = inverse(Projection);
    vec4 UnprojectedPoint = ViewInv * ProjectionInv * vec4(p, 1.f);
    vec3 Result = UnprojectedPoint.xyz / UnprojectedPoint.w;
    
    return Result;
}
#endif

vec3 UnprojectPoint(vec3 p, mat4 ViewProjection)
{
    mat4 ViewProjectionInv = inverse(ViewProjection);
    vec4 UnprojectedPoint = ViewProjectionInv * vec4(p, 1.f);
    vec3 Result = UnprojectedPoint.xyz / UnprojectedPoint.w;
    
    return Result;
}
