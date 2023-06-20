cbuffer Transform : register(b0)
{
    float4x4 ScreenProjection;
};

cbuffer Object : register(b1)
{
    float4x4 Model;
    float4 Color;
}

struct VertexShaderInput
{
	float3 Position : POSITION;
};

struct PixelShaderInput
{
    float4 PixelPosition : SV_POSITION;
};

// Vertex shader
PixelShaderInput VS(VertexShaderInput vin)
{
    PixelShaderInput vout;

    vout.PixelPosition = mul(float4(vin.Position, 1.f), mul(Model, ScreenProjection));

    return vout;
}

// Pixel shader
float4 PS(PixelShaderInput pin) : SV_Target
{
    return Color;
}
