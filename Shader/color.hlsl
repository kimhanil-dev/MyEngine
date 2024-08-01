
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
    float gTime;
}

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR0;
};

struct PS_INPUT
{
    float4 Pos : SV_Position;
    float4 Color : COLOR0;
};


float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453123);
}
// 
// vertex Shader
//
VS_OUTPUT VS(float3 PosL : POSITION, float4 Color : COLOR, float2 uv : TEXCOORD0)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.PosH = mul(float4(PosL, 1.0f), gWorldViewProj);
    output.Color = Color;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return input.Color; // Yellow, with Alpha = 1
}

RasterizerState WireFrameRS
{
    FillMode = Wireframe;
    CullMode = Back;
    FrontCounterClockWise = false;
    // Default values used for any properties we do not set.
};

RasterizerState SolidRS
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockWise = false;
    // Default values used for any properties we do not set.
};

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
        SetRasterizerState(SolidRS);
    }
}