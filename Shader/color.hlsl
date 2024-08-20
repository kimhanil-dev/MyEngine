
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
    float3 Normal : NORMAL;
    float2 Tex0 : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
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
    
    lightVec /= d;
    
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
  
    // Transform to world space space.
    output.PosW = mul(float4(input.PosL, 1.0f), gWorld).xyz;
    output.NormalW = mul(input.Normal, (float3x3) gWorld);
    
    // Transform to homogeneous clip space.
    output.PosH = mul(float4(output.PosW, 1.0f), gViewProj);
  
    return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    // Interpolating noormal can unnormalize it, so normalize it.
        // Rasterizer의 Interpolating은 linear interpolation이므로 input.NormalW의 길이는 1이 아니다(Normal은 sin, cos을 통해 얻어지는 구형 공간의 값)
    input.NormalW = normalize(input.NormalW);
    float3 toEyeW = normalize(gEyePosW - input.PosW);
    
    // Start with a sum of zero.
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Sum the light contribution from each right source.
    float4 A, D, S;
    ComputeDirectionalLight(gMaterial, gDirectionalLight, input.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    ComputePointLight(gMaterial, gPointLight, input.PosW, input.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    ComputeSpotLight(gMaterial, gSpotLight, input.PosW, input.NormalW, toEyeW, A, D, S);
    ambient += A;
    diffuse += D;
    spec += S;
    
    float diffuseFactor = length(diffuse.rgb);
    if(diffuseFactor > 0.0f)
    {
        diffuse.rgb = diffuse.rgb / diffuseFactor;
    
        float divider = 1.0f / 3.0f;
        diffuseFactor *= 3.0f * 3.0f;
        
        diffuseFactor = floor(diffuseFactor * divider) * divider;
        diffuse.rgb = diffuse.rgb * diffuseFactor;
    }
    
    float4 litColor = ambient + diffuse + spec;
    //Common to take alpha from diffuse material.
    litColor.a = gMaterial.Diffuse.a;
    return litColor;
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

technique11 LightTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
        SetRasterizerState(SolidRS);
    }
}