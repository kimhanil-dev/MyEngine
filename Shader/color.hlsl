
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gViewProj;
    float3 gLightPosW;
    float3 gLightColor;
    float3 gViewDir;
    float gTime;
}

struct VS_INPUT
{
    float3 PosL : POSITION;
    float3 Tangent : TANGENT0;
    float3 Normal : NORMAL0;
    float2 Tex0 : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
    float4 Color : COLOR0;
};

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
VS_OUTPUT VS(VS_INPUT input, uniform float3 lightDir = float3(0.0f, 1.0f, 0.0f))
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    float4 pos = float4(input.PosL, 1.0f);
    float3 normal = input.Normal;
    
    // local
    pos.y += cos(gTime * 5.0f) * pos.x * pos.x * 0.2f;
    
    // world
    pos = mul(pos, gWorld);
    normal = mul(normal,gWorld);
    
    float lDotN = max(dot(normal, lightDir), 0.0f);
    float3 reflLight = lightDir + (normal * (-2 * lDotN));
    
    float blinnPhong = dot(reflLight, gViewDir);

    float lambert = pow(lDotN, 3);
    output.Color.rgb = input.Color.rgb * (lambert * gLightColor) + float3(0.15f, 0.15f, 0.15f);
    
    output.PosH = mul(pos, gViewProj);
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return input.Color; 
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