
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gViewProj;
    
    float4 gLightPosW;
    float3 gLightColor;
    
    float3 gEyePosW;
    
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
VS_OUTPUT VS(VS_INPUT input, uniform float3 l = float3(0.0f, -1.0f, 0.0f))
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    float4 p = float4(input.PosL, 1.0f);
    float3 n = input.Normal;
  
    // local
    p.y += cos(gTime * 5.0f) * p.x * p.x * 0.2f;
    
    // world
    p = mul(p, gWorld);
    n = mul(n,gWorld);
    float3 viewDirW = gEyePosW - p;
    
    float3 r = l + (n * (-2 * dot(n, l))); // reflection light direction
    
    float blinnPhong = pow(max(dot(r, viewDirW), 0), 1); // birghtness (specular)
    float lambert = max(dot(n, -l), 0); // brightness (diffuse)
     
                                    // diffuse color    // light diffuse color
    float3 diffuse = lambert * input.Color.rgb * float3(1.0f, 1.0f, 1.0f);
                                    // specular color   // light specular color
    float3 specular = blinnPhong * input.Color.rgb * float3(1.0f, 1.0f, 1.0f);
                                    // ambient color    // light ambient color
    float3 ambient = float3(0.15f, 0.15f, 0.15f) * float3(1.0f, 1.0f, 1.0f);
    
    output.Color.rgb = ambient + diffuse + specular;
    output.Color.a = 1.0f;
    
    output.PosH = mul(p, gViewProj);
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