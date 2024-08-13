
struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
};

struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float4 Direction;
};

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Position;
    float Range;
    
    float3 Att;
    float pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Position;
    float Range;
    
    float3 Att;
    float pad;
    
    float3 Direction;
    float Spot;
};

//struct ArrayTest
//{
//    float4 array[4];
    
//    static float2 aggressivePackArray[8] = (float2[8])array;
//};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gViewProj;
    float3 gEyePosW;
    float gTime;
    
    Material gMaterial;
    
    DirectionalLight gDirectionalLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
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

void ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVec = -L.Direction;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
}

void ComputePointLight(Material mat, PointLight L, float3 posW, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVec = L.Position - posW;
    
    float d = length(lightVec);
    
    if (d > L.Range)
        return;
    
    lightVec = d;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    // Flatten avoid dynamic branching.
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
        
         // Attenuate
        float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));
        diffuse *= att;
        spec *= att;
    }
}

void ComputeSpotLight(Material mat, SpotLight L, float3 posW, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVec = L.Position - posW;
    
    float d = length(lightVec);
    
    if (d > L.Range)
        return;

    lightVec /= d;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), L.Specular.w);
        
        spec = specFactor * mat.Specular * L.Specular;
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
    }
    
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
    float att = spot / dot(L.Att, float3(1.0f, d, d * d));
    
    ambient *= spot;
    spec *= att;
    diffuse *= att;
}

// 
// vertex Shader
//
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
   
    float4 posW = mul(float4(input.PosL, 1.0f), gWorld);
    float3 normalW = mul(input.Normal, gWorld);
    
    float3 toEye = normalize(gEyePosW - posW.xyz);
    
    float4 ambient;
    float4 diffuse;
    float4 spec;
    
    //ComputeDirectionalLight(gMaterial, gDirectionalLight, normalW, toEye, ambient, diffuse, spec);
    //ComputePointLight(gMaterial, gPointLight, posW.xyz, normalW.xyz, toEye, ambient, diffuse, spec);
    ComputeSpotLight(gMaterial, gSpotLight, posW.xyz, normalW.xyz, toEye, ambient, diffuse, spec);
    
    output.PosH = mul(posW, gViewProj);
    output.Color = diffuse; //+ ambient + spec;
    
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